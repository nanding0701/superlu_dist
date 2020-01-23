/*! \file
Copyright (c) 2003, The Regents of the University of California, through
Lawrence Berkeley National Laboratory (subject to receipt of any required 
approvals from U.S. Dept. of Energy) 

All rights reserved. 

The source code is distributed under BSD license, see the file License.txt
at the top-level directory.
*/


/*! @file 
 * \brief Solves a system of distributed linear equations A*X = B with a
 * general N-by-N matrix A using the LU factors computed previously.
 *
 * <pre>
 * -- Distributed SuperLU routine (version 6.0) --
 * Lawrence Berkeley National Lab, Univ. of California Berkeley.
 * October 15, 2008
 * September 18, 2018  version 6.0
 * </pre>
 */
#include <math.h>
#include <unistd.h>
#include "superlu_ddefs.h"
#ifndef CACHELINE
#define CACHELINE 64  /* bytes, Xeon Phi KNL, Cori haswell, Edision */
#endif

/*
 * Sketch of the algorithm for L-solve:
 * =======================
 *
 * Self-scheduling loop:
 *
 *   while ( not finished ) { .. use message counter to control
 *
 *      reveive a message;
 * 	
 * 	if ( message is Xk ) {
 * 	    perform local block modifications into lsum[];
 *                 lsum[i] -= L_i,k * X[k]
 *          if all local updates done, Isend lsum[] to diagonal process;
 *
 *      } else if ( message is LSUM ) { .. this must be a diagonal process 
 *          accumulate LSUM;
 *          if ( all LSUM are received ) {
 *              perform triangular solve for Xi;
 *              Isend Xi down to the current process column;
 *              perform local block modifications into lsum[];
 *          }
 *      }
 *   }
 *
 * 
 * Auxiliary data structures: lsum[] / ilsum (pointer to lsum array)
 * =======================
 *
 * lsum[] array (local)
 *   + lsum has "nrhs" columns, row-wise is partitioned by supernodes
 *   + stored by row blocks, column wise storage within a row block
 *   + prepend a header recording the global block number.
 *
 *         lsum[]                        ilsum[nsupers + 1]
 *
 *         -----
 *         | | |  <- header of size 2     ---
 *         --------- <--------------------| |
 *         | | | | |			  ---
 * 	   | | | | |	      |-----------| |		
 *         | | | | | 	      |           ---
 *	   ---------          |   |-------| |
 *         | | |  <- header   |   |       ---
 *         --------- <--------|   |  |----| |
 *         | | | | |		  |  |    ---
 * 	   | | | | |              |  |
 *         | | | | |              |  |
 *	   ---------              |  |
 *         | | |  <- header       |  |
 *         --------- <------------|  |
 *         | | | | |                 |
 * 	   | | | | |                 |
 *         | | | | |                 |
 *	   --------- <---------------|
 */
  
/*#define ISEND_IRECV*/

/*
 * Function prototypes
 */
#ifdef _CRAY
fortran void STRSM(_fcd, _fcd, _fcd, _fcd, int*, int*, double*,
		   double*, int*, double*, int*);
_fcd ftcs1;
_fcd ftcs2;
_fcd ftcs3;
#endif

/*! \brief
 *
 * <pre>
 * Purpose
 * =======
 *   Re-distribute B on the diagonal processes of the 2D process mesh.
 * 
 * Note
 * ====
 *   This routine can only be called after the routine pxgstrs_init(),
 *   in which the structures of the send and receive buffers are set up.
 *
 * Arguments
 * =========
 * 
 * B      (input) double*
 *        The distributed right-hand side matrix of the possibly
 *        equilibrated system.
 *
 * m_loc  (input) int (local)
 *        The local row dimension of matrix B.
 *
 * nrhs   (input) int (global)
 *        Number of right-hand sides.
 *
 * ldb    (input) int (local)
 *        Leading dimension of matrix B.
 *
 * fst_row (input) int (global)
 *        The row number of B's first row in the global matrix.
 *
 * ilsum  (input) int* (global)
 *        Starting position of each supernode in a full array.
 *
 * x      (output) double*
 *        The solution vector. It is valid only on the diagonal processes.
 *
 * ScalePermstruct (input) ScalePermstruct_t*
 *        The data structure to store the scaling and permutation vectors
 *        describing the transformations performed to the original matrix A.
 *
 * grid   (input) gridinfo_t*
 *        The 2D process mesh.
 *
 * SOLVEstruct (input) SOLVEstruct_t*
 *        Contains the information for the communication during the
 *        solution phase.
 *
 * Return value
 * ============
 * </pre>
 */
int_t
pdReDistribute_B_to_X(double *B, int_t m_loc, int nrhs, int_t ldb,
                      int_t fst_row, int_t *ilsum, double *x,
		      ScalePermstruct_t *ScalePermstruct,
		      Glu_persist_t *Glu_persist,
		      gridinfo_t *grid, SOLVEstruct_t *SOLVEstruct)
{
    int  *SendCnt, *SendCnt_nrhs, *RecvCnt, *RecvCnt_nrhs;
    int  *sdispls, *sdispls_nrhs, *rdispls, *rdispls_nrhs;
    int  *ptr_to_ibuf, *ptr_to_dbuf;
    int_t  *perm_r, *perm_c; /* row and column permutation vectors */
    int_t  *send_ibuf, *recv_ibuf;
    double *send_dbuf, *recv_dbuf;
    int_t  *xsup, *supno;
    int_t  i, ii, irow, gbi, j, jj, k, knsupc, l, lk, nbrow;
    int    p, procs;
    pxgstrs_comm_t *gstrs_comm = SOLVEstruct->gstrs_comm;
	MPI_Request req_i, req_d, *req_send, *req_recv;
	MPI_Status status, *status_send, *status_recv;
	int Nreq_recv, Nreq_send, pp;
	double t;
#if ( DEBUGlevel>=1 )
    CHECK_MALLOC(grid->iam, "Enter pdReDistribute_B_to_X()");
#endif

    /* ------------------------------------------------------------
       INITIALIZATION.
       ------------------------------------------------------------*/
    perm_r = ScalePermstruct->perm_r;
    perm_c = ScalePermstruct->perm_c;
    procs = grid->nprow * grid->npcol;
    xsup = Glu_persist->xsup;
    supno = Glu_persist->supno;
    SendCnt      = gstrs_comm->B_to_X_SendCnt;
    SendCnt_nrhs = gstrs_comm->B_to_X_SendCnt +   procs;
    RecvCnt      = gstrs_comm->B_to_X_SendCnt + 2*procs;
    RecvCnt_nrhs = gstrs_comm->B_to_X_SendCnt + 3*procs;
    sdispls      = gstrs_comm->B_to_X_SendCnt + 4*procs;
    sdispls_nrhs = gstrs_comm->B_to_X_SendCnt + 5*procs;
    rdispls      = gstrs_comm->B_to_X_SendCnt + 6*procs;
    rdispls_nrhs = gstrs_comm->B_to_X_SendCnt + 7*procs;
    ptr_to_ibuf  = gstrs_comm->ptr_to_ibuf;
    ptr_to_dbuf  = gstrs_comm->ptr_to_dbuf;

    /* ------------------------------------------------------------
       NOW COMMUNICATE THE ACTUAL DATA.
       ------------------------------------------------------------*/

	if(procs==1){ // faster memory copy when procs=1 
	
#ifdef _OPENMP
#pragma omp parallel default (shared)
#endif
	{
#ifdef _OPENMP
#pragma omp master
#endif
	{	
		// t = SuperLU_timer_();
#ifdef _OPENMP
#pragma	omp	taskloop private (i,l,irow,k,j,knsupc) untied 
#endif
		for (i = 0; i < m_loc; ++i) {
			irow = perm_c[perm_r[i+fst_row]]; /* Row number in Pc*Pr*B */
	   
			k = BlockNum( irow );
			knsupc = SuperSize( k );
			l = X_BLK( k );
			
			x[l - XK_H] = k;      /* Block number prepended in the header. */
			
			irow = irow - FstBlockC(k); /* Relative row number in X-block */
			RHS_ITERATE(j) {
			x[l + irow + j*knsupc] = B[i + j*ldb];
			}
		}
	}
	}
	}else{
		k = sdispls[procs-1] + SendCnt[procs-1]; /* Total number of sends */
		l = rdispls[procs-1] + RecvCnt[procs-1]; /* Total number of receives */
		if ( !(send_ibuf = intMalloc_dist(k + l)) )
			ABORT("Malloc fails for send_ibuf[].");
		recv_ibuf = send_ibuf + k;
		if ( !(send_dbuf = doubleMalloc_dist((k + l)* (size_t)nrhs)) )
			ABORT("Malloc fails for send_dbuf[].");
		recv_dbuf = send_dbuf + k * nrhs;
		if ( !(req_send = (MPI_Request*) SUPERLU_MALLOC(procs*sizeof(MPI_Request))) )
			ABORT("Malloc fails for req_send[].");	
		if ( !(req_recv = (MPI_Request*) SUPERLU_MALLOC(procs*sizeof(MPI_Request))) )
			ABORT("Malloc fails for req_recv[].");
		if ( !(status_send = (MPI_Status*) SUPERLU_MALLOC(procs*sizeof(MPI_Status))) )
			ABORT("Malloc fails for status_send[].");
		if ( !(status_recv = (MPI_Status*) SUPERLU_MALLOC(procs*sizeof(MPI_Status))) )
			ABORT("Malloc fails for status_recv[].");
		
		for (p = 0; p < procs; ++p) {
			ptr_to_ibuf[p] = sdispls[p];
			ptr_to_dbuf[p] = sdispls[p] * nrhs;
		}
		
		/* Copy the row indices and values to the send buffer. */
		// t = SuperLU_timer_();
		for (i = 0, l = fst_row; i < m_loc; ++i, ++l) {
			irow = perm_c[perm_r[l]]; /* Row number in Pc*Pr*B */
		gbi = BlockNum( irow );
		p = PNUM( PROW(gbi,grid), PCOL(gbi,grid), grid ); /* Diagonal process */
		k = ptr_to_ibuf[p];
		send_ibuf[k] = irow;
		++ptr_to_ibuf[p];
		
		k = ptr_to_dbuf[p];
		RHS_ITERATE(j) { /* RHS is stored in row major in the buffer. */
			send_dbuf[k++] = B[i + j*ldb];
		}
		ptr_to_dbuf[p] += nrhs;
		}
		
		// t = SuperLU_timer_() - t;
		// printf(".. copy to send buffer time\t%8.4f\n", t);	

	#if 1
		/* Communicate the (permuted) row indices. */
		MPI_Alltoallv(send_ibuf, SendCnt, sdispls, mpi_int_t,
			  recv_ibuf, RecvCnt, rdispls, mpi_int_t, grid->comm);
 		/* Communicate the numerical values. */
		MPI_Alltoallv(send_dbuf, SendCnt_nrhs, sdispls_nrhs, MPI_DOUBLE,
			  recv_dbuf, RecvCnt_nrhs, rdispls_nrhs, MPI_DOUBLE,
			  grid->comm);
	#else
 		/* Communicate the (permuted) row indices. */
		MPI_Ialltoallv(send_ibuf, SendCnt, sdispls, mpi_int_t,
				recv_ibuf, RecvCnt, rdispls, mpi_int_t, grid->comm, &req_i);
 		/* Communicate the numerical values. */
		MPI_Ialltoallv(send_dbuf, SendCnt_nrhs, sdispls_nrhs, MPI_DOUBLE,
				recv_dbuf, RecvCnt_nrhs, rdispls_nrhs, MPI_DOUBLE,
				grid->comm, &req_d);	
		MPI_Wait(&req_i,&status);
		MPI_Wait(&req_d,&status);
 	#endif
		/* ------------------------------------------------------------
		   Copy buffer into X on the diagonal processes.
		   ------------------------------------------------------------*/
	    //int iam;
        //MPI_Comm_rank(MPI_COMM_WORLD,&iam);
		// t = SuperLU_timer_();
		ii = 0;
        //double checksum;
        //int lastk=-1;
        //int locatechecksum=0;
		for (p = 0; p < procs; ++p) {
			jj = rdispls_nrhs[p];
			for (i = 0; i < RecvCnt[p]; ++i) {
			/* Only the diagonal processes do this; the off-diagonal processes
			   have 0 RecvCnt. */
			    irow = recv_ibuf[ii]; /* The permuted row index. */
			    k = BlockNum( irow );
			    knsupc = SuperSize( k );
			    lk = LBi( k, grid );  /* Local block number. */
			    l = X_BLK( lk );
			    x[l - XK_H] = k;      /* Block number prepended in the header. */

			    irow = irow - FstBlockC(k); /* Relative row number in X-block */
                RHS_ITERATE(j) {
				    x[l + irow + j*knsupc] = recv_dbuf[jj++];
                }
			    ++ii;
            }
		}

		// t = SuperLU_timer_() - t;
		// printf(".. copy to x time\t%8.4f\n", t);	
		
		SUPERLU_FREE(send_ibuf);
		SUPERLU_FREE(send_dbuf);
		SUPERLU_FREE(req_send);
		SUPERLU_FREE(req_recv);
		SUPERLU_FREE(status_send);
		SUPERLU_FREE(status_recv);	
	}  

    
#if ( DEBUGlevel>=1 )
    CHECK_MALLOC(grid->iam, "Exit pdReDistribute_B_to_X()");
#endif
    return 0;
} /* pdReDistribute_B_to_X */

/*! \brief
 *
 * <pre>
 * Purpose
 * =======
 *   Re-distribute X on the diagonal processes to B distributed on all
 *   the processes.
 *
 * Note
 * ====
 *   This routine can only be called after the routine pxgstrs_init(),
 *   in which the structures of the send and receive buffers are set up.
 * </pre>
 */

int_t
pdReDistribute_X_to_B(int_t n, double *B, int_t m_loc, int_t ldb, int_t fst_row,
		      int_t nrhs, double *x, int_t *ilsum,
		      ScalePermstruct_t *ScalePermstruct,
		      Glu_persist_t *Glu_persist, gridinfo_t *grid,
		      SOLVEstruct_t *SOLVEstruct)
{
    int_t  i, ii, irow, j, jj, k, knsupc, nsupers, l, lk;
    int_t  *xsup, *supno;
    int  *SendCnt, *SendCnt_nrhs, *RecvCnt, *RecvCnt_nrhs;
    int  *sdispls, *rdispls, *sdispls_nrhs, *rdispls_nrhs;
    int  *ptr_to_ibuf, *ptr_to_dbuf;
    int_t  *send_ibuf, *recv_ibuf;
    double *send_dbuf, *recv_dbuf;
    int_t  *row_to_proc = SOLVEstruct->row_to_proc; /* row-process mapping */
    pxgstrs_comm_t *gstrs_comm = SOLVEstruct->gstrs_comm;
    int  iam, p, q, pkk, procs;
    int_t  num_diag_procs, *diag_procs;
	MPI_Request req_i, req_d, *req_send, *req_recv;
	MPI_Status status, *status_send, *status_recv;
	int Nreq_recv, Nreq_send, pp;
	
#if ( DEBUGlevel>=1 )
    CHECK_MALLOC(grid->iam, "Enter pdReDistribute_X_to_B()");
#endif

    /* ------------------------------------------------------------
       INITIALIZATION.
       ------------------------------------------------------------*/
    xsup = Glu_persist->xsup;
    supno = Glu_persist->supno;
    nsupers = Glu_persist->supno[n-1] + 1;
    iam = grid->iam;
    procs = grid->nprow * grid->npcol;
 
    SendCnt      = gstrs_comm->X_to_B_SendCnt;
    SendCnt_nrhs = gstrs_comm->X_to_B_SendCnt +   procs;
    RecvCnt      = gstrs_comm->X_to_B_SendCnt + 2*procs;
    RecvCnt_nrhs = gstrs_comm->X_to_B_SendCnt + 3*procs;
    sdispls      = gstrs_comm->X_to_B_SendCnt + 4*procs;
    sdispls_nrhs = gstrs_comm->X_to_B_SendCnt + 5*procs;
    rdispls      = gstrs_comm->X_to_B_SendCnt + 6*procs;
    rdispls_nrhs = gstrs_comm->X_to_B_SendCnt + 7*procs;
    ptr_to_ibuf  = gstrs_comm->ptr_to_ibuf;
    ptr_to_dbuf  = gstrs_comm->ptr_to_dbuf;

	
	if(procs==1){ //faster memory copy when procs=1
		
#ifdef _OPENMP
#pragma omp parallel default (shared)
#endif
	{
#ifdef _OPENMP
#pragma omp master
#endif
	{	
		// t = SuperLU_timer_();
#ifdef _OPENMP
#pragma	omp	taskloop private (k,knsupc,lk,irow,l,i,j) untied 
#endif		
		for (k = 0; k < nsupers; k++) { 
		knsupc = SuperSize( k );
		lk = LBi( k, grid ); /* Local block number */
		irow = FstBlockC( k );
		l = X_BLK( lk );
		for (i = 0; i < knsupc; ++i) {
			RHS_ITERATE(j) { /* RHS is stored in row major in the buffer. */
				B[irow-fst_row +i + j*ldb] = x[l + i + j*knsupc];
			}
			}
		}
	}
	}	
	}else{
		k = sdispls[procs-1] + SendCnt[procs-1]; /* Total number of sends */
		l = rdispls[procs-1] + RecvCnt[procs-1]; /* Total number of receives */
		if ( !(send_ibuf = intMalloc_dist(k + l)) )
			ABORT("Malloc fails for send_ibuf[].");
		recv_ibuf = send_ibuf + k;
		if ( !(send_dbuf = doubleMalloc_dist((k + l)*nrhs)) )
			ABORT("Malloc fails for send_dbuf[].");
		if ( !(req_send = (MPI_Request*) SUPERLU_MALLOC(procs*sizeof(MPI_Request))) )
			ABORT("Malloc fails for req_send[].");	
		if ( !(req_recv = (MPI_Request*) SUPERLU_MALLOC(procs*sizeof(MPI_Request))) )
			ABORT("Malloc fails for req_recv[].");
		if ( !(status_send = (MPI_Status*) SUPERLU_MALLOC(procs*sizeof(MPI_Status))) )
			ABORT("Malloc fails for status_send[].");
		if ( !(status_recv = (MPI_Status*) SUPERLU_MALLOC(procs*sizeof(MPI_Status))) )
			ABORT("Malloc fails for status_recv[].");	    
		recv_dbuf = send_dbuf + k * nrhs;
		for (p = 0; p < procs; ++p) {
			ptr_to_ibuf[p] = sdispls[p];
			ptr_to_dbuf[p] = sdispls_nrhs[p];
		}
		num_diag_procs = SOLVEstruct->num_diag_procs;
		diag_procs = SOLVEstruct->diag_procs;
 		for (p = 0; p < num_diag_procs; ++p) {  /* For all diagonal processes. */
		pkk = diag_procs[p];
		if ( iam == pkk ) {
			for (k = p; k < nsupers; k += num_diag_procs) {
			knsupc = SuperSize( k );
			lk = LBi( k, grid ); /* Local block number */
			irow = FstBlockC( k );
			l = X_BLK( lk );
			for (i = 0; i < knsupc; ++i) {
	#if 0
				ii = inv_perm_c[irow]; /* Apply X <== Pc'*Y */
	#else
				ii = irow;
	#endif
				q = row_to_proc[ii];
				jj = ptr_to_ibuf[q];
				send_ibuf[jj] = ii;
				jj = ptr_to_dbuf[q];
				RHS_ITERATE(j) { /* RHS stored in row major in buffer. */
					send_dbuf[jj++] = x[l + i + j*knsupc];
				}
				++ptr_to_ibuf[q];
				ptr_to_dbuf[q] += nrhs;
				++irow;
			}
			}
		}
		}
		
		/* ------------------------------------------------------------
			COMMUNICATE THE (PERMUTED) ROW INDICES AND NUMERICAL VALUES.
		   ------------------------------------------------------------*/
	#if 1
		MPI_Alltoallv(send_ibuf, SendCnt, sdispls, mpi_int_t,
			  recv_ibuf, RecvCnt, rdispls, mpi_int_t, grid->comm);
		MPI_Alltoallv(send_dbuf, SendCnt_nrhs, sdispls_nrhs,MPI_DOUBLE, 
			  recv_dbuf, RecvCnt_nrhs, rdispls_nrhs, MPI_DOUBLE,
			  grid->comm);
	#else
		MPI_Ialltoallv(send_ibuf, SendCnt, sdispls, mpi_int_t,
				recv_ibuf, RecvCnt, rdispls, mpi_int_t, grid->comm,&req_i);
		MPI_Ialltoallv(send_dbuf, SendCnt_nrhs, sdispls_nrhs, MPI_DOUBLE, 
				recv_dbuf, RecvCnt_nrhs, rdispls_nrhs, MPI_DOUBLE,
				grid->comm,&req_d);
 		MPI_Wait(&req_i,&status);
		MPI_Wait(&req_d,&status);		 
	#endif	
		/* ------------------------------------------------------------
		   COPY THE BUFFER INTO B.
		   ------------------------------------------------------------*/
		for (i = 0, k = 0; i < m_loc; ++i) {
		irow = recv_ibuf[i];
		irow -= fst_row; /* Relative row number */
		RHS_ITERATE(j) { /* RHS is stored in row major in the buffer. */
			B[irow + j*ldb] = recv_dbuf[k++];
		}
		}

    SUPERLU_FREE(send_ibuf);
    SUPERLU_FREE(send_dbuf);
	SUPERLU_FREE(req_send);
	SUPERLU_FREE(req_recv);
	SUPERLU_FREE(status_send);
	SUPERLU_FREE(status_recv);	
}
#if ( DEBUGlevel>=1 )
    CHECK_MALLOC(grid->iam, "Exit pdReDistribute_X_to_B()");
#endif
    return 0;

} /* pdReDistribute_X_to_B */




/*! \brief
 *
 * <pre>
 * Purpose
 * =======
 *   Compute the inverse of the diagonal blocks of the L and U
 *   triangular matrices.
 * </pre>
 */
void
pdCompute_Diag_Inv(int_t n, LUstruct_t *LUstruct,gridinfo_t *grid,
                   SuperLUStat_t *stat, int *info)
{
#ifdef HAVE_LAPACK
    Glu_persist_t *Glu_persist = LUstruct->Glu_persist;
    LocalLU_t *Llu = LUstruct->Llu;

    double *lusup;
    double *recvbuf, *tempv;
    double *Linv;/* Inverse of diagonal block */
    double *Uinv;/* Inverse of diagonal block */

    int_t  kcol, krow, mycol, myrow;
    int_t  i, ii, il, j, jj, k, lb, ljb, lk, lptr, luptr;
    int_t  nb, nlb,nlb_nodiag, nub, nsupers;
    int_t  *xsup, *supno, *lsub, *usub;
    int_t  *ilsum;    /* Starting position of each supernode in lsum (LOCAL)*/
    int    Pc, Pr, iam;
    int    knsupc, nsupr;
    int    ldalsum;   /* Number of lsum entries locally owned. */
    int    maxrecvsz, p, pi;
    int_t  **Lrowind_bc_ptr;
    double **Lnzval_bc_ptr;
    double **Linv_bc_ptr;
    double **Uinv_bc_ptr;
    int INFO;
    double t;

    double one = 1.0;
    double zero = 0.0;
	
#if ( PROFlevel>=1 )
    t = SuperLU_timer_();
#endif 

#if ( PRNTlevel>=1 )
    if ( grid->iam==0 ) {
	printf("computing inverse of diagonal blocks...\n");
	fflush(stdout);
    }
#endif
	
    /*
     * Initialization.
     */
    iam = grid->iam;
    Pc = grid->npcol;
    Pr = grid->nprow;
    myrow = MYROW( iam, grid );
    mycol = MYCOL( iam, grid );
    xsup = Glu_persist->xsup;
    supno = Glu_persist->supno;
    nsupers = supno[n-1] + 1;
    Lrowind_bc_ptr = Llu->Lrowind_bc_ptr;
    Linv_bc_ptr = Llu->Linv_bc_ptr;
    Uinv_bc_ptr = Llu->Uinv_bc_ptr;
    Lnzval_bc_ptr = Llu->Lnzval_bc_ptr;
    nlb = CEILING( nsupers, Pr ); /* Number of local block rows. */
    
    Llu->inv = 1;

    /*---------------------------------------------------
     * Compute inverse of L(lk,lk).
     *---------------------------------------------------*/

     for (k = 0; k < nsupers; ++k) {
         krow = PROW( k, grid );
	 if ( myrow == krow ) {
	     lk = LBi( k, grid );    /* local block number */
	     kcol = PCOL( k, grid );
	     if ( mycol == kcol ) { /* diagonal process */

	     	  lk = LBj( k, grid ); /* Local block number, column-wise. */
		  lsub = Lrowind_bc_ptr[lk];
		  lusup = Lnzval_bc_ptr[lk];
		  Linv = Linv_bc_ptr[lk];
		  Uinv = Uinv_bc_ptr[lk];
		  nsupr = lsub[1];	
		  knsupc = SuperSize( k );

		  for (j=0 ; j<knsupc; j++){
		      for (i=0 ; i<knsupc; i++){
		  	  Linv[j*knsupc+i] = zero;	
			  Uinv[j*knsupc+i] = zero;	
		      }
	          }
				
	   	  for (j=0 ; j<knsupc; j++){
		      Linv[j*knsupc+j] = one;
		      for (i=j+1 ; i<knsupc; i++){
		          Linv[j*knsupc+i] = lusup[j*nsupr+i];	
		      }
		      for (i=0 ; i<j+1; i++){
			  Uinv[j*knsupc+i] = lusup[j*nsupr+i];	
	              }
 		  }

		  /* Triangular inversion */
   		  dtrtri_("L","U",&knsupc,Linv,&knsupc,&INFO);

		  dtrtri_("U","N",&knsupc,Uinv,&knsupc,&INFO);

	    } /* end if (mycol === kcol) */
	} /* end if (myrow === krow) */
    } /* end fo k = ... nsupers */

#if ( PROFlevel>=1 )
    if( grid->iam==0 ) {
	t = SuperLU_timer_() - t;
	printf(".. L-diag_inv time\t%10.5f\n", t);
	fflush(stdout);
    }
#endif	

    return;
#endif /* HAVE_LAPACK */
}


/*! \brief
 *
 * <pre>
 * Purpose
 * =======
 *
 * PDGSTRS solves a system of distributed linear equations
 * A*X = B with a general N-by-N matrix A using the LU factorization
 * computed by PDGSTRF.
 * If the equilibration, and row and column permutations were performed,
 * the LU factorization was performed for A1 where
 *     A1 = Pc*Pr*diag(R)*A*diag(C)*Pc^T = L*U
 * and the linear system solved is
 *     A1 * Y = Pc*Pr*B1, where B was overwritten by B1 = diag(R)*B, and
 * the permutation to B1 by Pc*Pr is applied internally in this routine.
 * 
 * Arguments
 * =========
 *
 * n      (input) int (global)
 *        The order of the system of linear equations.
 *
 * LUstruct (input) LUstruct_t*
 *        The distributed data structures storing L and U factors.
 *        The L and U factors are obtained from PDGSTRF for
 *        the possibly scaled and permuted matrix A.
 *        See superlu_ddefs.h for the definition of 'LUstruct_t'.
 *        A may be scaled and permuted into A1, so that
 *        A1 = Pc*Pr*diag(R)*A*diag(C)*Pc^T = L*U
 *
 * grid   (input) gridinfo_t*
 *        The 2D process mesh. It contains the MPI communicator, the number
 *        of process rows (NPROW), the number of process columns (NPCOL),
 *        and my process rank. It is an input argument to all the
 *        parallel routines.
 *        Grid can be initialized by subroutine SUPERLU_GRIDINIT.
 *        See superlu_defs.h for the definition of 'gridinfo_t'.
 *
 * B      (input/output) double*
 *        On entry, the distributed right-hand side matrix of the possibly
 *        equilibrated system. That is, B may be overwritten by diag(R)*B.
 *        On exit, the distributed solution matrix Y of the possibly
 *        equilibrated system if info = 0, where Y = Pc*diag(C)^(-1)*X,
 *        and X is the solution of the original system.
 *
 * m_loc  (input) int (local)
 *        The local row dimension of matrix B.
 *
 * fst_row (input) int (global)
 *        The row number of B's first row in the global matrix.
 *
 * ldb    (input) int (local)
 *        The leading dimension of matrix B.
 *
 * nrhs   (input) int (global)
 *        Number of right-hand sides.
 * 
 * SOLVEstruct (input) SOLVEstruct_t* (global)
 *        Contains the information for the communication during the
 *        solution phase.
 *
 * stat   (output) SuperLUStat_t*
 *        Record the statistics about the triangular solves.
 *        See util.h for the definition of 'SuperLUStat_t'.
 *
 * info   (output) int*
 * 	   = 0: successful exit
 *	   < 0: if info = -i, the i-th argument had an illegal value
 * </pre>       
 */

void
pdgstrs(int_t n, LUstruct_t *LUstruct, 
	ScalePermstruct_t *ScalePermstruct,
	gridinfo_t *grid, double *B,
	int_t m_loc, int_t fst_row, int_t ldb, int nrhs,
	SOLVEstruct_t *SOLVEstruct,
	SuperLUStat_t *stat, int *info)
{
    Glu_persist_t *Glu_persist = LUstruct->Glu_persist;
    LocalLU_t *Llu = LUstruct->Llu;
    double alpha = 1.0;
	double beta = 0.0;
    double zero = 0.0;
		    /* NOTE: x and lsum are of same size. */
    double *lusup, *dest;
#ifdef pget
    double *recvbuf, *recvbuf_on, *tempv,
            *recvbufall, *recvbuf_BC_fwd, *xin;
#else
    double *recvbuf, *recvbuf_on, *tempv,
            *recvbufall, *recvbuf_BC_fwd, *recvbuf0, *xin;
    double *lsum;  /* Local running sum of the updates to B-components */
    double *x;     /* X component at step k. */
#endif
    double *rtemp, *rtemp_loc; /* Result of full matrix-vector multiply. */
    double *Linv; /* Inverse of diagonal block */
    double *Uinv; /* Inverse of diagonal block */
    int *ipiv; 
    int_t *leaf_send;
    int_t nleaf_send, nleaf_send_tmp;
    int_t *root_send;
    int_t nroot_send, nroot_send_tmp;
    int_t  **Ufstnz_br_ptr = Llu->Ufstnz_br_ptr;
        /*-- Data structures used for broadcast and reduction trees. --*/
    BcTree  *LBtree_ptr = Llu->LBtree_ptr;
    RdTree  *LRtree_ptr = Llu->LRtree_ptr;
    BcTree  *UBtree_ptr = Llu->UBtree_ptr;
    RdTree  *URtree_ptr = Llu->URtree_ptr;	
    int_t  *Urbs1, *Urbs2; /* Number of row blocks in each block column of U. */
    int_t  *Urbs = Llu->Urbs; /* Number of row blocks in each block column of U. */
    Ucb_indptr_t **Ucb_indptr = Llu->Ucb_indptr;/* Vertical linked list pointing to Uindex[] */
    int_t  **Ucb_valptr = Llu->Ucb_valptr;      /* Vertical linked list pointing to Unzval[] */
    int_t  kcol, krow, mycol, myrow;
    int_t  i, ii, il, j, jj, k, kk, lb, ljb, lk, lib, lptr, luptr, gb, nn;
    int_t  nb, nlb,nlb_nodiag, nub, nsupers, nsupers_j, nsupers_i,maxsuper;
    int_t  *xsup, *supno, *lsub, *usub;
    int_t  *ilsum;    /* Starting position of each supernode in lsum (LOCAL)*/
    int    Pc, Pr, iam;
    int    knsupc, nsupr, nprobe;
    int    nbtree, nrtree, outcount;
    int    ldalsum;   /* Number of lsum entries locally owned. */
    int    maxrecvsz, p, pi;
    int_t  **Lrowind_bc_ptr;
    double **Lnzval_bc_ptr;
    double **Linv_bc_ptr;
    double **Uinv_bc_ptr;
    double sum;
    MPI_Status status,status_on,statusx,statuslsum;
    MPI_Request *send_req, recv_req, req;
    pxgstrs_comm_t *gstrs_comm = SOLVEstruct->gstrs_comm;
    SuperLUStat_t **stat_loc;

    double tmax;
    	/*-- Counts used for L-solve --*/
    int_t  *fmod;         /* Modification count for L-solve --
    			 Count the number of local block products to
    			 be summed into lsum[lk]. */
    int_t fmod_tmp;
    int_t  **fsendx_plist = Llu->fsendx_plist;
    int_t  nfrecvx = Llu->nfrecvx; /* Number of X components to be recv'd. */
    int_t  nfrecvx_buf=0;						 	    			 
    int_t  *frecv;        /* Count of lsum[lk] contributions to be received
    			 from processes in this row. 
    			 It is only valid on the diagonal processes. */
    int_t  frecv_tmp;
    int_t  nfrecvmod = 0; /* Count of total modifications to be recv'd. */
    int_t  nfrecv = 0; /* Count of total messages to be recv'd. */
    int_t  nbrecv = 0; /* Count of total messages to be recv'd. */
    int_t  nleaf = 0, nroot = 0;
    int_t  nleaftmp = 0, nroottmp = 0;
    int_t  msgsize;
        /*-- Counts used for U-solve --*/
    int_t  *bmod;         /* Modification count for U-solve. */
    int_t  bmod_tmp;
    int_t  **bsendx_plist = Llu->bsendx_plist;
    int_t  nbrecvx = Llu->nbrecvx; /* Number of X components to be recv'd. */
    int_t  nbrecvx_buf=0;		
    int_t  *brecv;        /* Count of modifications to be recv'd from
    			 processes in this row. */
    int_t  nbrecvmod = 0; /* Count of total modifications to be recv'd. */
    int_t flagx,flaglsum,flag;
    int_t *LBTree_active, *LRTree_active, *LBTree_finish, *LRTree_finish, *leafsups, *rootsups; 
    int_t TAG;
    double t1_sol, t2_sol, t;
#if ( DEBUGlevel>=2 )
    int_t Ublocks = 0;
#endif

    int_t gik,iklrow,fnz;
    
    int_t *mod_bit = Llu->mod_bit; /* flag contribution from each row block */
    int INFO, pad;
    int_t tmpresult;

    // #if ( PROFlevel>=1 )
    double t1, t2;
    float msg_vol = 0, msg_cnt = 0;
    // #endif 

    int_t msgcnt[4]; /* Count the size of the message xfer'd in each buffer:
		      *     0 : transferred in Lsub_buf[]
		      *     1 : transferred in Lval_buf[]
		      *     2 : transferred in Usub_buf[]
		      *     3 : transferred in Uval_buf[]
		      */
    int iword = sizeof (int_t);
    int dword = sizeof (double);	
    int Nwork;
	int_t procs = grid->nprow * grid->npcol;
    	yes_no_t done;
    yes_no_t startforward;
    	int nbrow;
    int_t  ik, rel, idx_r, jb, nrbl, irow, pc,iknsupc;
    int_t  lptr1_tmp, idx_i, idx_v,m; 
    	int_t ready;
    	static int thread_id;
    yes_no_t empty;
    int_t sizelsum,sizertemp,aln_d,aln_i;
    aln_d = ceil(CACHELINE/(double)dword);
    aln_i = ceil(CACHELINE/(double)iword);
    int num_thread = 1;
	
	maxsuper = sp_ienv_dist(3);
	
#ifdef _OPENMP	
	#pragma omp threadprivate(thread_id)
#endif

#ifdef _OPENMP
#pragma omp parallel default(shared)
    {
    	if (omp_get_thread_num () == 0) {
    		num_thread = omp_get_num_threads();
    	}
		thread_id = omp_get_thread_num();
    }
#endif

#if ( PRNTlevel>=1 )
    if( grid->iam==0 ) {
	printf("num_thread: %5d\n", num_thread);
	fflush(stdout);
    }
#endif
	
    MPI_Barrier( grid->comm );
    t1_sol = SuperLU_timer_();
    t = SuperLU_timer_();
    /* Test input parameters. */
    *info = 0;
    if ( n < 0 ) *info = -1;
    else if ( nrhs < 0 ) *info = -9;
    if ( *info ) {
	pxerr_dist("PDGSTRS", grid, -*info);
	return;
    }
	
    /*
     * Initialization.
     */
    iam = grid->iam;
    Pc = grid->npcol;
    Pr = grid->nprow;
    myrow = MYROW( iam, grid );
    mycol = MYCOL( iam, grid );
    xsup = Glu_persist->xsup;
    supno = Glu_persist->supno;
    nsupers = supno[n-1] + 1;
    Lrowind_bc_ptr = Llu->Lrowind_bc_ptr;
    Lnzval_bc_ptr = Llu->Lnzval_bc_ptr;
    Linv_bc_ptr = Llu->Linv_bc_ptr;
    Uinv_bc_ptr = Llu->Uinv_bc_ptr;	
    nlb = CEILING( nsupers, Pr ); /* Number of local block rows. */

    //char processor_name[MPI_MAX_PROCESSOR_NAME];
    //
    //int name_len;
    //MPI_Get_processor_name(processor_name, &name_len);
    //printf("Hello world from processor %s, rank %d\n", processor_name, iam);


    stat->utime[SOL_COMM] = 0.0;
    stat->utime[SOL_GEMM] = 0.0;
    stat->utime[SOL_TRSM] = 0.0;
    stat->utime[SOL_TOT] = 0.0;	
	
#if ( DEBUGlevel>=1 )
    CHECK_MALLOC(iam, "Enter pdgstrs()");
#endif

    stat->ops[SOLVE] = 0.0;
    Llu->SolveMsgSent = 0;

    /* Save the count to be altered so it can be used by
       subsequent call to PDGSTRS. */
    if ( !(fmod = intMalloc_dist(nlb*aln_i)) )
	ABORT("Calloc fails for fmod[].");
    for (i = 0; i < nlb; ++i) fmod[i*aln_i] = Llu->fmod[i];
    if ( !(frecv = intCalloc_dist(nlb)) )
	ABORT("Malloc fails for frecv[].");
    Llu->frecv = frecv;

    if ( !(leaf_send = intMalloc_dist((CEILING( nsupers, Pr )+CEILING( nsupers, Pc ))*aln_i)) )
	ABORT("Malloc fails for leaf_send[].");
    nleaf_send=0;
    if ( !(root_send = intMalloc_dist((CEILING( nsupers, Pr )+CEILING( nsupers, Pc ))*aln_i)) )
	ABORT("Malloc fails for root_send[].");
    nroot_send=0;

#ifdef _CRAY
    ftcs1 = _cptofcd("L", strlen("L"));
    ftcs2 = _cptofcd("N", strlen("N"));
    ftcs3 = _cptofcd("U", strlen("U"));
#endif


    /* Obtain ilsum[] and ldalsum for process column 0. */
    ilsum = Llu->ilsum;
    ldalsum = Llu->ldalsum;

    /* Allocate working storage. */
    knsupc = sp_ienv_dist(3);
#ifdef oneside    
    maxrecvsz = knsupc * nrhs + SUPERLU_MAX( XK_H, LSUM_H ) + 1;
#else
    maxrecvsz = knsupc * nrhs + SUPERLU_MAX( XK_H, LSUM_H );
#endif    
    sizelsum = (((size_t)ldalsum)*nrhs + nlb*LSUM_H);
    sizelsum = ((sizelsum + (aln_d - 1)) / aln_d) * aln_d;


#ifdef pget
    #ifdef _OPENMP
    //printf("(%d) In solve,size of x=%d, size of lsum=%d\n",iam,ldalsum * nrhs + nlb * XK_H,sizelsum*num_thread);
    //fflush(stdout);
        #pragma omp parallel default(shared) private(ii)
        {
            for (ii=0; ii<sizelsum; ii++) lsum[thread_id*sizelsum+ii]=zero;
        }
    #else
        for ( ii=0; ii < sizelsum*num_thread; ii++ ) lsum[ii]=zero;
    #endif
#else
    #ifdef _OPENMP
        if ( !(lsum = (double*)SUPERLU_MALLOC(sizelsum*num_thread * sizeof(double))))
	        ABORT("Malloc fails for lsum[].");
        #pragma omp parallel default(shared) private(ii)
        {
	        for (ii=0; ii<sizelsum; ii++)
        	    lsum[thread_id*sizelsum+ii]=zero;
        }
    #else
        if ( !(lsum = (double*)SUPERLU_MALLOC(sizelsum*num_thread * sizeof(double))))
  	        ABORT("Malloc fails for lsum[].");
        for ( ii=0; ii < sizelsum*num_thread; ii++ )
	        lsum[ii]=zero;
    #endif
        if ( !(x = (double*)SUPERLU_MALLOC((ldalsum * nrhs + nlb * XK_H) * sizeof(double))) )
            ABORT("Calloc fails for x[].");
#endif



    sizertemp=ldalsum * nrhs;
    sizertemp = ((sizertemp + (aln_d - 1)) / aln_d) * aln_d;
#ifdef _OPENMP
    if ( !(rtemp = (double*)SUPERLU_MALLOC(sizertemp*num_thread * sizeof(double))) )
	ABORT("Malloc fails for rtemp[].");		
#pragma omp parallel default(shared) private(ii)
    {
	for ( ii=0; ii<sizertemp; ii++ )
		rtemp[thread_id*sizertemp+ii]=zero;			
    }
#else	
    if ( !(rtemp = (double*)SUPERLU_MALLOC(sizertemp*num_thread * sizeof(double))) )
	ABORT("Malloc fails for rtemp[].");
    for ( ii=0; ii<sizertemp*num_thread; ii++ )
	rtemp[ii]=zero;			
#endif	

    if ( !(stat_loc = (SuperLUStat_t**) SUPERLU_MALLOC(num_thread*sizeof(SuperLUStat_t*))) )
	ABORT("Malloc fails for stat_loc[].");

    for ( i=0; i<num_thread; i++) {
	stat_loc[i] = (SuperLUStat_t*)SUPERLU_MALLOC(sizeof(SuperLUStat_t));
	PStatInit(stat_loc[i]);
    }

    /* Dump the L factor using matlab triple-let format. */
    //dDumpLblocks(iam, nsupers, grid, Glu_persist, Llu);
    /*---------------------------------------------------
     * Forward solve Ly = b.
     *---------------------------------------------------*/
    /* Redistribute B into X on the diagonal processes. */
     //PROFILE_PHYSICS_INIT();
    pdReDistribute_B_to_X(B, m_loc, nrhs, ldb, fst_row, ilsum, x,
			  ScalePermstruct, Glu_persist, grid, SOLVEstruct);
//PROFILE_PHYSICS_FINISH();
#if ( PRNTlevel>=1 )
    t = SuperLU_timer_() - t;
    if ( !iam) printf(".. B to X redistribute time\t%8.4f\n", t);
    fflush(stdout);
    t = SuperLU_timer_();
#endif	

    /* Set up the headers in lsum[]. */
#ifdef _OPENMP	
	#pragma omp simd lastprivate(krow,lk,il)
#endif		
    for (k = 0; k < nsupers; ++k) {
	krow = PROW( k, grid );
	if ( myrow == krow ) {
	    lk = LBi( k, grid );   /* Local block number. */
	    il = LSUM_BLK( lk );
	    lsum[il - LSUM_H] = k; /* Block number prepended in the header. */
	}
    }

	/* ---------------------------------------------------------
	   Initialize the async Bcast trees on all processes.
	   --------------------------------------------------------- */		
	//PROFILE_DYNAMIC_INIT();
        nsupers_j = CEILING( nsupers, grid->npcol ); /* Number of local block columns */

	nbtree = 0;
	for (lk=0;lk<nsupers_j;++lk){
		if(LBtree_ptr[lk]!=NULL){
			// printf("LBtree_ptr lk %5d\n",lk); 
			if(BcTree_IsRoot(LBtree_ptr[lk],'d')==NO){			
				nbtree++;
				if(BcTree_getDestCount(LBtree_ptr[lk],'d')>0)nfrecvx_buf++;				  
			}
			BcTree_allocateRequest(LBtree_ptr[lk],'d');
		}
	}

	nsupers_i = CEILING( nsupers, grid->nprow ); /* Number of local block rows */
	if ( !(	leafsups = (int_t*)intCalloc_dist(nsupers_i)) )
		ABORT("Calloc fails for leafsups.");

	nrtree = 0;
	nleaf=0;
	nfrecvmod=0;
	
	
	
if(procs==1){
	for (lk=0;lk<nsupers_i;++lk){
		gb = myrow+lk*grid->nprow;  /* not sure */
		if(gb<nsupers){
			if (fmod[lk*aln_i]==0){
				leafsups[nleaf]=gb;				
				++nleaf;
			}
		}
	}
}else{	
	for (lk=0;lk<nsupers_i;++lk){
		if(LRtree_ptr[lk]!=NULL){
			nrtree++;
			RdTree_allocateRequest(LRtree_ptr[lk],'d');			
			frecv[lk] = RdTree_GetDestCount(LRtree_ptr[lk],'d');
			nfrecvmod += frecv[lk];
		}else{
			gb = myrow+lk*grid->nprow;  /* not sure */
			if(gb<nsupers){
				kcol = PCOL( gb, grid );
				if(mycol==kcol) { /* Diagonal process */
					if (fmod[lk*aln_i]==0){
						leafsups[nleaf]=gb;				
						++nleaf;
					}
				}
			}
		}
	}	
}	
	
	
#ifdef _OPENMP	
#pragma omp simd
#endif
	for (i = 0; i < nlb; ++i) fmod[i*aln_i] += frecv[i];
	
#ifdef oneside
	int iam_col=MYROW( iam, grid );
	int iam_row=MYCOL( iam, grid );    
    int *BCcount, *RDcount;
    long *BCbase, *RDbase; //BCsendoffset, RDsendoffset;
    //double* BC_taskq;
    //double* RD_taskq;
    double nfrecv1=0;
    int checkend=0;
    int ird=0, tidx=0, bcidx=0, rdidx=0, tmp_id=0;
    int *BCis_solved, *RDis_solved;
    int totalsolveBC=0, totalsolveRD=0;
    long* BC_taskbuf_offset;
    long *RD_taskbuf_offset;
    int BC_buffer_size=0; //= Pr * maxrecvsz*(nfrecvx+1) + Pr; 
    int RD_buffer_size=0; //= Pc * maxrecvsz*(nfrecvmod+1) + Pc; 
    int shift=0;
    int recvRankNum=-1;
    uint16_t crc_16_val;
    double myhash;
    int *validBCQindex;
    int *validRDQindex;
    int *validBCQindex_u;
    int *validRDQindex_u;

    BCcount = (int*)SUPERLU_MALLOC( Pr * sizeof(int));
    RDcount = (int*)SUPERLU_MALLOC( Pc * sizeof(int));
    memset(BCcount, 0, ( Pr * sizeof(int)));
    memset(RDcount, 0, ( Pc * sizeof(int)));
           
    BCbase = (long*)SUPERLU_MALLOC( Pr * sizeof(long));
    RDbase = (long*)SUPERLU_MALLOC( Pc * sizeof(long));
    memset(BCbase, 0, ( Pr * sizeof(long)));
    memset(RDbase, 0, ( Pc * sizeof(long)));
    
    if ( !(validBCQindex = (int*)SUPERLU_MALLOC( Pr * sizeof(int))) )  
    	ABORT("Malloc fails for validBCQindex[]");	
    if ( !(validRDQindex = (int*)SUPERLU_MALLOC( Pc *sizeof(int))) )  
    	ABORT("Malloc fails for validRDQindex[]");	
    if ( !(validBCQindex_u = (int*)SUPERLU_MALLOC( Pr * sizeof(int))) )  
    	ABORT("Malloc fails for validBCQindex_u[]");	
    if ( !(validRDQindex_u = (int*)SUPERLU_MALLOC( Pc *sizeof(int))) )  
    	ABORT("Malloc fails for validRDQindex_u[]");	
    
    if( Pr > 1){
        for (i=0;i<Pr;i++){
                BCbase[i] = recv_size_all[i]*maxrecvsz;
                validBCQindex[i]=keep_validBCQindex[i];
                validBCQindex_u[i]=keep_validBCQindex_u[i];
#if ( DEBUGlevel>=1 )        
                printf("iam=%d,BCbase[%d]=%lu,validBCQindex[%d]=%d,validBCQindex_u[%d]=%d\n",iam,i,BCbase[i],i,validBCQindex[i],i,validBCQindex_u[i]);
                fflush(stdout);
#endif    
        }
    }
    if(Pc > 1){
            for (i=0;i<Pc;i++){
                    RDbase[i] = recv_size_all[Pr+i]*maxrecvsz;
                    validRDQindex[i]=keep_validRDQindex[i];
                    validRDQindex_u[i]=keep_validRDQindex_u[i];
#if ( DEBUGlevel>=1 )        
                    printf("iam=%d,RDbase[%d]=%lu,validRDQindex[%d]=%d,validRDQindex_u[%d]=%d\n",iam,i,RDbase[i],i,validRDQindex[i],i,validRDQindex_u[i]);
                    fflush(stdout);
#endif    
            }
    }        
	
	nfrecvx_buf=0;
    double checksum=0; 

    BC_taskbuf_offset = (long*)SUPERLU_MALLOC( Pr * sizeof(long));   // this needs to be optimized for 1D row mapping
    RD_taskbuf_offset = (long*)SUPERLU_MALLOC( Pc * sizeof(long));   // this needs to be optimized for 1D row mapping
    memset(BC_taskbuf_offset, 0, Pr * sizeof(long));
    memset(RD_taskbuf_offset, 0, Pc * sizeof(long));
    
    for (bcidx=0;bcidx<Pr;bcidx++){
        for(int tmp=0;tmp<bcidx;tmp++){
            BC_taskbuf_offset[bcidx] += BufSize[tmp]*maxrecvsz;
        }       
#if ( DEBUGlevel>=1 )        
        printf("iam=%d, BC_taskbuf_offset[%d]=%lu\n",iam,bcidx,BC_taskbuf_offset[bcidx]);
        fflush(stdout);
#endif    
    }
    for (rdidx=0;rdidx<Pc;rdidx++){
        for(int tmp=0;tmp<rdidx;tmp++){ 
                RD_taskbuf_offset[rdidx] += BufSize_rd[tmp]*maxrecvsz;
        }        
#if ( DEBUGlevel>=1 )        
        printf("iam=%d, RD_taskbuf_offset[%d]=%lu\n",iam,rdidx,RD_taskbuf_offset[rdidx]);
        fflush(stdout);
#endif    
    }
    
    BCis_solved = (int*)SUPERLU_MALLOC( Pr * sizeof(int));   // this needs to be optimized for 1D row mapping
    RDis_solved = (int*)SUPERLU_MALLOC( Pc * sizeof(int));   // this needs to be optimized for 1D row mapping
    memset(BCis_solved, 0, Pr * sizeof(int));
    memset(RDis_solved, 0, Pc * sizeof(int));
    //foMPI_Win_create(BC_taskq, (BC_buffer_size)*sizeof(double), sizeof(double), MPI_INFO_NULL, col_comm, &bc_winl);
	//foMPI_Win_create(RD_taskq, (RD_buffer_size)*sizeof(double), sizeof(double), MPI_INFO_NULL, row_comm, &rd_winl);
    //foMPI_Win_lock_all(0, bc_winl);
    //foMPI_Win_lock_all(0, rd_winl);
#if ( DEBUGlevel>=1 )
    printf("iam=%d, End setup oneside L solve\n",iam);
	printf("(%2d) nfrecvx %4d,  nfrecvmod %4d,  nleaf %4d\n,  nbtree %4d\n,  nrtree %4d\n",
			iam, nfrecvx, nfrecvmod, nleaf, nbtree, nrtree);
    fflush(stdout);
#endif
#elif defined (pget)
    int iam_col=MYROW( iam, grid );
	int iam_row=MYCOL( iam, grid );


    double nfrecv1=0;
    int ird=0, tidx=0, bcidx=0, rdidx=0, tmp_id=0;

    /* for RDMA flag offset, each meassge has a unqie offset */
	int *BCcount, *RDcount;
    BCcount = (int*)SUPERLU_MALLOC( Pr * sizeof(int));   // this needs to be optimized for 1D row mapping
    RDcount = (int*)SUPERLU_MALLOC( Pc * sizeof(int));   // this needs to be optimized for 1D row mapping
    memset(BCcount, 0, ( Pr * sizeof(int)));
    memset(RDcount, 0, ( Pc * sizeof(int)));

   /* shifting valid sender */
    int totalsolveBC=0, totalsolveRD=0;
    int shift=0;
    int recvRankNum=-1;

    int *validBCQindex;
    int *validRDQindex;
    int *validBCQindex_u;
    int *validRDQindex_u;

    if ( !(validBCQindex = (int*)SUPERLU_MALLOC( Pr * sizeof(int))) )
    	ABORT("Malloc fails for validBCQindex[]");
    if ( !(validRDQindex = (int*)SUPERLU_MALLOC( Pc *sizeof(int))) )
    	ABORT("Malloc fails for validRDQindex[]");
    if ( !(validBCQindex_u = (int*)SUPERLU_MALLOC( Pr * sizeof(int))) )
    	ABORT("Malloc fails for validBCQindex_u[]");
    if ( !(validRDQindex_u = (int*)SUPERLU_MALLOC( Pc *sizeof(int))) )
    	ABORT("Malloc fails for validRDQindex_u[]");

    int *BCbase,*RDbase;
    BCbase = (int*)SUPERLU_MALLOC( Pr * sizeof(int));
    RDbase = (int*)SUPERLU_MALLOC( Pc * sizeof(int));

    if( Pr > 1){
        for (i=0;i<Pr;i++){
                BCbase[i] = recv_size_all[i]*RDMA_FLAG_SIZE;
                validBCQindex[i]=keep_validBCQindex[i];
                validBCQindex_u[i]=keep_validBCQindex_u[i];
#if ( DEBUGlevel>=1 )
                printf("iam=%d,validBCQindex[%d]=%d,validBCQindex_u[%d]=%d, BCbase[%d]=%d\n",iam,i,validBCQindex[i],i,validBCQindex_u[i],i,BCbase[i]);
                fflush(stdout);
#endif
        }
    }
    if(Pc > 1){
            for (i=0;i<Pc;i++){
                    RDbase[i] = recv_size_all[Pr+i]*RDMA_FLAG_SIZE;
                    validRDQindex[i]=keep_validRDQindex[i];
                    validRDQindex_u[i]=keep_validRDQindex_u[i];
#if ( DEBUGlevel>=1 )
                    printf("iam=%d,validRDQindex[%d]=%d,validRDQindex_u[%d]=%d,RDbase[%d]=%d\n",iam,i,validRDQindex[i],i,validRDQindex_u[i],i,RDbase[i]);
                    fflush(stdout);
#endif
            }
    }

    int *mylocal_BCbase,*mylocal_RDbase;
    mylocal_BCbase = (int*)SUPERLU_MALLOC( Pr * sizeof(int));
    mylocal_RDbase = (int*)SUPERLU_MALLOC( Pc * sizeof(int));
    memset(mylocal_BCbase, 0, ( Pr * sizeof(int)));
    memset(mylocal_RDbase, 0, ( Pc * sizeof(int)));

    for (i=0; i<Pr;i++){
             for(j=0;j<i;j++){
                     //oneside_buf_offset[i]: When rank i put flags to my flag buffer, the relative offet is oneside_buf_offset[i].
                     // Eg., RDMA_FLAG_SIZE*oneside_buf_offset[i] is the real offset to get flag from rank i
                     mylocal_BCbase[i] += BufSize[j]*RDMA_FLAG_SIZE;
             }
#if ( DEBUGlevel>=1 )
             printf("In solve, iam=%d, iam_col=%d, L mylocal_BCbase[%d]=%d\n",iam, iam_col,i,mylocal_BCbase[i]);
             fflush(stdout);
#endif
     }


    for (i=0; i<Pc;i++){
             for(j=0;j<i;j++){
                     //oneside_buf_offset[i]: When rank i put flags to my flag buffer, the relative offet is oneside_buf_offset[i].
                     // Eg., RDMA_FLAG_SIZE*oneside_buf_offset[i] is the real offset to get flag from rank i
                     mylocal_RDbase[i] += BufSize_rd[j]*RDMA_FLAG_SIZE;
             }
#if ( DEBUGlevel>=1 )
             printf("In solve, iam=%d, iam_row=%d, L mylocal_RDbase[%d]=%d\n",iam, iam_row,i,mylocal_RDbase[i]);
             fflush(stdout);
#endif
     }
	nfrecvx_buf=0;
/* tracking solved tasks */
    int *BCis_solved, *RDis_solved;
    BCis_solved = (int*)SUPERLU_MALLOC( Pr * sizeof(int));   // this needs to be optimized for 1D row mapping
    RDis_solved = (int*)SUPERLU_MALLOC( Pc * sizeof(int));   // this needs to be optimized for 1D row mapping
    memset(BCis_solved, 0, Pr * sizeof(int));
    memset(RDis_solved, 0, Pc * sizeof(int));


    for (i=0;i< RDMA_FLAG_SIZE * (nfrecvx>nbrecvx?nfrecvx:nbrecvx);i++) bc_pget_count[i] = -1;
    for (i=0;i< RDMA_FLAG_SIZE * (nfrecvmod>nbrecvmod?nfrecvmod:nbrecvmod);i++) rd_pget_count[i] = -1;

    int get_msgsize=0;
    int get_offset=0;
    int get_fromwhere=0; //1=x/lsum 2=tmp_buf_bc/tmp_buf_rd
    int mycur_offset=0, mynext_offset=0;
    int mycur_offset_rd=0, mynext_offset_rd=0;

    int* bc_rdma_start;
    int* rd_rdma_start;

    bc_rdma_start = (int*)SUPERLU_MALLOC( RDMA_FLAG_SIZE* (mysendmsg_num>mysendmsg_num_u?mysendmsg_num:mysendmsg_num_u) * sizeof(int));   // this needs to be optimized for 1D row mapping
    rd_rdma_start = (int*)SUPERLU_MALLOC( RDMA_FLAG_SIZE* (mysendmsg_num_rd>mysendmsg_num_urd?mysendmsg_num_rd:mysendmsg_num_urd) * sizeof(int));   // this needs to be optimized for 1D row mapping

    int mylocal_bc_put_flag_offset=0;
    int mylocal_rd_put_flag_offset=0;


#if ( DEBUGlevel>=1 )
    printf("iam=%d, End setup oneside L solve\n",iam);
    fflush(stdout);
	printf("(%2d) nfrecvx %4d,  nfrecvmod %4d,  nleaf %4d\n,  nbtree %4d\n,  nrtree %4d\n",
			iam, nfrecvx, nfrecvmod, nleaf, nbtree, nrtree);
    fflush(stdout);
#endif
#else

        if ( !(recvbuf_BC_fwd = (double*)SUPERLU_MALLOC(maxrecvsz*(nfrecvx+1) * sizeof(double))) )  // this needs to be optimized for 1D row 		ABORT("Malloc fails for recvbuf_BC_fwd[].");
		ABORT("Malloc fails for recvbuf_BC_fwd[].");
    	nfrecvx_buf=0;
			//PROFILE_DYNAMIC_FINISH();
#endif


#if ( DEBUGlevel>=2 )
	printf("(%2d) nfrecvx %4d,  nfrecvmod %4d,  nleaf %4d\n,  nbtree %4d\n,  nrtree %4d\n",
			iam, nfrecvx, nfrecvmod, nleaf, nbtree, nrtree);
	fflush(stdout);
#endif

#if ( PRNTlevel>=1 )
	t = SuperLU_timer_() - t;
	if ( !iam) printf(".. Setup L-solve time\t%8.4f\n", t);
	fflush(stdout);
	MPI_Barrier( grid->comm );
	t = SuperLU_timer_() ;
#endif

#if ( VAMPIR>=1 )
	// VT_initialize();
	VT_traceon();
#endif

#ifdef USE_VTUNE
	__SSC_MARK(0x111);// start SDE tracing, note uses 2 underscores
	__itt_resume(); // start VTune, again use 2 underscores
#endif

	/* ---------------------------------------------------------
	   Solve the leaf nodes first by all the diagonal processes.
	   --------------------------------------------------------- */
#if ( DEBUGlevel>=2 )
	printf("(%2d) nleaf %4d\n", iam, nleaf);
	fflush(stdout);
#endif


#ifdef _OPENMP
#pragma omp parallel default (shared)
#endif
	{
		{

if(Llu->inv == 1){

#ifdef _OPENMP
#pragma	omp	for firstprivate(nrhs,beta,alpha,x,rtemp,ldalsum) private (ii,k,knsupc,lk,luptr,lsub,nsupr,lusup,t1,t2,Linv,i,lib,rtemp_loc,nleaf_send_tmp) nowait
#endif
			for (jj=0;jj<nleaf;jj++){
				k=leafsups[jj];

				// #ifdef _OPENMP
				// #pragma	omp	task firstprivate (k,nrhs,beta,alpha,x,rtemp,ldalsum) private (ii,knsupc,lk,luptr,lsub,nsupr,lusup,thread_id,t1,t2,Linv,i,lib,rtemp_loc)
				// #endif
				{

#if ( PROFlevel>=1 )
					TIC(t1);
#endif
					rtemp_loc = &rtemp[sizertemp* thread_id];


					knsupc = SuperSize( k );
					lk = LBi( k, grid );

					ii = X_BLK( lk );
					lk = LBj( k, grid ); /* Local block number, column-wise. */
					lsub = Lrowind_bc_ptr[lk];
					lusup = Lnzval_bc_ptr[lk];

					nsupr = lsub[1];

					Linv = Linv_bc_ptr[lk];
#ifdef _CRAY
					SGEMM( ftcs2, ftcs2, &knsupc, &nrhs, &knsupc,
							&alpha, Linv, &knsupc, &[ii],
							&knsupc, &beta, rtemp_loc, &knsupc );
#elif defined (USE_VENDOR_BLAS)
					dgemm_( "N", "N", &knsupc, &nrhs, &knsupc,
							&alpha, Linv, &knsupc, &x[ii],
							&knsupc, &beta, rtemp_loc, &knsupc, 1, 1 );
#else
					dgemm_( "N", "N", &knsupc, &nrhs, &knsupc,
							&alpha, Linv, &knsupc, &x[ii],
							&knsupc, &beta, rtemp_loc, &knsupc );
#endif	

#ifdef _OPENMP
	#pragma omp simd
#endif		   
					for (i=0 ; i<knsupc*nrhs ; i++){
						x[ii+i] = rtemp_loc[i];
					}		
					
					// for (i=0 ; i<knsupc*nrhs ; i++){
					// printf("x_l: %f\n",x[ii+i]);
					// fflush(stdout);
					// }


#if ( PROFlevel>=1 )
					TOC(t2, t1);
					stat_loc[thread_id]->utime[SOL_TRSM] += t2;

#endif	

					stat_loc[thread_id]->ops[SOLVE] += knsupc * (knsupc - 1) * nrhs;
			
					
					// --nleaf;
#if ( DEBUGlevel>=2 )
					printf("(%2d) Solve X[%2d]\n", iam, k);
#endif

					/*
					 * Send Xk to process column Pc[k].
					 */

					if(LBtree_ptr[lk]!=NULL){ 
						lib = LBi( k, grid ); /* Local block number, row-wise. */
						ii = X_BLK( lib );	

#ifdef _OPENMP
#pragma omp atomic capture
#endif
						nleaf_send_tmp = ++nleaf_send;
						leaf_send[(nleaf_send_tmp-1)*aln_i] = lk;
						// BcTree_forwardMessageSimple(LBtree_ptr[lk],&x[ii - XK_H],'d');
					}
				}		
				}
		}else{
#ifdef _OPENMP
#pragma	omp	for firstprivate (nrhs,beta,alpha,x,rtemp,ldalsum) private (ii,k,knsupc,lk,luptr,lsub,nsupr,lusup,t1,t2,Linv,i,lib,rtemp_loc,nleaf_send_tmp) nowait	
#endif
			for (jj=0;jj<nleaf;jj++){
				k=leafsups[jj];
				{

#if ( PROFlevel>=1 )
					TIC(t1);
#endif	 
					rtemp_loc = &rtemp[sizertemp* thread_id];


					knsupc = SuperSize( k );
					lk = LBi( k, grid );

					ii = X_BLK( lk );
					lk = LBj( k, grid ); /* Local block number, column-wise. */
					lsub = Lrowind_bc_ptr[lk];
					lusup = Lnzval_bc_ptr[lk];

					nsupr = lsub[1];


#ifdef _CRAY
					STRSM(ftcs1, ftcs1, ftcs2, ftcs3, &knsupc, &nrhs, &alpha,
							lusup, &nsupr, &x[ii], &knsupc);
#elif defined (USE_VENDOR_BLAS)
					dtrsm_("L", "L", "N", "U", &knsupc, &nrhs, &alpha, 
							lusup, &nsupr, &x[ii], &knsupc, 1, 1, 1, 1);	
#else
					dtrsm_("L", "L", "N", "U", &knsupc, &nrhs, &alpha, 
							lusup, &nsupr, &x[ii], &knsupc);
#endif
				
					
					// for (i=0 ; i<knsupc*nrhs ; i++){
					// printf("x_l: %f\n",x[ii+i]);
					// fflush(stdout);
					// }


#if ( PROFlevel>=1 )
					TOC(t2, t1);
					stat_loc[thread_id]->utime[SOL_TRSM] += t2;

#endif	

					stat_loc[thread_id]->ops[SOLVE] += knsupc * (knsupc - 1) * nrhs;
			
					
					// --nleaf;
#if ( DEBUGlevel>=2 )
					printf("(%2d) Solve X[%2d]\n", iam, k);
#endif

					/*
					 * Send Xk to process column Pc[k].
					 */

					if(LBtree_ptr[lk]!=NULL){ 
						lib = LBi( k, grid ); /* Local block number, row-wise. */
						ii = X_BLK( lib );	

#ifdef _OPENMP
#pragma omp atomic capture
#endif
						nleaf_send_tmp = ++nleaf_send;
						leaf_send[(nleaf_send_tmp-1)*aln_i] = lk;
					}
				}		
				}		
			}						
		}	
	}


		jj=0;
#ifdef _OPENMP
#pragma omp parallel default (shared)
#endif
		{


#ifdef _OPENMP
#pragma omp master
#endif
				{

#ifdef _OPENMP
#pragma	omp	taskloop private (k,ii,lk) num_tasks(num_thread*8) nogroup
#endif

					for (jj=0;jj<nleaf;jj++){
						k=leafsups[jj];		

						{
							/* Diagonal process */
							lk = LBi( k, grid );
							ii = X_BLK( lk );
							/*
							 * Perform local block modifications: lsum[i] -= L_i,k * X[k]
							 */
							dlsum_fmod_inv(lsum, x, &x[ii], rtemp, nrhs, k,
									fmod, xsup, grid, Llu, 
									stat_loc, leaf_send, &nleaf_send,sizelsum,sizertemp,0,maxsuper,thread_id,num_thread);	
						}

						// } /* if diagonal process ... */
					} /* for k ... */
				}

			}
    //double t100;
    //t100= SuperLU_timer_();

			for (i=0;i<nleaf_send;i++){
				lk = leaf_send[i*aln_i];
				if(lk>=0){ // this is a bcast forwarding
					gb = mycol+lk*grid->npcol;  /* not sure */
					lib = LBi( gb, grid ); /* Local block number, row-wise. */
					ii = X_BLK( lib );			
#ifdef oneside
//#if ( PROFlevel>=1 )
//						TIC(t1);
//#endif
                    BcTree_forwardMessageOneSide(LBtree_ptr[lk],&x[ii - XK_H],BcTree_GetMsgSize(LBtree_ptr[lk],'d')*nrhs+XK_H,'d', &iam_col, BCcount, BCbase, &maxrecvsz,Pc);
//#if ( PROFlevel>=1 )
//						TOC(t2, t1);
//						stat_loc[thread_id]->utime[SOL_COMM] += t2;
//#endif
#elif defined (pget)
#if ( PROFlevel>=1 )
						TIC(t1);
#endif
                    bc_rdma_start[mylocal_bc_put_flag_offset]=ii-XK_H;
                    bc_rdma_start[mylocal_bc_put_flag_offset+1]=BcTree_GetMsgSize(LBtree_ptr[lk],'d')*nrhs+XK_H;
                    bc_rdma_start[mylocal_bc_put_flag_offset+2]=1;
                    //printf("In BC leaf node forward (%d),mylocal_bc_put_flag_offset=%d, bc_rdma_start=%d,%d,%d\n",iam,mylocal_bc_put_flag_offset,bc_rdma_start[mylocal_bc_put_flag_offset],bc_rdma_start[mylocal_bc_put_flag_offset+1],bc_rdma_start[mylocal_bc_put_flag_offset+2]);
		            //fflush(stdout);
		            BcTree_forwardMessageOneSide(LBtree_ptr[lk],'d',&bc_rdma_start[mylocal_bc_put_flag_offset], Pc, BCcount, BCbase);
                    mylocal_bc_put_flag_offset += RDMA_FLAG_SIZE;
#if ( PROFlevel>=1 )
						TOC(t2, t1);
						stat_loc[thread_id]->utime[SOL_COMM] += t2;
#endif
#else
					BcTree_forwardMessageSimple(LBtree_ptr[lk],&x[ii - XK_H],BcTree_GetMsgSize(LBtree_ptr[lk],'d')*nrhs+XK_H,'d');
#endif
				}else{ // this is a reduce forwarding
					lk = -lk - 1;
					il = LSUM_BLK( lk );
#ifdef oneside 
//#if ( PROFlevel>=1 )
//						TIC(t1);
//#endif
                    RdTree_forwardMessageOneSide(LRtree_ptr[lk],&lsum[il - LSUM_H ],RdTree_GetMsgSize(LRtree_ptr[lk],'d')*nrhs+LSUM_H,'d', &iam_row, RDcount, RDbase, &maxrecvsz, Pc);
//#if ( PROFlevel>=1 )
//						TOC(t2, t1);
//						stat_loc[thread_id]->utime[SOL_COMM] += t2;
//#endif
#elif defined (pget)
 #if ( PROFlevel>=1 )
						TIC(t1);
#endif
                    rd_rdma_start[mylocal_rd_put_flag_offset+0]=il-LSUM_H;
                    rd_rdma_start[mylocal_rd_put_flag_offset+1]=RdTree_GetMsgSize(LRtree_ptr[lk],'d')*nrhs+LSUM_H;
                    rd_rdma_start[mylocal_rd_put_flag_offset+2]=1;

                    RdTree_forwardMessageOneSide(LRtree_ptr[lk],'d',&rd_rdma_start[mylocal_rd_put_flag_offset], Pc, RDcount,RDbase);
                    mylocal_rd_put_flag_offset += RDMA_FLAG_SIZE;
#if ( PROFlevel>=1 )
						TOC(t2, t1);
						stat_loc[thread_id]->utime[SOL_COMM] += t2;
#endif
#else
					RdTree_forwardMessageSimple(LRtree_ptr[lk],&lsum[il - LSUM_H ],RdTree_GetMsgSize(LRtree_ptr[lk],'d')*nrhs+LSUM_H,'d');
#endif 
				}
			}


#ifdef USE_VTUNE
	__itt_pause(); // stop VTune
	__SSC_MARK(0x222); // stop SDE tracing
#endif

			/* -----------------------------------------------------------
			   Compute the internal nodes asynchronously by all processes.
			   ----------------------------------------------------------- */
#ifdef oneside
    while( nfrecv1 < nfrecvx+nfrecvmod ){
        thread_id = 0;
        if (totalsolveBC < nfrecvx){
	        shift=0;
            for (bcidx=0;bcidx<Pr && validBCQindex[bcidx]!=-1;bcidx++){

                recvRankNum=validBCQindex[bcidx];  //bcidx; //validBCQindex[bcidx];
                i=BC_taskbuf_offset[recvRankNum]+BCis_solved[recvRankNum]*maxrecvsz; //BCis_solved[bcidx];	
                recvbuf0 = &BC_taskq[i];
                k = *recvbuf0;
                
#if ( PROFlevel>=1 )
						TIC(t1);
#endif
                if (k < 0)  {
                   if(shift>0){
                        validBCQindex[bcidx-shift]=validBCQindex[bcidx];
                        validBCQindex[bcidx]=-1;
                        //printf("iam=%d,Now shift %d to %d\n",iam,bcidx,bcidx-shift);
                        //fflush(stdout);
                   }
                   continue;
                }  

                lk = LBj( k, grid );    /* local block number */
                
                checkend=BcTree_GetMsgSize(LBtree_ptr[lk],'d')*nrhs;
                crc_16_val=crc_16((unsigned char*)&recvbuf0[XK_H],sizeof(double)*checkend);
                //myhash=calcul_hash(&recvbuf0[XK_H],sizeof(double)*checkend);
                if(crc_16_val!=(uint16_t)recvbuf0[LSUM_H-1]) {
                    if(shift>0){
                        validBCQindex[bcidx-shift]=validBCQindex[bcidx];
                        validBCQindex[bcidx]=-1;
                        //printf("iam=%d,Now shift %d to %d\n",iam,bcidx,bcidx-shift);
                        //fflush(stdout);
                   }
                   continue;
                }
#if ( PROFlevel>=1 )
						TOC(t2, t1);
						stat_loc[thread_id]->utime[SOL_COMM] += t2;
#endif
                    //t= SuperLU_timer_();
                
                totalsolveBC += 1; //BC_subtotal[bcidx] - BCis_solved[bcidx];
			    BCis_solved[recvRankNum]++;
                        

                if(BcTree_getDestCount(LBtree_ptr[lk],'d')>0){
	                //BcTree_forwardMessageOneSide(LBtree_ptr[lk],recvbuf0,checkend,'d', &iam_col, BCcount, BCbase, &maxrecvsz, Pc);
	                BcTree_forwardMessageOneSide(LBtree_ptr[lk],recvbuf0,BcTree_GetMsgSize(LBtree_ptr[lk],'d')*nrhs+XK_H,'d', &iam_col, BCcount, BCbase, &maxrecvsz, Pc);
			    }
		        lsub = Lrowind_bc_ptr[lk];
                //printf("In BC solve, iam %d, k=%d, lk=%d, lsub =%d\n", iam, k, lk, lsub);
                //fflush(stdout);
		        
                 if ( lsub ) {
                         krow = PROW( k, grid );
		                 if(myrow==krow){
		         	        nb = lsub[0] - 1;
		                     knsupc = SuperSize( k );
		                     ii = X_BLK( LBi( k, grid ) );
		           	        xin = &x[ii];
				         }else{
		                         nb   = lsub[0];
		                         knsupc = SuperSize( k );
				                 xin = &recvbuf0[XK_H] ;					
				         }
		                dlsum_fmod_inv_master(lsum, x, xin, rtemp, nrhs, knsupc, k,
			        	              fmod, nb, xsup, grid, Llu,
			                	      stat_loc,sizelsum,sizertemp,0,maxsuper,thread_id,num_thread, 
			                	      &iam_row, RDcount, RDbase, &iam_col, BCcount, BCbase, Pc, maxrecvsz);
			    } /* if lsub */
              
               if (BCis_solved[recvRankNum] == BufSize[recvRankNum]) {
                   validBCQindex[bcidx]=-1; 
                   shift += 1;
                   //printf("iam=%d,shift=%d\n",iam,shift);
                   //fflush(stdout);
               }else{
                   if(shift>0){
                        validBCQindex[bcidx-shift]=validBCQindex[bcidx];
                        validBCQindex[bcidx]=-1;
                        //printf("iam=%d,Now shift %d to %d\n",iam,bcidx,bcidx-shift);
                        //fflush(stdout);
                   }
               }
               //printf("iam=%d,BCis_solved[%d]=%d,BufSize[%d]=%d\n",iam,recvRankNum,BCis_solved[recvRankNum],recvRankNum,BufSize[recvRankNum]);
               //fflush(stdout);
            } // for bcidx 

                //TOC(t2, t1);
                //onesidecomm_rd += t2;
        } 
       
    if (totalsolveRD < nfrecvmod){
        shift=0;
       //for (rdidx=0;rdidx<Pc ;rdidx++){
       for (rdidx=0;rdidx<Pc && validRDQindex[rdidx]!=-1;rdidx++){
                //if (validRDQindex[rdidx]==-1) continue;
                //if (rdidx == iam_row) continue;
                //if (BufSize_rd[rdidx] == 0) continue;
                //if(RDis_solved[rdidx] == BufSize_rd[rdidx]) continue;
                
                recvRankNum=validRDQindex[rdidx];  //bcidx; //validBCQindex[bcidx];
                ird=RD_taskbuf_offset[recvRankNum]+RDis_solved[recvRankNum]*maxrecvsz;
                recvbuf0 = &RD_taskq[ird];
                k = *recvbuf0;
                //printf("rdrd--111--iam=%d, rdidx=%d,k=%d\n",iam,rdidx,k);
                //fflush(stdout);
#if ( PROFlevel>=1 )
						TIC(t1);
#endif
	            if (k < 0)  {
                   if(shift>0){
                        validRDQindex[rdidx-shift]=validRDQindex[rdidx];
                        validRDQindex[rdidx]=-1;
                   }
                   continue;
                }    
                lk = LBi( k, grid );
                
                checkend=RdTree_GetMsgSize(LRtree_ptr[lk],'d')*nrhs;
                crc_16_val=crc_16((unsigned char*)&recvbuf0[LSUM_H],sizeof(double)*checkend);
                //myhash=calcul_hash(&recvbuf0[LSUM_H],sizeof(double)*checkend);
                ////printf("rdrd--333--iam=%d, checksum=%lf,should be %lf\n",iam, sum, recvbuf0[LSUM_H-1]);
                ////fflush(stdout);
                //
                if(crc_16_val!=(uint16_t)recvbuf0[LSUM_H-1]) {
               //// if((recvbuf0[checkend-1]-recvbuf0[LSUM_H-1])!=0.0) {
                ////if(myhash!=(unsigned int)recvbuf0[XK_H-1]) {
                //if((myhash-recvbuf0[LSUM_H-1])!=0.0) {
                   if(shift>0){
                        validRDQindex[rdidx-shift]=validRDQindex[rdidx];
                        validRDQindex[rdidx]=-1;
                   }
                   continue;
                }
#if ( PROFlevel>=1 )
						TOC(t2, t1);
						stat_loc[thread_id]->utime[SOL_COMM] += t2;
#endif
	            //t = SuperLU_timer_();
                totalsolveRD += 1; //RD_subtotal[rdidx]-RDis_solved[rdidx];
                
                RDis_solved[recvRankNum] += 1 ;	
	            
                knsupc = SuperSize( k );
                tempv = &recvbuf0[LSUM_H];
                il = LSUM_BLK( lk );		  
                RHS_ITERATE(j) {
                               for (i = 0; i < knsupc; ++i)
                                        lsum[i + il + j*knsupc + thread_id*sizelsum] += tempv[i + j*knsupc];
                }			

                       fmod_tmp=--fmod[lk*aln_i];
                
	                   thread_id = 0;
	                   rtemp_loc = &rtemp[sizertemp* thread_id];
                       //printf("5----iam=%d,k=%d\n",iam,k);
                       //fflush(stdout);
	                   if ( fmod_tmp==0 ) {	  
                            //printf("6----iam=%d,k=%d\n",iam,k);
                            //fflush(stdout);
		                    if(RdTree_IsRoot(LRtree_ptr[lk],'d')==YES){
                                knsupc = SuperSize( k );
                                //printf("7----iam=%d,k=%d\n",iam,k);
                                //fflush(stdout);
                                for (ii=1;ii<num_thread;ii++)
#ifdef _OPENMP
	#pragma omp simd
#endif	
 				                for (jj=0;jj<knsupc*nrhs;jj++)
					                lsum[il + jj ] += lsum[il + jj + ii*sizelsum];

				                ii = X_BLK( lk );
			        	        RHS_ITERATE(j){
#ifdef _OPENMP
	#pragma omp simd
#endif												
			  	                for (i = 0; i < knsupc; ++i)	
				        	        x[i + ii + j*knsupc] += lsum[i + il + j*knsupc ];
                                        }
        		        	    lk = LBj( k, grid ); /* Local block number, column-wise. */
	                         	lsub = Lrowind_bc_ptr[lk];
	        	                lusup = Lnzval_bc_ptr[lk];
	        	        	    nsupr = lsub[1];
        
#if ( PROFlevel>=1 )
	 		        	        TIC(t1);
#endif

       				            if(Llu->inv == 1){
	        			            Linv = Linv_bc_ptr[lk];		  
#ifdef _CRAY
		        			        SGEMM( ftcs2, ftcs2, &knsupc, &nrhs, &knsupc,
				        		       	&alpha, Linv, &knsupc, &x[ii],
						              	&knsupc, &beta, rtemp_loc, &knsupc );
#elif defined (USE_VENDOR_BLAS)
						            dgemm_( "N", "N", &knsupc, &nrhs, &knsupc,
						      	        &alpha, Linv, &knsupc, &x[ii],
							            &knsupc, &beta, rtemp_loc, &knsupc, 1, 1 );
#else
			        	            dgemm_( "N", "N", &knsupc, &nrhs, &knsupc,
					        	        &alpha, Linv, &knsupc, &x[ii],
				        		        &knsupc, &beta, rtemp_loc, &knsupc );
#endif				   
#ifdef _OPENMP
	#pragma omp simd	
#endif
						            for (i=0 ; i<knsupc*nrhs ; i++){
				                        x[ii+i] = rtemp_loc[i];
					                }		
                	            }else{   //if(Llu->inv == 1)
#ifdef _CRAY
					                STRSM(ftcs1, ftcs1, ftcs2, ftcs3, &knsupc, &nrhs, &alpha,
					        	        lusup, &nsupr, &x[ii], &knsupc);
#elif defined (USE_VENDOR_BLAS)
					       	        dtrsm_("L", "L", "N", "U", &knsupc, &nrhs, &alpha, 
					        	        lusup, &nsupr, &x[ii], &knsupc, 1, 1, 1, 1);		
#else
					     	        dtrsm_("L", "L", "N", "U", &knsupc, &nrhs, &alpha, 
					       		        lusup, &nsupr, &x[ii], &knsupc);
#endif
					            } // end if (Llu->inv == 1)

#if ( PROFlevel>=1 )
          				        TOC(t2, t1);
          				        stat_loc[thread_id]->utime[SOL_TRSM] += t2;
#endif

			                    stat_loc[thread_id]->ops[SOLVE] += knsupc * (knsupc - 1) * nrhs;

                                //printf("8----iam=%d,k=%d\n",iam,k);
                                //fflush(stdout);

               				    if(LBtree_ptr[lk]!=NULL){ 
                                    //printf("9----iam=%d,k=%d\n",iam,k);
                                    //fflush(stdout);
       				        	    BcTree_forwardMessageOneSide(LBtree_ptr[lk],&x[ii - XK_H],BcTree_GetMsgSize(LBtree_ptr[lk],'d')*nrhs+XK_H,'d',  &iam_col, BCcount, BCbase, &maxrecvsz,Pc); 
                                    //printf("10----iam=%d,k=%d\n",iam,k);
                                    //fflush(stdout);
			        	        }		  
               
               				    lk = LBj( k, grid ); /* Local block number, column-wise. */
               				    lsub = Lrowind_bc_ptr[lk];
               				    lusup = Lnzval_bc_ptr[lk];
               				    if ( lsub ) {
               				 	    krow = PROW( k, grid );
               				 	    nb = lsub[0] - 1;
               				        knsupc = SuperSize( k );
               				 	    ii = X_BLK( LBi( k, grid ) );
               				 	    xin = &x[ii];		
                                    //printf("11----iam=%d,k=%d\n",iam,k);
                                    //fflush(stdout);
               				        dlsum_fmod_inv_master(lsum, x, xin, rtemp, nrhs, knsupc, k,
               				 	        	fmod, nb, xsup, grid, Llu,
               				 		        stat_loc,sizelsum,sizertemp,0,maxsuper,thread_id,num_thread,
                                            &iam_row, RDcount, RDbase, &iam_col, BCcount, BCbase, Pc, maxrecvsz);
                                    //printf("12----iam=%d,k=%d\n",iam,k);
                                    //fflush(stdout);
               			        } /* if lsub */
                        }else{ // RdTree Yes
                            //printf("13----iam=%d,k=%d\n",iam,k);
                            //fflush(stdout);
				       	    il = LSUM_BLK( lk );		  
			              	knsupc = SuperSize( k );

			      	        for (ii=1;ii<num_thread;ii++)
#ifdef _OPENMP
#pragma omp simd
#endif										
			     	                  for (jj=0;jj<knsupc*nrhs;jj++)
			                               	lsum[il + jj] += lsum[il + jj + ii*sizelsum];
                    //printf("14----iam=%d,k=%d\n",iam,k);
                    //fflush(stdout);
					RdTree_forwardMessageOneSide(LRtree_ptr[lk],&lsum[il-LSUM_H],RdTree_GetMsgSize(LRtree_ptr[lk],'d')*nrhs+LSUM_H,'d', &iam_row, RDcount, RDbase, &maxrecvsz, Pc);
                    //printf("15----iam=%d,k=%d\n",iam,k);
                    //fflush(stdout);
                    } // end if RD xxxx YES
			} // end of fmod_tmp=0 
            if (RDis_solved[recvRankNum] == BufSize_rd[recvRankNum]) {
                validRDQindex[rdidx]=-1; 
                shift += 1;
                //printf("iam=%d,shift=%d\n",iam,shift);
                //fflush(stdout);
            }else{
                if(shift>0){
                     validRDQindex[rdidx-shift]=validRDQindex[rdidx];
                     validRDQindex[rdidx]=-1;
                     //printf("iam=%d,Now shift %d to %d\n",iam,bcidx,bcidx-shift);
                     //fflush(stdout);
                }
            }
            //printf("iam=%d,RDis_solved[%d]=%d,BufSize_rd[%d]=%d\n",iam,recvRankNum,RDis_solved[recvRankNum],recvRankNum,BufSize_rd[recvRankNum]); 
            //fflush(stdout);
        }// for (rdidx=0;rdidx<Pc;rdidx++)
        }
        nfrecv1 = totalsolveBC + totalsolveRD;
}// outer-most while

//printf("Iam %d OUT!!!\n",iam);
//fflush(stdout);

#elif defined (pget)
//printf("Will begin pget while\n");
//fflush(stdout);
int tmp_counter=0;
while( nfrecv1 < nfrecvx+nfrecvmod ){
    thread_id = 0;
    if (totalsolveBC < nfrecvx){
        shift=0;
        for (bcidx=0;bcidx<Pr && validBCQindex[bcidx]!=-1;bcidx++){

            recvRankNum=validBCQindex[bcidx];  //recvRankNum is rank no. in col/row sub-communicator
            int tmp_idx=mylocal_BCbase[recvRankNum]+BCis_solved[recvRankNum]*RDMA_FLAG_SIZE;
            //printf("In BC solve, iam %d will get from %d, mylocal_BCbase=%d, BCis_solved=%d\n",iam,recvRankNum,mylocal_BCbase[bcidx],BCis_solved[recvRankNum]);
            //fflush(stdout);
            if ((bc_pget_count[tmp_idx] == -1) || (bc_pget_count[tmp_idx+1] == -1) || (bc_pget_count[tmp_idx+2] == -1))  {

                if(shift>0){
                    validBCQindex[bcidx-shift]=validBCQindex[bcidx];
                    validBCQindex[bcidx]=-1;
                    //printf("iam=%d,Now shift %d to %d\n",iam,bcidx,bcidx-shift);
                    //fflush(stdout);
                }
               continue;
            } else {
                get_offset=bc_pget_count[tmp_idx];
                get_msgsize=bc_pget_count[tmp_idx+1];
                get_fromwhere=bc_pget_count[tmp_idx+2];
                //printf("In BC solve (%d),  %d,get from %d, get_offset=%d, get_msgsize=%d, get_fromwhere=%d,mylocal_BCbase=%d, BCis_solved=%d \n",tmp_counter,iam,recvRankNum,get_offset,get_msgsize, get_fromwhere,mylocal_BCbase[bcidx],BCis_solved[recvRankNum]);
                //fflush(stdout);

                mycur_offset=mynext_offset;

                if (get_fromwhere==1){
                    //printf("In BC solve (%d), iam %d in 1, mycur_offset=%d,get_msgsize=%d\n", tmp_counter, iam,mycur_offset,get_msgsize);
                    //fflush(stdout);
                    //foMPI_Win_lock(foMPI_LOCK_SHARED,recvRankNum,0,bc_winl_get);
                    //foMPI_Win_lock(foMPI_LOCK_SHARED,recvRankNum,0,bc_winl_get);
#if ( PROFlevel>=1 )
						TIC(t1);
#endif
                    //int foMPI_Get(void *origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, foMPI_Win win);
                    foMPI_Get(&tmp_buf_bc[mycur_offset], get_msgsize, MPI_DOUBLE, recvRankNum, get_offset, get_msgsize, MPI_DOUBLE,bc_winl_get);
                    foMPI_Win_flush_local(recvRankNum,bc_winl_get);
#if ( PROFlevel>=1 )
						TOC(t2, t1);
						stat_loc[thread_id]->utime[SOL_COMM] += t2;
#endif
                    //foMPI_Win_unlock(recvRankNum,bc_winl_get);

                    //printf("In BC solve (%d),iam %d finish get from x\n",tmp_counter,iam);
                    //fflush(stdout);

                }else{
                    //printf("In BC solve (%d), iam %d in 2, get from %d, mycur_offset=%d,get_msgsize=%d\n", tmp_counter, iam,recvRankNum, mycur_offset,get_msgsize);
                    //fflush(stdout);
                    //foMPI_Win_lock(foMPI_LOCK_SHARED,recvRankNum,0,tmp_bc_winl_get);
#if ( PROFlevel>=1 )
						TIC(t1);
#endif
                    //int foMPI_Get(void *origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, foMPI_Win win);
                    foMPI_Get(&tmp_buf_bc[mycur_offset], get_msgsize, MPI_DOUBLE, recvRankNum, get_offset, get_msgsize, MPI_DOUBLE,tmp_bc_winl_get);
                    foMPI_Win_flush_local(recvRankNum,tmp_bc_winl_get);
//                    foMPI_Win_flush_local(recvRankNum, tmp_bc_winl_get);
#if ( PROFlevel>=1 )
						TOC(t2, t1);
						stat_loc[thread_id]->utime[SOL_COMM] += t2;
#endif
                    //foMPI_Win_unlock(recvRankNum,tmp_bc_winl_get);

                    //printf("In BC solve (%d),iam %d finish get from tmp_buf_bc\n",tmp_counter,iam);
                    //fflush(stdout);
                }


                mynext_offset+=get_msgsize;

                //printf("In BC solve (%d), mycur_offset=%d\n", iam,mycur_offset);
                //fflush(stdout);

                k = tmp_buf_bc[mycur_offset];
                lk = LBj( k, grid );    /* local block number */
	            lsub = Lrowind_bc_ptr[lk];

                //printf("In BC solve (%d),  iam %d,mycur_offset=%d, k=%d, lk=%d, lsub =%d\n", tmp_counter,iam, mycur_offset,k, lk, lsub);
                //fflush(stdout);

	            totalsolveBC += 1; //BC_subtotal[bcidx] - BCis_solved[bcidx];
		        BCis_solved[recvRankNum]++;
                //printf("In BC solve (%d), iam %d,BCis_solved[%d]=%d,getDestCount=%d\n",tmp_counter, iam,recvRankNum,BCis_solved[recvRankNum],BcTree_getDestCount(LBtree_ptr[lk],'d'));
                //fflush(stdout);


                //printf("In BC solve (%d), iam %d before getDestCount\n",tmp_counter,iam );
                //fflush(stdout);

                if(BcTree_getDestCount(LBtree_ptr[lk],'d')>0){
                   // printf("In BC solve (%d), iam %d enter getDestCount,mylocal_bc_put_flag_offset=%d,mycur_offset=%d\n",tmp_counter,iam,mylocal_bc_put_flag_offset,mycur_offset);
                   // fflush(stdout);
#if ( PROFlevel>=1 )
						TIC(t1);
#endif
                    bc_rdma_start[mylocal_bc_put_flag_offset]=mycur_offset;
                    bc_rdma_start[mylocal_bc_put_flag_offset+1]=get_msgsize;
                    bc_rdma_start[mylocal_bc_put_flag_offset+2]=2;
                    BcTree_forwardMessageOneSide(LBtree_ptr[lk],'d',&bc_rdma_start[mylocal_bc_put_flag_offset], Pc,BCcount, BCbase);
                    mylocal_bc_put_flag_offset += RDMA_FLAG_SIZE;
#if ( PROFlevel>=1 )
						TOC(t2, t1);
						stat_loc[thread_id]->utime[SOL_COMM] += t2;
#endif
                   // printf("In BC solve (%d), iam %d notify get x ,offset=%d,msgsize=%d\n",tmp_counter, iam, bc_rdma_start[mylocal_bc_put_flag_offset],bc_rdma_start[mylocal_bc_put_flag_offset+1]);
                   // fflush(stdout);

		        }

                //printf("In BC solve (%d), iam %d before if lsub\n",tmp_counter, iam );
                //fflush(stdout);

                if ( lsub ) {
                     krow = PROW( k, grid );
	                 if(myrow==krow){
	         	        nb = lsub[0] - 1;
	                     knsupc = SuperSize( k );
	                     ii = X_BLK( LBi( k, grid ) );
	           	        xin = &x[ii];
			         }else{
	                         nb   = lsub[0];
	                         knsupc = SuperSize( k );
			                 xin = &tmp_buf_bc[mycur_offset+XK_H] ;
			         }
                    //printf("In BC solve (%d), iam %d before dlsum_fmod_inv_master \n",tmp_counter,iam);
                    //fflush(stdout);
	                dlsum_fmod_inv_master(lsum, x, xin, rtemp, nrhs, knsupc, k,
		        	              fmod, nb, xsup, grid, Llu,
		                	      stat_loc,sizelsum,sizertemp,0,maxsuper,thread_id,num_thread,
                                  &iam_row, rd_rdma_start, &iam_col, bc_rdma_start, Pc, maxrecvsz,BCcount, BCbase,RDcount,RDbase,&mylocal_bc_put_flag_offset, &mylocal_rd_put_flag_offset);
                    //printf("In BC solve (%d), iam %d after dlsum_fmod_inv_master \n", tmp_counter,iam);
                    //fflush(stdout);

		        } /* if lsub */

                if (BCis_solved[recvRankNum] == BufSize[recvRankNum]) {
                    validBCQindex[bcidx]=-1;
                    shift += 1;
                    //printf("iam=%d,shift=%d\n",iam,shift);
                    //fflush(stdout);
                }else{
                    if(shift>0){
                        validBCQindex[bcidx-shift]=validBCQindex[bcidx];
                        validBCQindex[bcidx]=-1;
                        //printf("iam=%d,Now shift %d to %d\n",iam,bcidx,bcidx-shift);
                        //fflush(stdout);
                    }
                }
                //printf("In BC solve (%d),iam %d, BCis_solved[%d]=%d,BufSize[%d]=%d\n",tmp_counter,iam,recvRankNum,BCis_solved[recvRankNum],recvRankNum,BufSize[recvRankNum]);
                //fflush(stdout);
                tmp_counter +=1;
             } // if bc_pget_count>0
        } // for bcidx
        //TOC(t2, t1);
        //onesidecomm_rd += t2;
    }

    if (totalsolveRD < nfrecvmod){
       shift=0;
       for (rdidx=0;rdidx<Pc && validRDQindex[rdidx]!=-1;rdidx++){
           recvRankNum=validRDQindex[rdidx];  //bcidx; //validBCQindex[bcidx];
           int tmp_idx=mylocal_RDbase[recvRankNum]+RDis_solved[recvRankNum]*RDMA_FLAG_SIZE;
           if ((rd_pget_count[tmp_idx] == -1) || (rd_pget_count[tmp_idx+1] == -1) || (rd_pget_count[tmp_idx+2] == -1)){
                if(shift>0){
                    validRDQindex[rdidx-shift]=validRDQindex[rdidx];
                    validRDQindex[rdidx]=-1;
                }
              continue;
           } else {
                //printf("In RD solve, iam %d will get from %d, mylocal_RDbase=%d, RDis_solved=%d\n",iam,recvRankNum,mylocal_RDbase[rdidx],RDis_solved[recvRankNum]);
                //fflush(stdout);
                get_offset=rd_pget_count[tmp_idx];
                get_msgsize=rd_pget_count[tmp_idx+1];

                //printf("In RD solve, iam %d get from %d, rd_rdma_offset=%d, get_msgsize=%d\n",iam,recvRankNum,get_offset,get_msgsize);
                //fflush(stdout);
                get_fromwhere=rd_pget_count[tmp_idx+2];
                //printf("In RD solve, iam %d get from %d, get_fromwhere=%d\n",iam,recvRankNum,get_fromwhere);
                //fflush(stdout);
                mycur_offset_rd=mynext_offset_rd;

                if (get_fromwhere==1){
                    //printf("In RD solve, iam %d in 1\n",iam);
                    //fflush(stdout);
                    //int foMPI_Get(void *origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, foMPI_Win win);
                    //foMPI_Win_lock(foMPI_LOCK_EXCLUSIVE,recvRankNum,0,rd_winl_get);
                    //foMPI_Win_lock(foMPI_LOCK_SHARED,recvRankNum,0,rd_winl_get);
 #if ( PROFlevel>=1 )
						TIC(t1);
#endif
                    foMPI_Get(&tmp_buf_rd[mycur_offset_rd], get_msgsize, MPI_DOUBLE, recvRankNum, get_offset, get_msgsize, MPI_DOUBLE,rd_winl_get);
                    foMPI_Win_flush_local(recvRankNum,rd_winl_get);
#if ( PROFlevel>=1 )
						TOC(t2, t1);
						stat_loc[thread_id]->utime[SOL_COMM] += t2;
#endif

                    //foMPI_Win_unlock(recvRankNum,rd_winl_get);
                    //printf("In RD solve, iam=%d,finish get from tmp_buf_rd\n",iam);
                    //fflush(stdout);
                }else{
                    //printf("In RD solve, iam %d in 2\n",iam);
                    //fflush(stdout);
                    //int foMPI_Get(void *origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, foMPI_Win win);
                    //foMPI_Win_lock(foMPI_LOCK_EXCLUSIVE,recvRankNum,0,tmp_rd_winl_get);
                    //foMPI_Win_lock(foMPI_LOCK_SHARED,recvRankNum,0,tmp_rd_winl_get);
 #if ( PROFlevel>=1 )
						TIC(t1);
#endif
                    foMPI_Get(&tmp_buf_rd[mycur_offset_rd], get_msgsize, MPI_DOUBLE, recvRankNum, get_offset, get_msgsize, MPI_DOUBLE,tmp_rd_winl_get);
                    foMPI_Win_flush_local(recvRankNum, tmp_rd_winl_get);
#if ( PROFlevel>=1 )
						TOC(t2, t1);
						stat_loc[thread_id]->utime[SOL_COMM] += t2;
#endif

                    //foMPI_Win_unlock(recvRankNum,tmp_rd_winl_get);
                    //printf("In RD solve, iam=%d,finish get from tmp_buf_rd\n",iam);
                    //fflush(stdout);
                }

                mynext_offset+=get_msgsize;
                k = tmp_buf_rd[mycur_offset_rd];
                //printf("iam %d,Get k=%d\n",iam,k);
                //fflush(stdout);
                lk = LBi( k, grid );
                //printf("iam %d,Get lk=%d\n",iam,lk);
                //fflush(stdout);

                totalsolveRD += 1; //RD_subtotal[rdidx]-RDis_solved[rdidx];
                RDis_solved[recvRankNum] += 1 ;

                knsupc = SuperSize( k );
                tempv = &tmp_buf_rd[mycur_offset_rd+LSUM_H];
                il = LSUM_BLK( lk );
                RHS_ITERATE(j) {
                               for (i = 0; i < knsupc; ++i)
                                        lsum[i + il + j*knsupc + thread_id*sizelsum] += tempv[i + j*knsupc];
                }

                fmod_tmp=--fmod[lk*aln_i];

	            thread_id = 0;
	            rtemp_loc = &rtemp[sizertemp* thread_id];
                //printf("5----iam=%d,k=%d\n",iam,k);
                //fflush(stdout);
	            if ( fmod_tmp==0 ) {
                    //printf("6----iam=%d,k=%d\n",iam,k);
                    //fflush(stdout);
		            if(RdTree_IsRoot(LRtree_ptr[lk],'d')==YES){
                        knsupc = SuperSize( k );
                        //printf("7----iam=%d,k=%d\n",iam,k);
                        //fflush(stdout);
                        for (ii=1;ii<num_thread;ii++)
#ifdef _OPENMP
	#pragma omp simd
#endif
 				            for (jj=0;jj<knsupc*nrhs;jj++)
					            lsum[il + jj ] += lsum[il + jj + ii*sizelsum];

				                ii = X_BLK( lk );
			        	        RHS_ITERATE(j){
#ifdef _OPENMP
	#pragma omp simd
#endif
			  	            for (i = 0; i < knsupc; ++i)
				        	    x[i + ii + j*knsupc] += lsum[i + il + j*knsupc ];
                                    }
        		        	lk = LBj( k, grid ); /* Local block number, column-wise. */
	                        lsub = Lrowind_bc_ptr[lk];
	        	            lusup = Lnzval_bc_ptr[lk];
	        	        	nsupr = lsub[1];

#if ( PROFlevel>=1 )
	 		        	        TIC(t1);
#endif

       				            if(Llu->inv == 1){
	        			            Linv = Linv_bc_ptr[lk];
#ifdef _CRAY
		        			        SGEMM( ftcs2, ftcs2, &knsupc, &nrhs, &knsupc,
				        		       	&alpha, Linv, &knsupc, &x[ii],
						              	&knsupc, &beta, rtemp_loc, &knsupc );
#elif defined (USE_VENDOR_BLAS)
						            dgemm_( "N", "N", &knsupc, &nrhs, &knsupc,
						      	        &alpha, Linv, &knsupc, &x[ii],
							            &knsupc, &beta, rtemp_loc, &knsupc, 1, 1 );
#else
			        	            dgemm_( "N", "N", &knsupc, &nrhs, &knsupc,
					        	        &alpha, Linv, &knsupc, &x[ii],
				        		        &knsupc, &beta, rtemp_loc, &knsupc );
#endif
#ifdef _OPENMP
	#pragma omp simd
#endif
						            for (i=0 ; i<knsupc*nrhs ; i++){
				                        x[ii+i] = rtemp_loc[i];
					                }
                	            }else{   //if(Llu->inv == 1)
#ifdef _CRAY
					                STRSM(ftcs1, ftcs1, ftcs2, ftcs3, &knsupc, &nrhs, &alpha,
					        	        lusup, &nsupr, &x[ii], &knsupc);
#elif defined (USE_VENDOR_BLAS)
					       	        dtrsm_("L", "L", "N", "U", &knsupc, &nrhs, &alpha,
					        	        lusup, &nsupr, &x[ii], &knsupc, 1, 1, 1, 1);
#else
					     	        dtrsm_("L", "L", "N", "U", &knsupc, &nrhs, &alpha,
					       		        lusup, &nsupr, &x[ii], &knsupc);
#endif
					            } // end if (Llu->inv == 1)

#if ( PROFlevel>=1 )
          				        TOC(t2, t1);
          				        stat_loc[thread_id]->utime[SOL_TRSM] += t2;
#endif

			                    stat_loc[thread_id]->ops[SOLVE] += knsupc * (knsupc - 1) * nrhs;

                                //printf("8----iam=%d,k=%d\n",iam,k);
                                //fflush(stdout);

               				    if(LBtree_ptr[lk]!=NULL){
#if ( PROFlevel>=1 )
						TIC(t1);
#endif
                                    bc_rdma_start[mylocal_bc_put_flag_offset]=ii-XK_H;
                                    bc_rdma_start[mylocal_bc_put_flag_offset+1]=BcTree_GetMsgSize(LBtree_ptr[lk],'d')*nrhs+XK_H;
                                    bc_rdma_start[mylocal_bc_put_flag_offset+2]=1;
                                   	//printf("In RD solve, will notify get x ,offset=%d,msgsize=%d\n",bc_rdma_start[0],bc_rdma_start[1]);
                                    //fflush(stdout);
       				        	    BcTree_forwardMessageOneSide(LBtree_ptr[lk],'d', &bc_rdma_start[mylocal_bc_put_flag_offset], Pc, BCcount,BCbase);
                                    mylocal_bc_put_flag_offset += RDMA_FLAG_SIZE;
#if ( PROFlevel>=1 )
						TOC(t2, t1);
						stat_loc[thread_id]->utime[SOL_COMM] += t2;
#endif
                                    //printf("10----iam=%d,k=%d\n",iam,k);
                                    //fflush(stdout);
			        	        }

               				    lk = LBj( k, grid ); /* Local block number, column-wise. */
               				    lsub = Lrowind_bc_ptr[lk];
               				    lusup = Lnzval_bc_ptr[lk];
               				    if ( lsub ) {
               				 	    krow = PROW( k, grid );
               				 	    nb = lsub[0] - 1;
               				        knsupc = SuperSize( k );
               				 	    ii = X_BLK( LBi( k, grid ) );
               				 	    xin = &x[ii];
                                    //printf("iam %d, before dlsum_fmod_inv_master \n",iam);
                                    //fflush(stdout);
               				        dlsum_fmod_inv_master(lsum, x, xin, rtemp, nrhs, knsupc, k,
               				 	        	fmod, nb, xsup, grid, Llu,
               				 		        stat_loc,sizelsum,sizertemp,0,maxsuper,thread_id,num_thread,
                                            &iam_row, rd_rdma_start, &iam_col, bc_rdma_start, Pc, maxrecvsz, BCcount, BCbase, RDcount, RDbase, &mylocal_bc_put_flag_offset, &mylocal_rd_put_flag_offset);
               				        //printf("iam %d, after dlsum_fmod_inv_master \n",iam);
                                    //fflush(stdout);
               			        } /* if lsub */
                        }else{ // RdTree Yes
				       	    il = LSUM_BLK( lk );
			              	knsupc = SuperSize( k );

			      	        for (ii=1;ii<num_thread;ii++)
#ifdef _OPENMP
#pragma omp simd
#endif
			     	                  for (jj=0;jj<knsupc*nrhs;jj++)
			                               	lsum[il + jj] += lsum[il + jj + ii*sizelsum];
#if ( PROFlevel>=1 )
						TIC(t1);
#endif
                    rd_rdma_start[mylocal_rd_put_flag_offset]=il-LSUM_H;
                    rd_rdma_start[mylocal_rd_put_flag_offset+1]=RdTree_GetMsgSize(LRtree_ptr[lk],'d')*nrhs+LSUM_H;
                    rd_rdma_start[mylocal_rd_put_flag_offset+2]=1;
                    //printf("In RD solve, will notify get lsum ,offset=%d,msgsize=%d\n",rd_rdma_start[mylocal_rd_put_flag_offset],rd_rdma_start[mylocal_rd_put_flag_offset+1]);
                    //fflush(stdout);
					RdTree_forwardMessageOneSide(LRtree_ptr[lk],'d',&rd_rdma_start[mylocal_rd_put_flag_offset], Pc, RDcount, RDbase);
                    mylocal_rd_put_flag_offset += RDMA_FLAG_SIZE;
#if ( PROFlevel>=1 )
						TOC(t2, t1);
						stat_loc[thread_id]->utime[SOL_COMM] += t2;
#endif
                    } // end if RD xxxx YES
			} // end of fmod_tmp=0
            if (RDis_solved[recvRankNum] == BufSize_rd[recvRankNum]) {
                validRDQindex[rdidx]=-1;
                shift += 1;
                //printf("iam=%d,shift=%d\n",iam,shift);
                //fflush(stdout);
            }else{
                if(shift>0){
                     validRDQindex[rdidx-shift]=validRDQindex[rdidx];
                     validRDQindex[rdidx]=-1;
                     //printf("iam=%d,Now shift %d to %d\n",iam,bcidx,bcidx-shift);
                     //fflush(stdout);
                }
            }
            //printf("iam=%d,RDis_solved[%d]=%d,BufSize_rd[%d]=%d\n",iam,recvRankNum,RDis_solved[recvRankNum],recvRankNum,BufSize_rd[recvRankNum]);
            //fflush(stdout);
        } // if (d_pget_count[bcdix]>0)
       }// for (rdidx=0;rdidx<Pc;rdidx++)
    }
    nfrecv1 = totalsolveBC + totalsolveRD;
}// outer-most while
#else

#ifdef _OPENMP
#pragma omp parallel default (shared) 
#endif
			{	
#ifdef _OPENMP
#pragma omp master 
#endif
				{									 
					for ( nfrecv =0; nfrecv<nfrecvx+nfrecvmod;nfrecv++) { /* While not finished. */
						thread_id = 0;
#if ( PROFlevel>=1 )
						TIC(t1);
						// msgcnt[1] = maxrecvsz;
#endif	

						recvbuf0 = &recvbuf_BC_fwd[nfrecvx_buf*maxrecvsz];

						/* Receive a message. */
						MPI_Recv( recvbuf0, maxrecvsz, MPI_DOUBLE,
								MPI_ANY_SOURCE, MPI_ANY_TAG, grid->comm, &status );	 	
						// MPI_Irecv(recvbuf0,maxrecvsz,MPI_DOUBLE,MPI_ANY_SOURCE,MPI_ANY_TAG,grid->comm,&req);
						// ready=0;
						// while(ready==0){
						// MPI_Test(&req,&ready,&status);
						// #pragma omp taskyield
						// }

#if ( PROFlevel>=1 )		 
						TOC(t2, t1);
						stat_loc[thread_id]->utime[SOL_COMM] += t2;

						msg_cnt += 1;
						msg_vol += maxrecvsz * dword;			
#endif					  

						{  
							
							k = *recvbuf0;
		
							//printf("(%2d) Recv'd block %d, tag %2d\n", iam, k, status.MPI_TAG);

							if(status.MPI_TAG==BC_L){
								// --nfrecvx;
								nfrecvx_buf++;
								{
									lk = LBj( k, grid );    /* local block number */

									if(BcTree_getDestCount(LBtree_ptr[lk],'d')>0){

										BcTree_forwardMessageSimple(LBtree_ptr[lk],recvbuf0,BcTree_GetMsgSize(LBtree_ptr[lk],'d')*nrhs+XK_H,'d');	
										// nfrecvx_buf++;
									}

									/*
									 * Perform local block modifications: lsum[i] -= L_i,k * X[k]
									 */	  

									lk = LBj( k, grid ); /* Local block number, column-wise. */
									lsub = Lrowind_bc_ptr[lk];
									lusup = Lnzval_bc_ptr[lk];
									if ( lsub ) {
										krow = PROW( k, grid );
										if(myrow==krow){
											nb = lsub[0] - 1;
											knsupc = SuperSize( k );
											ii = X_BLK( LBi( k, grid ) );
											xin = &x[ii];
										}else{
											nb   = lsub[0];
											knsupc = SuperSize( k );
											xin = &recvbuf0[XK_H] ;					
										}

										dlsum_fmod_inv_master(lsum, x, xin, rtemp, nrhs, knsupc, k,
												fmod, nb, xsup, grid, Llu,
												stat_loc,sizelsum,sizertemp,0,maxsuper,thread_id,num_thread);	

									} /* if lsub */
								}

							}else if(status.MPI_TAG==RD_L){
								// --nfrecvmod;		  
								lk = LBi( k, grid ); /* Local block number, row-wise. */

								knsupc = SuperSize( k );
								tempv = &recvbuf0[LSUM_H];
								il = LSUM_BLK( lk );		  
								RHS_ITERATE(j) {
									for (i = 0; i < knsupc; ++i)
										lsum[i + il + j*knsupc + thread_id*sizelsum] += tempv[i + j*knsupc];
										
								}			

								// #ifdef _OPENMP
								// #pragma omp atomic capture
								// #endif
								fmod_tmp=--fmod[lk*aln_i];
								{
									thread_id = 0;
									rtemp_loc = &rtemp[sizertemp* thread_id];
									if ( fmod_tmp==0 ) {	  
										if(RdTree_IsRoot(LRtree_ptr[lk],'d')==YES){
											// ii = X_BLK( lk );
											knsupc = SuperSize( k );
											for (ii=1;ii<num_thread;ii++)
											#ifdef _OPENMP
												#pragma omp simd
											#endif	
												for (jj=0;jj<knsupc*nrhs;jj++)
													lsum[il + jj ] += lsum[il + jj + ii*sizelsum];

											ii = X_BLK( lk );
											RHS_ITERATE(j)
												#ifdef _OPENMP
													#pragma omp simd
												#endif												
												for (i = 0; i < knsupc; ++i)	
													x[i + ii + j*knsupc] += lsum[i + il + j*knsupc ];

											// fmod[lk] = -1; /* Do not solve X[k] in the future. */
											lk = LBj( k, grid ); /* Local block number, column-wise. */
											lsub = Lrowind_bc_ptr[lk];
											lusup = Lnzval_bc_ptr[lk];
											nsupr = lsub[1];

#if ( PROFlevel>=1 )
											TIC(t1);
#endif			  

											if(Llu->inv == 1){
												Linv = Linv_bc_ptr[lk];		  
#ifdef _CRAY
												SGEMM( ftcs2, ftcs2, &knsupc, &nrhs, &knsupc,
														&alpha, Linv, &knsupc, &x[ii],
														&knsupc, &beta, rtemp_loc, &knsupc );
#elif defined (USE_VENDOR_BLAS)
												dgemm_( "N", "N", &knsupc, &nrhs, &knsupc,
														&alpha, Linv, &knsupc, &x[ii],
														&knsupc, &beta, rtemp_loc, &knsupc, 1, 1 );
#else
												dgemm_( "N", "N", &knsupc, &nrhs, &knsupc,
														&alpha, Linv, &knsupc, &x[ii],
														&knsupc, &beta, rtemp_loc, &knsupc );
#endif			   
												#ifdef _OPENMP
													#pragma omp simd
												#endif
												for (i=0 ; i<knsupc*nrhs ; i++){
													x[ii+i] = rtemp_loc[i];
												}		
											}
											else{
#ifdef _CRAY
												STRSM(ftcs1, ftcs1, ftcs2, ftcs3, &knsupc, &nrhs, &alpha,
														lusup, &nsupr, &x[ii], &knsupc);
#elif defined (USE_VENDOR_BLAS)
												dtrsm_("L", "L", "N", "U", &knsupc, &nrhs, &alpha, 
														lusup, &nsupr, &x[ii], &knsupc, 1, 1, 1, 1);		
#else
												dtrsm_("L", "L", "N", "U", &knsupc, &nrhs, &alpha, 
														lusup, &nsupr, &x[ii], &knsupc);
#endif
											}

#if ( PROFlevel>=1 )
											TOC(t2, t1);
											stat_loc[thread_id]->utime[SOL_TRSM] += t2;
#endif	

											stat_loc[thread_id]->ops[SOLVE] += knsupc * (knsupc - 1) * nrhs;
#if ( DEBUGlevel>=2 )
											printf("(%2d) Solve X[%2d]\n", iam, k);
#endif

											/*
											 * Send Xk to process column Pc[k].
											 */						  
											if(LBtree_ptr[lk]!=NULL){ 
												BcTree_forwardMessageSimple(LBtree_ptr[lk],&x[ii - XK_H],BcTree_GetMsgSize(LBtree_ptr[lk],'d')*nrhs+XK_H,'d');
											}		  


											/*
											 * Perform local block modifications.
											 */
											lk = LBj( k, grid ); /* Local block number, column-wise. */
											lsub = Lrowind_bc_ptr[lk];
											lusup = Lnzval_bc_ptr[lk];
											if ( lsub ) {
												krow = PROW( k, grid );
												nb = lsub[0] - 1;
												knsupc = SuperSize( k );
												ii = X_BLK( LBi( k, grid ) );
												xin = &x[ii];		
												dlsum_fmod_inv_master(lsum, x, xin, rtemp, nrhs, knsupc, k,
														fmod, nb, xsup, grid, Llu,
														stat_loc,sizelsum,sizertemp,0,maxsuper,thread_id,num_thread);	
											} /* if lsub */
											// }

									}else{

										il = LSUM_BLK( lk );		  
										knsupc = SuperSize( k );

										for (ii=1;ii<num_thread;ii++)
											#ifdef _OPENMP
												#pragma omp simd
											#endif										
											for (jj=0;jj<knsupc*nrhs;jj++)
												lsum[il + jj] += lsum[il + jj + ii*sizelsum];
										RdTree_forwardMessageSimple(LRtree_ptr[lk],&lsum[il-LSUM_H],RdTree_GetMsgSize(LRtree_ptr[lk],'d')*nrhs+LSUM_H,'d'); 
									}  

								}

							}					
						} /* check Tag */		  
					}

				} /* while not finished ... */

			}
		}
#endif
//PROFILE_BAROCLINIC_FINISH();
#if ( PRNTlevel>=1 )
		t = SuperLU_timer_() - t;
		stat->utime[SOL_TOT] += t;
		if ( !iam ) {
			printf(".. L-solve time\t%f\n", t);
			fflush(stdout);
		}

		MPI_Reduce (&t, &tmax, 1, MPI_DOUBLE,
				MPI_MAX, 0, grid->comm);
		if ( !iam ) {
			printf(".. L-solve time (MAX) \t%8.4f\n", tmax);
			fflush(stdout);
		}


		t = SuperLU_timer_();

#endif
                //exit(0);

#if ( DEBUGlevel==2 )
		{
			printf("(%d*2) .. After L-solve: y =\n", iam);
			for (i = 0, k = 0; k < nsupers; ++k) {
				krow = PROW( k, grid );
				kcol = PCOL( k, grid );
				if ( myrow == krow && mycol == kcol ) { /* Diagonal process */
					knsupc = SuperSize( k );
					lk = LBi( k, grid );
					ii = X_BLK( lk );
					for (j = 0; j < knsupc; ++j)
						printf("\t(%d)\t%4d\t%.10f\n", iam, xsup[k]+j, x[ii+j]);
					fflush(stdout);
				}
				MPI_Barrier( grid->comm );
			}
		}
#endif

		SUPERLU_FREE(fmod);
		SUPERLU_FREE(frecv);
		SUPERLU_FREE(leaf_send);
		SUPERLU_FREE(leafsups);
#ifdef oneside
        int tmp_size=(1+nfrecvx)*maxrecvsz;
        for(i=0;i<tmp_size;i++){
            BC_taskq[i]=(-1.0);
        }
        tmp_size=(1+nfrecvmod)*maxrecvsz;
        for(i=0;i<tmp_size;i++){
            RD_taskq[i]=(-1.0);
        }
#elif defined(pget)
        /*
    foMPI_Win_unlock_all(bc_winl);
    foMPI_Win_unlock_all(rd_winl);
    foMPI_Win_unlock_all(bc_winl_get);
    foMPI_Win_unlock_all(rd_winl_get);
    foMPI_Win_unlock_all(tmp_bc_winl_get);
    foMPI_Win_unlock_all(tmp_rd_winl_get);
         */
    //double nbrecv1=0;
    //totalsolveBC=0;
    //totalsolveRD=0;
    //mylocal_bc_put_flag_offset=0;
    //mylocal_rd_put_flag_offset=0;
    //memset(mylocal_BCbase, 0, ( Pr * sizeof(int)));
    //memset(mylocal_RDbase, 0, ( Pc * sizeof(int)));
    //memset(BCis_solved, 0, Pr * sizeof(int));
    //memset(RDis_solved, 0, Pc * sizeof(int));
    //memset(BCcount, 0, ( Pr * sizeof(int)));
    //memset(RDcount, 0, ( Pc * sizeof(int)));


    //for (i=0;i<RDMA_FLAG_SIZE*(mysendmsg_num>mysendmsg_num_u?mysendmsg_num:mysendmsg_num_u);i++) bc_rdma_start[i]=-1;
    //for (i=0;i<RDMA_FLAG_SIZE*(mysendmsg_num_rd>mysendmsg_num_urd?mysendmsg_num_rd:mysendmsg_num_urd);i++) rd_rdma_start[i]=-1;

    //int bc_flag_size=RDMA_FLAG_SIZE * (nfrecvx>nbrecvx?nfrecvx:nbrecvx);
    //int rd_flag_size=RDMA_FLAG_SIZE * (nfrecvmod>nbrecvmod?nfrecvmod:nbrecvmod);

    //for (i=0;i<bc_flag_size;i++) bc_pget_count[i] = -1;
    //for (i=0;i<rd_flag_size;i++) rd_pget_count[i] = -1;

    //bc_flag_size=maxrecvsz* (nfrecvx>nbrecvx?nfrecvx:nbrecvx);
    //rd_flag_size=maxrecvsz * (nfrecvmod>nbrecvmod?nfrecvmod:nbrecvmod);

    //for(i=0;i<bc_flag_size;i++) tmp_buf_bc[i]=-1;
    //for(i=0;i<rd_flag_size;i++) tmp_buf_rd[i]=-1;
    //printf("(%d) End L solve\n",iam);
    //fflush(stdout);

#else
		SUPERLU_FREE(recvbuf_BC_fwd);

		for (lk=0;lk<nsupers_j;++lk){
			if(LBtree_ptr[lk]!=NULL){
				// if(BcTree_IsRoot(LBtree_ptr[lk],'d')==YES){			
				BcTree_waitSendRequest(LBtree_ptr[lk],'d');		
				// }
				// deallocate requests here
			}
		}

		for (lk=0;lk<nsupers_i;++lk){
			if(LRtree_ptr[lk]!=NULL){		
				RdTree_waitSendRequest(LRtree_ptr[lk],'d');		
				// deallocate requests here
			}
		}		
#endif
    MPI_Barrier( grid->comm );
#if ( VAMPIR>=1 )
		VT_traceoff();	
		VT_finalize(); 
#endif


		/*---------------------------------------------------
		 * Back solve Ux = y.
		 *
		 * The Y components from the forward solve is already
		 * on the diagonal processes.
	 *---------------------------------------------------*/
		 
		/* Save the count to be altered so it can be used by
		   subsequent call to PDGSTRS. */
		if ( !(bmod = intMalloc_dist(nlb*aln_i)) )
			ABORT("Calloc fails for bmod[].");
		for (i = 0; i < nlb; ++i) bmod[i*aln_i] = Llu->bmod[i];
		if ( !(brecv = intCalloc_dist(nlb)) )
			ABORT("Malloc fails for brecv[].");
		Llu->brecv = brecv;

		k = SUPERLU_MAX( Llu->nfsendx, Llu->nbsendx ) + nlb;

		/* Re-initialize lsum to zero. Each block header is already in place. */
		
#ifdef _OPENMP

#pragma omp parallel default(shared) private(ii)
	{
		for(ii=0;ii<sizelsum;ii++)
			lsum[thread_id*sizelsum+ii]=zero;
	}
    /* Set up the headers in lsum[]. */
#ifdef _OPENMP	
	#pragma omp simd lastprivate(krow,lk,il)
#endif		
    for (k = 0; k < nsupers; ++k) {
	krow = PROW( k, grid );
	if ( myrow == krow ) {
	    lk = LBi( k, grid );   /* Local block number. */
	    il = LSUM_BLK( lk );
	    lsum[il - LSUM_H] = k; /* Block number prepended in the header. */
	}
    }	

#else	
	for (k = 0; k < nsupers; ++k) {
		krow = PROW( k, grid );
		if ( myrow == krow ) {
			knsupc = SuperSize( k );
			lk = LBi( k, grid );
			il = LSUM_BLK( lk );
			dest = &lsum[il];
			
			for (jj = 0; jj < num_thread; ++jj) {						
				RHS_ITERATE(j) {
					for (i = 0; i < knsupc; ++i) dest[i + j*knsupc + jj*sizelsum] = zero;
				}	
			}	
		}
	}
#endif		

#if ( DEBUGlevel>=2 )
		for (p = 0; p < Pr*Pc; ++p) {
			if (iam == p) {
				printf("(%2d) .. Ublocks %d\n", iam, Ublocks);
				for (lb = 0; lb < nub; ++lb) {
					printf("(%2d) Local col %2d: # row blocks %2d\n",
							iam, lb, Urbs[lb]);
					if ( Urbs[lb] ) {
						for (i = 0; i < Urbs[lb]; ++i)
							printf("(%2d) .. row blk %2d:\
									lbnum %d, indpos %d, valpos %d\n",
									iam, i, 
									Ucb_indptr[lb][i].lbnum,
									Ucb_indptr[lb][i].indpos,
									Ucb_valptr[lb][i]);
					}
				}
			}
			MPI_Barrier( grid->comm );
		}
		for (p = 0; p < Pr*Pc; ++p) {
			if ( iam == p ) {
				printf("\n(%d) bsendx_plist[][]", iam);
				for (lb = 0; lb < nub; ++lb) {
					printf("\n(%d) .. local col %2d: ", iam, lb);
					for (i = 0; i < Pr; ++i)
						printf("%4d", bsendx_plist[lb][i]);
				}
				printf("\n");
			}
			MPI_Barrier( grid->comm );
		}
#endif /* DEBUGlevel */




	/* ---------------------------------------------------------
	   Initialize the async Bcast trees on all processes.
	   --------------------------------------------------------- */		
	nsupers_j = CEILING( nsupers, grid->npcol ); /* Number of local block columns */

	nbtree = 0;
	for (lk=0;lk<nsupers_j;++lk){
		if(UBtree_ptr[lk]!=NULL){
			// printf("UBtree_ptr lk %5d\n",lk); 
			if(BcTree_IsRoot(UBtree_ptr[lk],'d')==NO){			
				nbtree++;
				if(BcTree_getDestCount(UBtree_ptr[lk],'d')>0)nbrecvx_buf++;				  
			}
			BcTree_allocateRequest(UBtree_ptr[lk],'d');
		}
	}

	nsupers_i = CEILING( nsupers, grid->nprow ); /* Number of local block rows */
	if ( !(	rootsups = (int_t*)intCalloc_dist(nsupers_i)) )
		ABORT("Calloc fails for rootsups.");

	nrtree = 0;
	nroot=0;
	for (lk=0;lk<nsupers_i;++lk){
		if(URtree_ptr[lk]!=NULL){
			// printf("here lk %5d myid %5d\n",lk,iam);
			// fflush(stdout);
			nrtree++;
			RdTree_allocateRequest(URtree_ptr[lk],'d');			
			brecv[lk] = RdTree_GetDestCount(URtree_ptr[lk],'d');
			nbrecvmod += brecv[lk];
		}else{
			gb = myrow+lk*grid->nprow;  /* not sure */
			if(gb<nsupers){
				kcol = PCOL( gb, grid );
				if(mycol==kcol) { /* Diagonal process */
					if (bmod[lk*aln_i]==0){
						rootsups[nroot]=gb;				
						++nroot;
					}
				}
			}
		}
	}	

	#ifdef _OPENMP	
	#pragma omp simd
	#endif
	for (i = 0; i < nlb; ++i) bmod[i*aln_i] += brecv[i];
	// for (i = 0; i < nlb; ++i)printf("bmod[i]: %5d\n",bmod[i]);
	

#ifdef oneside
    double nbrecv1=0;
    totalsolveBC=0;
    totalsolveRD=0;
    memset(BCcount, 0, ( Pr * sizeof(int)));
    memset(RDcount, 0, ( Pc * sizeof(int)));
    memset(BCbase, 0, ( Pr * sizeof(long)));
    memset(RDbase, 0, ( Pc * sizeof(long)));
    memset(BC_taskbuf_offset, 0, Pr * sizeof(long));
    memset(RD_taskbuf_offset, 0, Pc * sizeof(long));
    memset(BCis_solved, 0, Pr * sizeof(int));
    memset(RDis_solved, 0, Pc * sizeof(int));
    
    if( Pr > 1){
        for (i=0;i<Pr;i++){
                BCbase[i] = recv_size_all_u[i]*maxrecvsz;
        }
    }
    if(Pc > 1){
        for (i=0;i<Pc;i++){
                RDbase[i] = recv_size_all_u[Pr+i]*maxrecvsz;
        }
    }        
	

    for (bcidx=0;bcidx<Pr;bcidx++){
        for(int tmp=0;tmp<bcidx;tmp++){
            BC_taskbuf_offset[bcidx] += BufSize_u[tmp]*maxrecvsz;
        }       
    }
    for (rdidx=0;rdidx<Pc;rdidx++){
             for(int tmp=0;tmp<rdidx;tmp++){ 
                     RD_taskbuf_offset[rdidx] += BufSize_urd[tmp]*maxrecvsz; 
             }        
    }
    
    //foMPI_Win_lock_all(0, bc_winl);
    //foMPI_Win_lock_all(0, rd_winl);
#elif defined (pget)
    //printf("(%d) Enter U solve setup\n",iam);
    //fflush(stdout);


    double nbrecv1=0;
    totalsolveBC=0;
    totalsolveRD=0;
    mylocal_bc_put_flag_offset=0;
    mylocal_rd_put_flag_offset=0;
    memset(mylocal_BCbase, 0, ( Pr * sizeof(int)));
    memset(mylocal_RDbase, 0, ( Pc * sizeof(int)));
    memset(BCis_solved, 0, Pr * sizeof(int));
    memset(RDis_solved, 0, Pc * sizeof(int));
    memset(BCcount, 0, ( Pr * sizeof(int)));
    memset(RDcount, 0, ( Pc * sizeof(int)));

    if( Pr > 1){
        for (i=0;i<Pr;i++){
                BCbase[i] = recv_size_all_u[i]*RDMA_FLAG_SIZE;
                //printf("In U solve, iam=%d, recv_size_all_u[%d]=%d,BCbase[%d]=%d\n",iam, i, recv_size_all_u[i],i,BCbase[i]);
                //fflush(stdout);
        }
    }
    if(Pc > 1){
        for (i=0;i<Pc;i++){
                RDbase[i] = recv_size_all_u[Pr+i]*RDMA_FLAG_SIZE;
                //printf("In U solve, iam=%d, recv_size_all_u[%d]=%d,RDbase[%d]=%d\n",iam, i, recv_size_all_u[Pr+i],i,RDbase[i]);
                //fflush(stdout);
        }
    }

    for (i=0; i<Pr;i++){
             for(j=0;j<i;j++){
                     //oneside_buf_offset[i]: When rank i put flags to my flag buffer, the relative offet is oneside_buf_offset[i].
                     // Eg., RDMA_FLAG_SIZE*oneside_buf_offset[i] is the real offset to get flag from rank i
                     mylocal_BCbase[i] += BufSize_u[j]*RDMA_FLAG_SIZE;
             }
             //printf("In U solve, iam=%d, iam_col=%d, U mylocal_BCbase[%d]=%d\n",iam, iam_col,i,mylocal_BCbase[i]);
             //fflush(stdout);
     }


    for (i=0; i<Pc;i++){
             for(j=0;j<i;j++){
                     //oneside_buf_offset[i]: When rank i put flags to my flag buffer, the relative offet is oneside_buf_offset[i].
                     // Eg., RDMA_FLAG_SIZE*oneside_buf_offset[i] is the real offset to get flag from rank i
                     mylocal_RDbase[i] += BufSize_urd[j]*RDMA_FLAG_SIZE;
             }
             //printf("In U solve, iam=%d, iam_row=%d, U mylocal_RDbase[%d]=%d\n",iam, iam_row,i,mylocal_RDbase[i]);
             //fflush(stdout);
     }




    for (i=0;i<RDMA_FLAG_SIZE*(mysendmsg_num>mysendmsg_num_u?mysendmsg_num:mysendmsg_num_u);i++) bc_rdma_start[i]=-1;
    for (i=0;i<RDMA_FLAG_SIZE*(mysendmsg_num_rd>mysendmsg_num_urd?mysendmsg_num_rd:mysendmsg_num_urd);i++) rd_rdma_start[i]=-1;

    int bc_flag_size=RDMA_FLAG_SIZE * (nfrecvx>nbrecvx?nfrecvx:nbrecvx);
    int rd_flag_size=RDMA_FLAG_SIZE * (nfrecvmod>nbrecvmod?nfrecvmod:nbrecvmod);

    for (i=0;i<bc_flag_size;i++) bc_pget_count[i] = -1;
    for (i=0;i<rd_flag_size;i++) rd_pget_count[i] = -1;

    bc_flag_size=maxrecvsz* (nfrecvx>nbrecvx?nfrecvx:nbrecvx);
    rd_flag_size=maxrecvsz * (nfrecvmod>nbrecvmod?nfrecvmod:nbrecvmod);

    for(i=0;i<bc_flag_size;i++) tmp_buf_bc[i]=-1;
    for(i=0;i<rd_flag_size;i++) tmp_buf_rd[i]=-1;

/*
   foMPI_Win_lock_all(0, bc_winl);
   foMPI_Win_lock_all(0, rd_winl);
   foMPI_Win_lock_all(0, bc_winl_get);
   foMPI_Win_lock_all(0, rd_winl_get);
   foMPI_Win_lock_all(0, tmp_bc_winl_get);
   foMPI_Win_lock_all(0, tmp_rd_winl_get);
   */
   // printf("(%2d) nbrecvx %4d,  nbrecvmod %4d,  nroot %4d\n,  nbtree %4d\n,  nrtree %4d\n",
   //		iam, nbrecvx, nbrecvmod, nroot, nbtree, nrtree);
   // printf("(%d) End U solve setup\n",iam);
   // fflush(stdout);
#else
    if ( !(recvbuf_BC_fwd = (double*)SUPERLU_MALLOC(maxrecvsz*(nbrecvx+1) * sizeof(double))) )  // this needs to be optimized for 1D row mapping
    ABORT("Malloc fails for recvbuf_BC_fwd[].");
    nbrecvx_buf=0;
#endif

#if ( DEBUGlevel>=2 )
    printf("(%2d) nbrecvx %4d,  nbrecvmod %4d,  nroot %4d\n,  nbtree %4d\n,  nrtree %4d\n",
			iam, nbrecvx, nbrecvmod, nroot, nbtree, nrtree);
	fflush(stdout);
#endif


#if ( PRNTlevel>=1 )
    t = SuperLU_timer_() - t;
	if ( !iam) printf(".. Setup U-solve time\t%8.4f\n", t);
	fflush(stdout);
	MPI_Barrier( grid->comm );
	t = SuperLU_timer_();
#endif
    /*
     * Solve the roots first by all the diagonal processes.
     */
#if ( DEBUGlevel>=2 )
    printf("(%2d) nroot %4d\n", iam, nroot);
		fflush(stdout);
#endif


//PROFILE_LND_INIT();
#ifdef _OPENMP
#pragma omp parallel default (shared)
#endif
    {
#ifdef _OPENMP
#pragma omp master
#endif
        {
#ifdef _OPENMP
#pragma	omp	taskloop firstprivate (nrhs,beta,alpha,x,rtemp,ldalsum) private (ii,jj,k,knsupc,lk,luptr,lsub,nsupr,lusup,t1,t2,Uinv,i,lib,rtemp_loc,nroot_send_tmp) nogroup
#endif
            for (jj=0;jj<nroot;jj++){
                k=rootsups[jj];

#if ( PROFlevel>=1 )
                TIC(t1);
#endif

                rtemp_loc = &rtemp[sizertemp* thread_id];



                knsupc = SuperSize( k );
                lk = LBi( k, grid ); /* Local block number, row-wise. */

                // bmod[lk] = -1;       /* Do not solve X[k] in the future. */
                ii = X_BLK( lk );
                lk = LBj( k, grid ); /* Local block number, column-wise */
                lsub = Lrowind_bc_ptr[lk];
                lusup = Lnzval_bc_ptr[lk];
                nsupr = lsub[1];


                if(Llu->inv == 1){

                    Uinv = Uinv_bc_ptr[lk];
#ifdef _CRAY
                    SGEMM( ftcs2, ftcs2, &knsupc, &nrhs, &knsupc,
						&alpha, Uinv, &knsupc, &x[ii],
						&knsupc, &beta, rtemp_loc, &knsupc );
#elif defined (USE_VENDOR_BLAS)
                    dgemm_( "N", "N", &knsupc, &nrhs, &knsupc,
						&alpha, Uinv, &knsupc, &x[ii],
						&knsupc, &beta, rtemp_loc, &knsupc, 1, 1 );
#else
                    dgemm_( "N", "N", &knsupc, &nrhs, &knsupc,
                            &alpha, Uinv, &knsupc, &x[ii],
                            &knsupc, &beta, rtemp_loc, &knsupc );
#endif
#ifdef _OPENMP
#pragma omp simd
#endif
                    for (i=0 ; i<knsupc*nrhs ; i++){
                        x[ii+i] = rtemp_loc[i];
                    }
                }else{
#ifdef _CRAY
                    STRSM(ftcs1, ftcs3, ftcs2, ftcs2, &knsupc, &nrhs, &alpha,
						lusup, &nsupr, &x[ii], &knsupc);
#elif defined (USE_VENDOR_BLAS)
                    dtrsm_("L", "U", "N", "N", &knsupc, &nrhs, &alpha,
						lusup, &nsupr, &x[ii], &knsupc, 1, 1, 1, 1);
#else
                    dtrsm_("L", "U", "N", "N", &knsupc, &nrhs, &alpha,
                           lusup, &nsupr, &x[ii], &knsupc);
#endif
                }
                // for (i=0 ; i<knsupc*nrhs ; i++){
                // printf("x_u: %f\n",x[ii+i]);
                // fflush(stdout);
                // }

                // for (i=0 ; i<knsupc*nrhs ; i++){
                // printf("x: %f\n",x[ii+i]);
                // fflush(stdout);
                // }

#if ( PROFlevel>=1 )
                TOC(t2, t1);
			stat_loc[thread_id]->utime[SOL_TRSM] += t2;
#endif
                stat_loc[thread_id]->ops[SOLVE] += knsupc * (knsupc + 1) * nrhs;

#if ( DEBUGlevel>=2 )
                printf("(%2d) Solve X[%2d]\n", iam, k);
#endif

                /*
                 * Send Xk to process column Pc[k].
                 */

                if(UBtree_ptr[lk]!=NULL){
#ifdef _OPENMP
#pragma omp atomic capture
#endif
                    nroot_send_tmp = ++nroot_send;
                    root_send[(nroot_send_tmp-1)*aln_i] = lk;

                }
            } /* for k ... */
        }
    }


#ifdef _OPENMP
#pragma omp parallel default (shared)
#endif
    {
#ifdef _OPENMP
#pragma omp master
#endif
        {
#ifdef _OPENMP
#pragma	omp	taskloop private (ii,jj,k,lk) nogroup
#endif
            for (jj=0;jj<nroot;jj++){
                k=rootsups[jj];
                lk = LBi( k, grid ); /* Local block number, row-wise. */
                ii = X_BLK( lk );
                lk = LBj( k, grid ); /* Local block number, column-wise */

                /*
                 * Perform local block modifications: lsum[i] -= U_i,k * X[k]
                 */
                if ( Urbs[lk] )
                    dlsum_bmod_inv(lsum, x, &x[ii], rtemp, nrhs, k, bmod, Urbs,Urbs2,
                                   Ucb_indptr, Ucb_valptr, xsup, grid, Llu,
                                   send_req, stat_loc, root_send, &nroot_send, sizelsum,sizertemp,thread_id,num_thread);

            } /* for k ... */

        }
    }

    for (i=0;i<nroot_send;i++){
        lk = root_send[(i)*aln_i];
        if(lk>=0){ // this is a bcast forwarding
            gb = mycol+lk*grid->npcol;  /* not sure */
            lib = LBi( gb, grid ); /* Local block number, row-wise. */
            ii = X_BLK( lib );
#ifdef oneside
            BcTree_forwardMessageOneSide(UBtree_ptr[lk],&x[ii - XK_H],BcTree_GetMsgSize(UBtree_ptr[lk],'d')*nrhs+XK_H,'d',&iam_col,BCcount, BCbase, &maxrecvsz,Pc);
#elif defined (pget)
            //printf("(%2d) send U leaf nodes, offset=%d,k=%lf\n",iam,ii-XK_H,x[ii-XK_H]);
            //fflush(stdout);
#if ( PROFlevel>=1 )
						TIC(t1);
#endif
            bc_rdma_start[mylocal_bc_put_flag_offset]=ii-XK_H;
            bc_rdma_start[mylocal_bc_put_flag_offset+1]=BcTree_GetMsgSize(UBtree_ptr[lk],'d')*nrhs+XK_H;
            bc_rdma_start[mylocal_bc_put_flag_offset+2]=1;
            BcTree_forwardMessageOneSide(UBtree_ptr[lk],'d', &bc_rdma_start[mylocal_bc_put_flag_offset], Pc, BCcount, BCbase);
            mylocal_bc_put_flag_offset += RDMA_FLAG_SIZE;
#if ( PROFlevel>=1 )
						TOC(t2, t1);
						stat_loc[thread_id]->utime[SOL_COMM] += t2;
#endif
#else
            //printf("(%2d) send U leaf nodes, k=%d\n",iam,x[ii-XK_H]);
            //fflush(stdout);
            BcTree_forwardMessageSimple(UBtree_ptr[lk],&x[ii - XK_H],BcTree_GetMsgSize(UBtree_ptr[lk],'d')*nrhs+XK_H,'d');
#endif
        }else{ // this is a reduce forwarding
            lk = -lk - 1;
            il = LSUM_BLK( lk );
#ifdef oneside
            RdTree_forwardMessageOneSide(URtree_ptr[lk],&lsum[il - LSUM_H ],RdTree_GetMsgSize(URtree_ptr[lk],'d')*nrhs+LSUM_H,'d',&iam_row, RDcount, RDbase, &maxrecvsz, Pc);
#elif defined (pget)
#if ( PROFlevel>=1 )
						TIC(t1);
#endif
            //printf("(%d), U RD send leaf nodes\n",iam);
            //fflush(stdout);
            rd_rdma_start[mylocal_rd_put_flag_offset]=il-LSUM_H;
            rd_rdma_start[mylocal_rd_put_flag_offset+1]=RdTree_GetMsgSize(URtree_ptr[lk],'d')*nrhs+LSUM_H;
            rd_rdma_start[mylocal_rd_put_flag_offset+2]=1;
            RdTree_forwardMessageOneSide(URtree_ptr[lk],'d', &rd_rdma_start[mylocal_rd_put_flag_offset], Pc, RDcount, RDbase);
            mylocal_rd_put_flag_offset += RDMA_FLAG_SIZE;
#if ( PROFlevel>=1 )
						TOC(t2, t1);
						stat_loc[thread_id]->utime[SOL_COMM] += t2;
#endif
#else
            RdTree_forwardMessageSimple(URtree_ptr[lk],&lsum[il - LSUM_H ],RdTree_GetMsgSize(URtree_ptr[lk],'d')*nrhs+LSUM_H,'d');
#endif
        }
    }

    /*
     * Compute the internal nodes asychronously by all processes.
     */
#ifdef oneside
recvRankNum=-1;
shift=0;
while(nbrecv1< nbrecvx+nbrecvmod){
    thread_id=0;
    if (totalsolveBC < nbrecvx){
	    shift=0;
       //foMPI_Win_flush_all(bc_winl);
        //for (int debug=0;debug<Pr; debug++){
            //printf("iam=%d,count=%d,validBCQindex[%d]=%d\n",iam,debug_count,debug,validBCQindex_u[debug]);
            //fflush(stdout);
        //}
        for (bcidx=0;bcidx<Pr && validBCQindex_u[bcidx]!=-1;bcidx++){

            recvRankNum=validBCQindex_u[bcidx];  //bcidx; //validBCQindex[bcidx];
            i=BC_taskbuf_offset[recvRankNum]+BCis_solved[recvRankNum]*maxrecvsz; //BCis_solved[bcidx];
            recvbuf0 = &BC_taskq[i];
            k = *recvbuf0;

            //printf("bcbc--111--iam=%d, bcidx=%d,k=%d\n",iam,bcidx,k);
            //fflush(stdout);
#if ( PROFlevel>=1 )
						TIC(t1);
#endif

            if (k < 0 ) {
               if(shift>0){
                    validBCQindex_u[bcidx-shift]=validBCQindex_u[bcidx];
                    validBCQindex_u[bcidx]=-1;
                    //printf("iam=%d,Now shift %d to %d\n",iam,bcidx,bcidx-shift);
                    //fflush(stdout);
               }
               continue;
            }

            lk = LBj( k, grid );    /* local block number */

            //if (totalsolveBC % 10 == 0){
            checkend=BcTree_GetMsgSize(UBtree_ptr[lk],'d')*nrhs;
            crc_16_val=crc_16((unsigned char*)&recvbuf0[XK_H],sizeof(double)*checkend);
            ////printf("bcbc--222--iam=%d, checksum=%lf,should be %lf\n",iam, recvbuf0[checkend-1], recvbuf0[XK_H-1]);
            ////fflush(stdout);
            //myhash=calcul_hash(&recvbuf0[XK_H],sizeof(double)*checkend);
            //
            //if((myhash-recvbuf0[XK_H-1])!=0.0) {
            ////if(myhash!=(unsigned int)recvbuf0[XK_H-1]) {
            if(crc_16_val!=(uint16_t)recvbuf0[XK_H-1]) {
               if(shift>0){
                    validBCQindex_u[bcidx-shift]=validBCQindex_u[bcidx];
                    validBCQindex_u[bcidx]=-1;
                    //printf("1-iam=%d,Now shift %d to %d\n",iam,bcidx,bcidx-shift);
                    //fflush(stdout);
               }
               continue;
            }
#if ( PROFlevel>=1 )
						TOC(t2, t1);
						stat_loc[thread_id]->utime[SOL_COMM] += t2;
#endif
                //t= SuperLU_timer_();
            //}
            totalsolveBC += 1; //BC_subtotal[bcidx] - BCis_solved[bcidx];
		    BCis_solved[recvRankNum]++;

            if(BcTree_getDestCount(UBtree_ptr[lk],'d')>0){
                //printf("iam=%d,before BcTree_forwardMessageOneSide\n",iam);
                //fflush(stdout);
                //BcTree_forwardMessageOneSide(UBtree_ptr[lk],recvbuf0,checkend,'d',&iam_col, BCcount, BCbase, &maxrecvsz, Pc,sendbufval);
                BcTree_forwardMessageOneSide(UBtree_ptr[lk],recvbuf0,BcTree_GetMsgSize(UBtree_ptr[lk],'d')*nrhs+XK_H,'d',&iam_col, BCcount, BCbase, &maxrecvsz, Pc);
                //printf("iam=%d,end BcTree_forwardMessageOneSide\n",iam);
                //fflush(stdout);
            }
            //printf("iam=%d,before dlsum_bmod_inv_master debug_count=%d\n",iam,debug_count);
            //fflush(stdout);

			dlsum_bmod_inv_master(lsum, x, &recvbuf0[XK_H], rtemp, nrhs, k, bmod, Urbs,Urbs2,
			            	Ucb_indptr, Ucb_valptr, xsup, grid, Llu,
				            send_req, stat_loc, sizelsum,sizertemp,thread_id,num_thread,
                            &iam_row, RDcount, RDbase, &iam_col, BCcount, BCbase, Pc, maxrecvsz);
            //printf("iam=%d,End dlsum_bmod_inv_master debug_count=%d,shift=%d\n",iam,debug_count,shift);
            //fflush(stdout);

            if (BCis_solved[recvRankNum] == BufSize_u[recvRankNum]) {
                validBCQindex_u[bcidx]=-1;
                shift += 1;
                //printf("iam=%d,shift=%d\n",iam,shift);
                //fflush(stdout);
            }else{
                if(shift>0){
                     validBCQindex_u[bcidx-shift]=validBCQindex_u[bcidx];
                     validBCQindex_u[bcidx]=-1;
                     //printf("End-iam=%d,Now shift %d to %d\n",iam,bcidx,bcidx-shift);
                     //fflush(stdout);
                }
            }
            //printf("iam=%d,at end of iter, BCis_solved[%d]=%d, BufSize_u[%d]=%d\n",iam, recvRankNum,BCis_solved[recvRankNum],recvRankNum,BufSize_u[recvRankNum]);
            //fflush(stdout);
        } // for (bcidx=0;bcidx<Pr && validBCQindex_u[bcidx]!=-1;bcidx++)
    } // if (totalsolveBC < nbrecvx)

    if (totalsolveRD < nbrecvmod){
        shift=0;
       //foMPI_Win_flush_all(rd_winl);
       for (rdidx=0;rdidx<Pc && validRDQindex_u[rdidx]!=-1;rdidx++){

                recvRankNum=validRDQindex_u[rdidx];  //bcidx; //validBCQindex[bcidx];
                ird=RD_taskbuf_offset[recvRankNum]+RDis_solved[recvRankNum]*maxrecvsz;
                recvbuf0 = &RD_taskq[ird];
                k = *recvbuf0;
                //printf("rdrd--111--iam=%d, rdidx=%d,k=%d\n",iam,rdidx,k);
                //fflush(stdout);
#if ( PROFlevel>=1 )
						TIC(t1);
#endif
	            if (k < 0) {
                   if(shift>0){
                        validRDQindex_u[rdidx-shift]=validRDQindex_u[rdidx];
                        validRDQindex_u[rdidx]=-1;
                   }
                   continue;
                }
                lk = LBi( k, grid );
                //if (totalsolveRD %10 == 0){
                checkend=RdTree_GetMsgSize(URtree_ptr[lk],'d')*nrhs;
                crc_16_val=crc_16((unsigned char*)&recvbuf0[LSUM_H],sizeof(double)*checkend);
	            ////crc_32_val = 0xffffffffL;
                ////for (int tmp=0; tmp<checkend; ++tmp){
                ////    crc_16_val=update_crc_16(crc_16_val, recvbuf0[tmp]);
                ////    //if(!isnan(recvbuf0[tmp])) checksum += recvbuf0[tmp];
                ////}
                ////printf("bcbc--222--iam=%d, checksum=%f,should be %f\n",iam,checksum,recvbuf0[checkend]);
                ////fflush(stdout);
                ////if(abs(checksum-recvbuf0[checkend])<0.00000001) {
                ////if((uint16_t)crc_16_val!=(uint16_t)recvbuf0[checkend]) {
                ////if((uint32_t)crc_32_val!=(uint32_t)recvbuf0[checkend]) {
                ////if((int)checksum!=(int)recvbuf0[checkend]) {
                ////if(abs(checksum-recvbuf0[checkend])!=0) {
                //myhash=calcul_hash(&recvbuf0[LSUM_H],sizeof(double)*checkend);

                ////if(myhash!=(unsigned int)recvbuf0[LSUM_H-1]) {
                //if((myhash-recvbuf0[LSUM_H-1])!=0.0) {
                if(crc_16_val!=(uint16_t)recvbuf0[LSUM_H-1]) {
                   if(shift>0){
                        validRDQindex_u[rdidx-shift]=validRDQindex_u[rdidx];
                        validRDQindex_u[rdidx]=-1;
                   }
                   continue;
                }
#if ( PROFlevel>=1 )
						TOC(t2, t1);
						stat_loc[thread_id]->utime[SOL_COMM] += t2;
#endif
                //}
	            //t = SuperLU_timer_();
                totalsolveRD += 1; //RD_subtotal[rdidx]-RDis_solved[rdidx];

                RDis_solved[recvRankNum] += 1 ;

				knsupc = SuperSize( k );
			 	tempv = &recvbuf0[LSUM_H];
				il = LSUM_BLK( lk );
				RHS_ITERATE(j) {
#ifdef _OPENMP
	#pragma omp simd
#endif
			        for (i = 0; i < knsupc; ++i)
						lsum[i + il + j*knsupc + thread_id*sizelsum] += tempv[i + j*knsupc];
				 }

                bmod_tmp=--bmod[lk*aln_i];
				thread_id = 0;
				rtemp_loc = &rtemp[sizertemp* thread_id];
				if ( bmod_tmp==0 ) {
				    if(RdTree_IsRoot(URtree_ptr[lk],'d')==YES){
						knsupc = SuperSize( k );
						for (ii=1;ii<num_thread;ii++)
#ifdef _OPENMP
	#pragma omp simd
#endif
							for (jj=0;jj<knsupc*nrhs;jj++)
								lsum[il+ jj ] += lsum[il + jj + ii*sizelsum];

						ii = X_BLK( lk );
						RHS_ITERATE(j)
#ifdef _OPENMP
	#pragma omp simd
#endif
							for (i = 0; i < knsupc; ++i)
								x[i + ii + j*knsupc] += lsum[i + il + j*knsupc ];

						lk = LBj( k, grid ); /* Local block number, column-wise. */
						lsub = Lrowind_bc_ptr[lk];
						lusup = Lnzval_bc_ptr[lk];
						nsupr = lsub[1];

						if(Llu->inv == 1){

							Uinv = Uinv_bc_ptr[lk];

#ifdef _CRAY
							SGEMM( ftcs2, ftcs2, &knsupc, &nrhs, &knsupc,
									&alpha, Uinv, &knsupc, &x[ii],
									&knsupc, &beta, rtemp_loc, &knsupc );
#elif defined (USE_VENDOR_BLAS)
							dgemm_( "N", "N", &knsupc, &nrhs, &knsupc,
									&alpha, Uinv, &knsupc, &x[ii],
									&knsupc, &beta, rtemp_loc, &knsupc, 1, 1 );
#else
							dgemm_( "N", "N", &knsupc, &nrhs, &knsupc,
									&alpha, Uinv, &knsupc, &x[ii],
									&knsupc, &beta, rtemp_loc, &knsupc );
#endif

#ifdef _OPENMP
	#pragma omp simd
#endif
							for (i=0 ; i<knsupc*nrhs ; i++){
								x[ii+i] = rtemp_loc[i];
							}
						}else{
#ifdef _CRAY
							STRSM(ftcs1, ftcs3, ftcs2, ftcs2, &knsupc, &nrhs, &alpha,
									lusup, &nsupr, &x[ii], &knsupc);
#elif defined (USE_VENDOR_BLAS)
							dtrsm_("L", "U", "N", "N", &knsupc, &nrhs, &alpha,
									lusup, &nsupr, &x[ii], &knsupc, 1, 1, 1, 1);
#else
							dtrsm_("L", "U", "N", "N", &knsupc, &nrhs, &alpha,
									lusup, &nsupr, &x[ii], &knsupc);
#endif
						}

#if ( PROFlevel>=1 )
							TOC(t2, t1);
							stat_loc[thread_id]->utime[SOL_TRSM] += t2;
#endif
							stat_loc[thread_id]->ops[SOLVE] += knsupc * (knsupc + 1) * nrhs;

#if ( DEBUGlevel>=2 )
						printf("(%2d) Solve X[%2d]\n", iam, k);
#endif

						if(UBtree_ptr[lk]!=NULL){
							BcTree_forwardMessageOneSide(UBtree_ptr[lk],&x[ii - XK_H],BcTree_GetMsgSize(UBtree_ptr[lk],'d')*nrhs+XK_H,'d',&iam_col,BCcount, BCbase, &maxrecvsz,Pc);
						}

						if ( Urbs[lk] )
							dlsum_bmod_inv_master(lsum, x, &x[ii], rtemp, nrhs, k, bmod, Urbs,Urbs2,
									            Ucb_indptr, Ucb_valptr, xsup, grid, Llu,
									            send_req, stat_loc, sizelsum,sizertemp,thread_id,num_thread,
                                                &iam_row, RDcount, RDbase, &iam_col, BCcount, BCbase, Pc, maxrecvsz);

					}else{ // if(RdTree_IsRoot(URtree_ptr[lk],'d')==YES)
						il = LSUM_BLK( lk );
						knsupc = SuperSize( k );

						for (ii=1;ii<num_thread;ii++)
#ifdef _OPENMP
	#pragma omp simd
#endif
							for (jj=0;jj<knsupc*nrhs;jj++)
								lsum[il+ jj ] += lsum[il + jj + ii*sizelsum];

						RdTree_forwardMessageOneSide(URtree_ptr[lk],&lsum[il-LSUM_H],RdTree_GetMsgSize(URtree_ptr[lk],'d')*nrhs+LSUM_H,'d',&iam_row, RDcount,RDbase, &maxrecvsz, Pc);
					}//if(RdTree_IsRoot(URtree_ptr[lk],'d')==YES)
                }//if ( bmod_tmp==0 )
            if (RDis_solved[recvRankNum] == BufSize_urd[recvRankNum]) {
                validRDQindex_u[rdidx]=-1;
                shift += 1;
                //printf("iam=%d,shift=%d\n",iam,shift);
                //fflush(stdout);
            }else{
                if(shift>0){
                     validRDQindex_u[rdidx-shift]=validRDQindex_u[rdidx];
                     validRDQindex_u[rdidx]=-1;
                     //printf("iam=%d,Now shift %d to %d\n",iam,bcidx,bcidx-shift);
                     //fflush(stdout);
                }
            }
        }//for (rdidx=0;rdidx<Pc;rdidx++)
        }
        nbrecv1 = totalsolveBC + totalsolveRD;
}

#elif defined (pget)
    recvRankNum=-1;
    shift=0;
    get_msgsize=0;
    mynext_offset=0;
    mycur_offset=0;
    mynext_offset_rd=0;
    mycur_offset_rd=0;
    tmp_counter=0;
    //printf("iam=%d, will enter U while\n",iam);
    //fflush(stdout);
    while(nbrecv1< nbrecvx+nbrecvmod){
        thread_id=0;
        if (totalsolveBC < nbrecvx){
            shift=0;
            for (bcidx=0;bcidx<Pr && validBCQindex_u[bcidx]!=-1;bcidx++){
                recvRankNum=validBCQindex_u[bcidx];  //bcidx; //validBCQindex[bcidx];
                int tmp_idx=mylocal_BCbase[recvRankNum]+BCis_solved[recvRankNum]*RDMA_FLAG_SIZE;
                if ((bc_pget_count[tmp_idx] == -1) || (bc_pget_count[tmp_idx+1] == -1) || (bc_pget_count[tmp_idx+2] == -1))  {
                    if(shift>0){
                        validBCQindex_u[bcidx-shift]=validBCQindex_u[bcidx];
                        validBCQindex_u[bcidx]=-1;
                    //printf("iam=%d,Now shift %d to %d\n",iam,bcidx,bcidx-shift);
                    //fflush(stdout);
                    }
                    continue;
                } else {
                    get_offset=bc_pget_count[tmp_idx];
                    get_msgsize=bc_pget_count[tmp_idx+1];
                    get_fromwhere=bc_pget_count[tmp_idx+2];

                    //printf("In U BC solve (%d),iam %d get from %d, get_offset=%d, get_msgsize=%d, get_fromwhere=%d\n",tmp_counter,iam,recvRankNum,get_offset,get_msgsize, get_fromwhere);
                    //fflush(stdout);

                    mycur_offset=mynext_offset;

                    if (get_fromwhere==1){
                        //printf("In U BC solve (%d), iam %d in 1,mycur_offset=%d,get_msgsize=%d\n",tmp_counter, iam,mycur_offset,get_msgsize);
                        //fflush(stdout);
                        //foMPI_Win_lock(foMPI_LOCK_EXCLUSIVE,recvRankNum,0,bc_winl_get);//
                        //int foMPI_Get(void *origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, foMPI_Win win);
                        //foMPI_Win_lock(foMPI_LOCK_SHARED,recvRankNum,0,bc_winl_get);
#if ( PROFlevel>=1 )
						TIC(t1);
#endif
                        foMPI_Get(&tmp_buf_bc[mycur_offset], get_msgsize, MPI_DOUBLE, recvRankNum, get_offset, get_msgsize, MPI_DOUBLE,bc_winl_get);
                        foMPI_Win_flush_local(recvRankNum,bc_winl_get);
#if ( PROFlevel>=1 )
						TOC(t2, t1);
						stat_loc[thread_id]->utime[SOL_COMM] += t2;
#endif

                        //foMPI_Win_unlock(recvRankNum,bc_winl_get);
                        //printf("In U BC solve (%d), iam %d,finish get from x\n",tmp_counter,iam);
                        //fflush(stdout);

                    }else{
                        //printf("In U BC solve (%d), iam %d in 2, get from %d, mycur_offset=%d,get_msgsize=%d\n", tmp_counter, iam,recvRankNum, mycur_offset,get_msgsize);
                        //fflush(stdout);
                        //foMPI_Win_lock(foMPI_LOCK_SHARED,recvRankNum,0,tmp_bc_winl_get);
                        //foMPI_Win_lock(foMPI_LOCK_EXCLUSIVE,recvRankNum,0, tmp_bc_winl_get);//
#if ( PROFlevel>=1 )
						TIC(t1);
#endif
                        //int foMPI_Get(void *origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, foMPI_Win win);
                        foMPI_Get(&tmp_buf_bc[mycur_offset], get_msgsize, MPI_DOUBLE, recvRankNum, get_offset, get_msgsize, MPI_DOUBLE,tmp_bc_winl_get);
                        foMPI_Win_flush_local(recvRankNum,tmp_bc_winl_get);
#if ( PROFlevel>=1 )
						TOC(t2, t1);
						stat_loc[thread_id]->utime[SOL_COMM] += t2;
#endif

                        //foMPI_Win_unlock(recvRankNum,tmp_bc_winl_get);
                        //printf("In U BC solve (%d), iam %d,finish get from tmp_buf_bc\n",tmp_counter,iam);
                        //fflush(stdout);
                    }


                    mynext_offset+=get_msgsize;

                    k = tmp_buf_bc[mycur_offset];
                    lk = LBj( k, grid );    /* local block number */

                    //printf("In U BC solve (%d), iam %d, mycur_offset=%d, k=%d, lk=%d\n",tmp_counter, iam,mycur_offset, k, lk);
                    //fflush(stdout);

                    totalsolveBC += 1; //BC_subtotal[bcidx] - BCis_solved[bcidx];
		            BCis_solved[recvRankNum]++;

		            //printf("In U BC solve (%d), iam %d before getDestCount\n",tmp_counter,iam );
                    //fflush(stdout);
                    if(BcTree_getDestCount(UBtree_ptr[lk],'d')>0){
		                //printf("In U BC solve (%d), iam %d in getDestCount\n",tmp_counter,iam );
                        //fflush(stdout);
#if ( PROFlevel>=1 )
						TIC(t1);
#endif
                        bc_rdma_start[mylocal_bc_put_flag_offset]=mycur_offset;
                        bc_rdma_start[mylocal_bc_put_flag_offset+1]=get_msgsize;
                        bc_rdma_start[mylocal_bc_put_flag_offset+2]=2;
                        //printf("In U BC solve(%d), iam %d, will notify get x ,mylocal_bc_put_flag_offset=%d,offset=%d,msgsize=%d\n",tmp_counter, iam,mylocal_bc_put_flag_offset, bc_rdma_start[mylocal_bc_put_flag_offset],bc_rdma_start[mylocal_bc_put_flag_offset+1]);
                        //fflush(stdout);
                        BcTree_forwardMessageOneSide(UBtree_ptr[lk],'d', &bc_rdma_start[mylocal_bc_put_flag_offset], Pc,BCcount, BCbase);
                        mylocal_bc_put_flag_offset += RDMA_FLAG_SIZE;
#if ( PROFlevel>=1 )
						TOC(t2, t1);
						stat_loc[thread_id]->utime[SOL_COMM] += t2;
#endif
                    }
                    //printf("In U BC solve (%d), iam %d, before dlsum_bmod_inv_master \n",tmp_counter,iam);
                    //fflush(stdout);

			        dlsum_bmod_inv_master(lsum, x, &tmp_buf_bc[mycur_offset+XK_H], rtemp, nrhs, k, bmod, Urbs,Urbs2,
			            	Ucb_indptr, Ucb_valptr, xsup, grid, Llu,
				            send_req, stat_loc, sizelsum,sizertemp,thread_id,num_thread,
                            &iam_row, rd_rdma_start, &iam_col, bc_rdma_start, Pc, maxrecvsz, BCcount, BCbase, RDcount, RDbase,&mylocal_bc_put_flag_offset, &mylocal_rd_put_flag_offset);
                    //printf("In U BC solve (%d), iam %d, after dlsum_bmod_inv_master \n",tmp_counter,iam);
                    //fflush(stdout);

                    if (BCis_solved[recvRankNum] == BufSize_u[recvRankNum]) {
                        validBCQindex_u[bcidx]=-1;
                        shift += 1;
                        //printf("iam=%d,shift=%d\n",iam,shift);
                        //fflush(stdout);
                    }else{
                        if(shift>0){
                            validBCQindex_u[bcidx-shift]=validBCQindex_u[bcidx];
                            validBCQindex_u[bcidx]=-1;
                            //printf("End-iam=%d,Now shift %d to %d\n",iam,bcidx,bcidx-shift);
                            //fflush(stdout);
                        }
                    }
                    //printf("In U BC solve (%d), iam %d,BCis_solved[%d]=%d,BufSize_u[%d]=%d\n",tmp_counter,iam,recvRankNum,BCis_solved[recvRankNum],recvRankNum,BufSize_u[recvRankNum]);
                    //fflush(stdout);
                    tmp_counter+=1;
                    //tmp_counter +=1;
            } // if bc_pget_count>0
        } // for (bcidx=0;bcidx<Pr && validBCQindex_u[bcidx]!=-1;bcidx++)
    } // if (totalsolveBC < nbrecvx)

    if (totalsolveRD < nbrecvmod){
       shift=0;
       for (rdidx=0;rdidx<Pc && validRDQindex_u[rdidx]!=-1;rdidx++){
                recvRankNum=validRDQindex_u[rdidx];  //bcidx; //validBCQindex[bcidx];
                int tmp_idx=mylocal_RDbase[recvRankNum]+RDis_solved[recvRankNum]*RDMA_FLAG_SIZE;
	            if ((rd_pget_count[tmp_idx] == -1) || (rd_pget_count[tmp_idx+1] == -1) || (rd_pget_count[tmp_idx+2] == -1)) {
                   if(shift>0){
                        validRDQindex_u[rdidx-shift]=validRDQindex_u[rdidx];
                        validRDQindex_u[rdidx]=-1;
                   }
                   continue;
                } else {
                    //printf("In U RD solve (%d)--1\n",iam);
                    //fflush(stdout);
                    get_offset=rd_pget_count[tmp_idx];
                    get_msgsize=rd_pget_count[tmp_idx+1];
                    get_fromwhere=rd_pget_count[tmp_idx+2];

                    mycur_offset_rd=mynext_offset_rd;
                    if (get_fromwhere==1){
                        //int foMPI_Get(void *origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, foMPI_Win win);
                        //printf("In U RD solve (%d)--2,get from x\n",iam);
                        //fflush(stdout);
                        //foMPI_Win_lock(foMPI_LOCK_EXCLUSIVE,recvRankNum,0,rd_winl_get);
#if ( PROFlevel>=1 )
						TIC(t1);
#endif
                        foMPI_Get(&tmp_buf_rd[mycur_offset_rd], get_msgsize, MPI_DOUBLE, recvRankNum, get_offset, get_msgsize, MPI_DOUBLE,rd_winl_get);
                        foMPI_Win_flush_local(recvRankNum,rd_winl_get);
#if ( PROFlevel>=1 )
						TOC(t2, t1);
						stat_loc[thread_id]->utime[SOL_COMM] += t2;
#endif

                        //foMPI_Win_unlock(recvRankNum,rd_winl_get);
                        //printf("In U RD solve (%d)--2,END get from x\n",iam);
                        //fflush(stdout);
                    }else{
                        //int foMPI_Get(void *origin_addr, int origin_count, MPI_Datatype origin_datatype, int target_rank, MPI_Aint target_disp, int target_count, MPI_Datatype target_datatype, foMPI_Win win);
                        //printf("In U RD solve (%d)--2,get from tmp_buf_rd\n",iam);
                        //fflush(stdout);
                        //foMPI_Win_lock(foMPI_LOCK_EXCLUSIVE,recvRankNum,0,tmp_rd_winl_get);
#if ( PROFlevel>=1 )
						TIC(t1);
#endif
                        foMPI_Get(&tmp_buf_rd[mycur_offset_rd], get_msgsize, MPI_DOUBLE, recvRankNum, get_offset, get_msgsize, MPI_DOUBLE,tmp_rd_winl_get);
                        foMPI_Win_flush_local(recvRankNum,tmp_rd_winl_get);
#if ( PROFlevel>=1 )
						TOC(t2, t1);
						stat_loc[thread_id]->utime[SOL_COMM] += t2;
#endif

                        //foMPI_Win_unlock(recvRankNum,tmp_rd_winl_get);
                        //printf("In U RD solve (%d)--2,END get from tmp_buf_rd\n",iam);
                        //fflush(stdout);
                    }

                    mynext_offset_rd+=get_msgsize;


                    k = tmp_buf_rd[mycur_offset_rd];
                    lk = LBi( k, grid );

                    totalsolveRD += 1; //RD_subtotal[rdidx]-RDis_solved[rdidx];
                    RDis_solved[recvRankNum] += 1 ;

				    knsupc = SuperSize( k );
			 	    tempv = &tmp_buf_rd[mycur_offset_rd+LSUM_H];
				    il = LSUM_BLK( lk );
                    //printf("In U RD solve (%d)--3,k=%d,lk=%d,il=%d\n",iam,k, lk, il);
                    //fflush(stdout);
				    RHS_ITERATE(j) {
#ifdef _OPENMP
	#pragma omp simd
#endif
			            for (i = 0; i < knsupc; ++i)
						    lsum[i + il + j*knsupc + thread_id*sizelsum] += tempv[i + j*knsupc];
				    }

                    bmod_tmp=--bmod[lk*aln_i];
				    thread_id = 0;
				    rtemp_loc = &rtemp[sizertemp* thread_id];
				    if ( bmod_tmp==0 ) {
				        if(RdTree_IsRoot(URtree_ptr[lk],'d')==YES){
						    knsupc = SuperSize( k );
						    for (ii=1;ii<num_thread;ii++)
#ifdef _OPENMP
	#pragma omp simd
#endif
							    for (jj=0;jj<knsupc*nrhs;jj++)
								    lsum[il+ jj ] += lsum[il + jj + ii*sizelsum];

						    ii = X_BLK( lk );
						    RHS_ITERATE(j)
#ifdef _OPENMP
	#pragma omp simd
#endif
							for (i = 0; i < knsupc; ++i)
								x[i + ii + j*knsupc] += lsum[i + il + j*knsupc ];

						    lk = LBj( k, grid ); /* Local block number, column-wise. */
						    lsub = Lrowind_bc_ptr[lk];
						    lusup = Lnzval_bc_ptr[lk];
						    nsupr = lsub[1];

						    if(Llu->inv == 1){

							    Uinv = Uinv_bc_ptr[lk];

#ifdef _CRAY
							    SGEMM( ftcs2, ftcs2, &knsupc, &nrhs, &knsupc,
							    		&alpha, Uinv, &knsupc, &x[ii],
							    		&knsupc, &beta, rtemp_loc, &knsupc );
#elif defined (USE_VENDOR_BLAS)
							    dgemm_( "N", "N", &knsupc, &nrhs, &knsupc,
									    &alpha, Uinv, &knsupc, &x[ii],
									    &knsupc, &beta, rtemp_loc, &knsupc, 1, 1 );
#else
							    dgemm_( "N", "N", &knsupc, &nrhs, &knsupc,
									    &alpha, Uinv, &knsupc, &x[ii],
									    &knsupc, &beta, rtemp_loc, &knsupc );
#endif

#ifdef _OPENMP
	#pragma omp simd
#endif
							    for (i=0 ; i<knsupc*nrhs ; i++){
								    x[ii+i] = rtemp_loc[i];
							    }
						    }else{
#ifdef _CRAY
							    STRSM(ftcs1, ftcs3, ftcs2, ftcs2, &knsupc, &nrhs, &alpha,
									lusup, &nsupr, &x[ii], &knsupc);
#elif defined (USE_VENDOR_BLAS)
							    dtrsm_("L", "U", "N", "N", &knsupc, &nrhs, &alpha,
									lusup, &nsupr, &x[ii], &knsupc, 1, 1, 1, 1);
#else
							    dtrsm_("L", "U", "N", "N", &knsupc, &nrhs, &alpha,
									lusup, &nsupr, &x[ii], &knsupc);
#endif
						    }

#if ( PROFlevel>=1 )
							TOC(t2, t1);
							stat_loc[thread_id]->utime[SOL_TRSM] += t2;
#endif
							stat_loc[thread_id]->ops[SOLVE] += knsupc * (knsupc + 1) * nrhs;

#if ( DEBUGlevel>=2 )
						    printf("(%2d) Solve X[%2d]\n", iam, k);
#endif

						    if(UBtree_ptr[lk]!=NULL){
                                //printf("In U RD solve (%d)--3,BC send\n",iam);
                                //fflush(stdout);
#if ( PROFlevel>=1 )
						TIC(t1);
#endif
						    	bc_rdma_start[mylocal_bc_put_flag_offset]=ii-XK_H;
						    	bc_rdma_start[mylocal_bc_put_flag_offset+1]=BcTree_GetMsgSize(UBtree_ptr[lk],'d')*nrhs+XK_H;
						    	bc_rdma_start[mylocal_bc_put_flag_offset+2]=1;
						        BcTree_forwardMessageOneSide(UBtree_ptr[lk],'d', &bc_rdma_start[mylocal_bc_put_flag_offset],Pc, BCcount, BCbase);
						        mylocal_bc_put_flag_offset += RDMA_FLAG_SIZE;
                                //printf("In U RD solve (%d)--3,END BC send\n",iam);
                                //fflush(stdout);
#if ( PROFlevel>=1 )
						TOC(t2, t1);
						stat_loc[thread_id]->utime[SOL_COMM] += t2;
#endif
						    }

						    if ( Urbs[lk] ){
                                //printf("In U RD solve (%d)--before dlsum_bmod_inv_master \n",iam);
                                //fflush(stdout);
						    	dlsum_bmod_inv_master(lsum, x, &x[ii], rtemp, nrhs, k, bmod, Urbs,Urbs2,
						    			            Ucb_indptr, Ucb_valptr, xsup, grid, Llu,
						    			            send_req, stat_loc, sizelsum,sizertemp,thread_id,num_thread,
                                                    &iam_row, rd_rdma_start, &iam_col, bc_rdma_start,Pc, maxrecvsz, BCcount, BCbase, RDcount, RDbase, &mylocal_bc_put_flag_offset, &mylocal_rd_put_flag_offset);
                                //printf("In U RD solve (%d)--After dlsum_bmod_inv_master \n",iam);
                                //fflush(stdout);
                            }
					    }else{ // if(RdTree_IsRoot(URtree_ptr[lk],'d')==YES)
						    il = LSUM_BLK( lk );
						    knsupc = SuperSize( k );

						    for (ii=1;ii<num_thread;ii++)
#ifdef _OPENMP
	#pragma omp simd
#endif
							    for (jj=0;jj<knsupc*nrhs;jj++)
								    lsum[il+ jj ] += lsum[il + jj + ii*sizelsum];

                            //printf("In U RD solve (%d)--before RD send \n",iam);
                            //fflush(stdout);
#if ( PROFlevel>=1 )
						TIC(t1);
#endif
						    rd_rdma_start[mylocal_rd_put_flag_offset]=il-LSUM_H;
						    rd_rdma_start[mylocal_rd_put_flag_offset+1]=RdTree_GetMsgSize(URtree_ptr[lk],'d')*nrhs+LSUM_H;
						    rd_rdma_start[mylocal_rd_put_flag_offset+2]=1;
						    //RdTree_forwardMessageSimple(URtree_ptr[lk],&lsum[il-LSUM_H],RdTree_GetMsgSize(URtree_ptr[lk],'d')*nrhs+LSUM_H,'d');
						    RdTree_forwardMessageOneSide(URtree_ptr[lk],'d', &rd_rdma_start[mylocal_rd_put_flag_offset], Pc, RDcount, RDbase);
					        mylocal_rd_put_flag_offset += RDMA_FLAG_SIZE;
#if ( PROFlevel>=1 )
						TOC(t2, t1);
						stat_loc[thread_id]->utime[SOL_COMM] += t2;
#endif
					        //printf("In U RD solve (%d)--After RD send \n",iam);
                            //fflush(stdout);
				        }//if(RdTree_IsRoot(URtree_ptr[lk],'d')==YES)
                }//if ( bmod_tmp==0 )
            if (RDis_solved[recvRankNum] == BufSize_urd[recvRankNum]) {
                validRDQindex_u[rdidx]=-1;
                shift += 1;
                //printf("iam=%d,shift=%d\n",iam,shift);
                //fflush(stdout);
            }else{
                if(shift>0){
                     validRDQindex_u[rdidx-shift]=validRDQindex_u[rdidx];
                     validRDQindex_u[rdidx]=-1;
                     //printf("iam=%d,Now shift %d to %d\n",iam,bcidx,bcidx-shift);
                     //fflush(stdout);
                }
            }
            //printf("In U RD solve, iam=%d, RDis_solved[%d]=%d,BufSize_urd[%d]=%d\n",iam,recvRankNum,RDis_solved[recvRankNum],recvRankNum,BufSize_urd[recvRankNum]);
            //fflush(stdout);

            } // if (rd_pget_count[rdidx]>0)
        }//for (rdidx=0;rdidx<Pc;rdidx++)
        }
        nbrecv1 = totalsolveBC + totalsolveRD;
}


#else
#ifdef _OPENMP
#pragma omp parallel default (shared)
#endif
    {
#ifdef _OPENMP
#pragma omp master
#endif
        for ( nbrecv =0; nbrecv<nbrecvx+nbrecvmod;nbrecv++) { /* While not finished. */

            // printf("iam %4d nbrecv %4d nbrecvx %4d nbrecvmod %4d\n", iam, nbrecv, nbrecvxnbrecvmod);
            // fflush(stdout);

            thread_id = 0;
#if ( PROFlevel>=1 )
            TIC(t1);
#endif

            recvbuf0 = &recvbuf_BC_fwd[nbrecvx_buf*maxrecvsz];

            /* Receive a message. */
            MPI_Recv( recvbuf0, maxrecvsz, MPI_DOUBLE,
                      MPI_ANY_SOURCE, MPI_ANY_TAG, grid->comm, &status );

#if ( PROFlevel>=1 )
            TOC(t2, t1);
			stat_loc[thread_id]->utime[SOL_COMM] += t2;

			msg_cnt += 1;
			msg_vol += maxrecvsz * dword;
#endif

            k = *recvbuf0;
#if ( DEBUGlevel>=2 )
            printf("(%2d) Recv'd block %d, tag %2d\n", iam, k, status.MPI_TAG);
			fflush(stdout);
#endif

            if(status.MPI_TAG==BC_U){
                // --nfrecvx;
                nbrecvx_buf++;

                lk = LBj( k, grid );    /* local block number */

                if(BcTree_getDestCount(UBtree_ptr[lk],'d')>0){

                    BcTree_forwardMessageSimple(UBtree_ptr[lk],recvbuf0,BcTree_GetMsgSize(UBtree_ptr[lk],'d')*nrhs+XK_H,'d');
                    // nfrecvx_buf++;
                }

                /*
                 * Perform local block modifications: lsum[i] -= L_i,k * X[k]
                 */

                lk = LBj( k, grid ); /* Local block number, column-wise. */
                dlsum_bmod_inv_master(lsum, x, &recvbuf0[XK_H], rtemp, nrhs, k, bmod, Urbs,Urbs2,
                                      Ucb_indptr, Ucb_valptr, xsup, grid, Llu,
                                      send_req, stat_loc, sizelsum,sizertemp,thread_id,num_thread);
            }else if(status.MPI_TAG==RD_U){

                lk = LBi( k, grid ); /* Local block number, row-wise. */

                knsupc = SuperSize( k );
                tempv = &recvbuf0[LSUM_H];
                il = LSUM_BLK( lk );
                RHS_ITERATE(j) {
#ifdef _OPENMP
#pragma omp simd
#endif
                    for (i = 0; i < knsupc; ++i)
                        lsum[i + il + j*knsupc + thread_id*sizelsum] += tempv[i + j*knsupc];

                }
                // #ifdef _OPENMP
                // #pragma omp atomic capture
                // #endif
                bmod_tmp=--bmod[lk*aln_i];
                thread_id = 0;
                rtemp_loc = &rtemp[sizertemp* thread_id];
                if ( bmod_tmp==0 ) {
                    if(RdTree_IsRoot(URtree_ptr[lk],'d')==YES){

                        knsupc = SuperSize( k );
                        for (ii=1;ii<num_thread;ii++)
#ifdef _OPENMP
#pragma omp simd
#endif
                                for (jj=0;jj<knsupc*nrhs;jj++)
                                    lsum[il+ jj ] += lsum[il + jj + ii*sizelsum];

                        ii = X_BLK( lk );
                        RHS_ITERATE(j)
#ifdef _OPENMP
#pragma omp simd
#endif
                                for (i = 0; i < knsupc; ++i)
                                    x[i + ii + j*knsupc] += lsum[i + il + j*knsupc ];

                        lk = LBj( k, grid ); /* Local block number, column-wise. */
                        lsub = Lrowind_bc_ptr[lk];
                        lusup = Lnzval_bc_ptr[lk];
                        nsupr = lsub[1];

                        if(Llu->inv == 1){

                            Uinv = Uinv_bc_ptr[lk];

#ifdef _CRAY
                            SGEMM( ftcs2, ftcs2, &knsupc, &nrhs, &knsupc,
									&alpha, Uinv, &knsupc, &x[ii],
									&knsupc, &beta, rtemp_loc, &knsupc );
#elif defined (USE_VENDOR_BLAS)
                            dgemm_( "N", "N", &knsupc, &nrhs, &knsupc,
									&alpha, Uinv, &knsupc, &x[ii],
									&knsupc, &beta, rtemp_loc, &knsupc, 1, 1 );
#else
                            dgemm_( "N", "N", &knsupc, &nrhs, &knsupc,
                                    &alpha, Uinv, &knsupc, &x[ii],
                                    &knsupc, &beta, rtemp_loc, &knsupc );
#endif

#ifdef _OPENMP
#pragma omp simd
#endif
                            for (i=0 ; i<knsupc*nrhs ; i++){
                                x[ii+i] = rtemp_loc[i];
                            }
                        }else{
#ifdef _CRAY
                            STRSM(ftcs1, ftcs3, ftcs2, ftcs2, &knsupc, &nrhs, &alpha,
									lusup, &nsupr, &x[ii], &knsupc);
#elif defined (USE_VENDOR_BLAS)
                            dtrsm_("L", "U", "N", "N", &knsupc, &nrhs, &alpha,
									lusup, &nsupr, &x[ii], &knsupc, 1, 1, 1, 1);
#else
                            dtrsm_("L", "U", "N", "N", &knsupc, &nrhs, &alpha,
                                   lusup, &nsupr, &x[ii], &knsupc);
#endif
                        }

#if ( PROFlevel>=1 )
                        TOC(t2, t1);
							stat_loc[thread_id]->utime[SOL_TRSM] += t2;
#endif
                        stat_loc[thread_id]->ops[SOLVE] += knsupc * (knsupc + 1) * nrhs;

#if ( DEBUGlevel>=2 )
                        printf("(%2d) Solve X[%2d]\n", iam, k);
#endif

                        /*
                         * Send Xk to process column Pc[k].
                         */
                        if(UBtree_ptr[lk]!=NULL){
                            BcTree_forwardMessageSimple(UBtree_ptr[lk],&x[ii - XK_H],BcTree_GetMsgSize(UBtree_ptr[lk],'d')*nrhs+XK_H,'d');
                        }


                        /*
                         * Perform local block modifications:
                         *         lsum[i] -= U_i,k * X[k]
                         */
                        if ( Urbs[lk] )
                            dlsum_bmod_inv_master(lsum, x, &x[ii], rtemp, nrhs, k, bmod, Urbs,Urbs2,
                                                  Ucb_indptr, Ucb_valptr, xsup, grid, Llu,
                                                  send_req, stat_loc, sizelsum,sizertemp,thread_id,num_thread);

                    }else{
                        il = LSUM_BLK( lk );
                        knsupc = SuperSize( k );

                        for (ii=1;ii<num_thread;ii++)
#ifdef _OPENMP
#pragma omp simd
#endif
                                for (jj=0;jj<knsupc*nrhs;jj++)
                                    lsum[il+ jj ] += lsum[il + jj + ii*sizelsum];

                        RdTree_forwardMessageSimple(URtree_ptr[lk],&lsum[il-LSUM_H],RdTree_GetMsgSize(URtree_ptr[lk],'d')*nrhs+LSUM_H,'d');
                    }

                }
            }
        } /* while not finished ... */
    }
    //PROFILE_LND_FINISH();
#endif
#if ( PRNTlevel>=1 )
    t = SuperLU_timer_() - t;
		stat->utime[SOL_TOT] += t;
		if ( !iam ) printf(".. U-solve time\t%8.4f\n", t);
		MPI_Reduce (&t, &tmax, 1, MPI_DOUBLE,
				MPI_MAX, 0, grid->comm);
		if ( !iam ) {
			printf(".. U-solve time (MAX) \t%8.4f\n", tmax);
			fflush(stdout);
		}
		t = SuperLU_timer_();
#endif



#if ( DEBUGlevel>=2 )
    {
			double *x_col;
			int diag;
			printf("\n(%d) .. After U-solve: x (ON DIAG PROCS) = \n", iam);
			ii = 0;
			for (k = 0; k < nsupers; ++k) {
				knsupc = SuperSize( k );
				krow = PROW( k, grid );
				kcol = PCOL( k, grid );
				diag = PNUM( krow, kcol, grid);
				if ( iam == diag ) { /* Diagonal process. */
					lk = LBi( k, grid );
					jj = X_BLK( lk );
					x_col = &x[jj];
					RHS_ITERATE(j) {
						for (i = 0; i < knsupc; ++i) { /* X stored in blocks */
							printf("\t(%d)\t%4d\t%.10f\n",
									iam, xsup[k]+i, x_col[i]);
						}
						x_col += knsupc;
					}
				}
				ii += knsupc;
			} /* for k ... */
		}
#endif
//PROFILE_ICE_INIT();
    pdReDistribute_X_to_B(n, B, m_loc, ldb, fst_row, nrhs, x, ilsum,
                          ScalePermstruct, Glu_persist, grid, SOLVEstruct);
//PROFILE_ICE_FINISH();


#if ( PRNTlevel>=1 )
    t = SuperLU_timer_() - t;
		if ( !iam) {
		    printf(".. X to B redistribute time\t%8.4f\n", t);
		    fflush(stdout);
		}
		t = SuperLU_timer_();
#endif


    double tmp1=0;
    double tmp2=0;
    double tmp3=0;
    double tmp4=0;
    for(i=0;i<num_thread;i++){
        tmp1 = MAX(tmp1,stat_loc[i]->utime[SOL_TRSM]);
        tmp2 = MAX(tmp2,stat_loc[i]->utime[SOL_GEMM]);
        tmp3 = MAX(tmp3,stat_loc[i]->utime[SOL_COMM]);
        tmp4 += stat_loc[i]->ops[SOLVE];
#if ( PRNTlevel>=2 )
        if(iam==0)printf("thread %5d gemm %9.5f\n",i,stat_loc[i]->utime[SOL_GEMM]);
#endif
    }


    stat->utime[SOL_TRSM] += tmp1;
    stat->utime[SOL_GEMM] += tmp2;
    stat->utime[SOL_COMM] += tmp3;
    stat->ops[SOLVE]+= tmp4;


    /* Deallocate storage. */
    for(i=0;i<num_thread;i++){
        PStatFree(stat_loc[i]);
        SUPERLU_FREE(stat_loc[i]);
    }
    SUPERLU_FREE(stat_loc);
    SUPERLU_FREE(rtemp);


    SUPERLU_FREE(bmod);
    SUPERLU_FREE(brecv);
    SUPERLU_FREE(root_send);

    SUPERLU_FREE(rootsups);
#ifdef oneside
    //foMPI_Win_unlock_all(bc_winl);
    //foMPI_Win_unlock_all(rd_winl);
    SUPERLU_FREE(BC_taskq);
    SUPERLU_FREE(RD_taskq);
#elif defined (pget)
    /* if free, free a freed pointer */
    //if (Pr > 1) SUPERLU_FREE(tmp_buf_bc);
    //if (Pc > 1) SUPERLU_FREE(tmp_buf_rd);
    //SUPERLU_FREE(bc_pget_count);
    //SUPERLU_FREE(rd_pget_count);
#else
    SUPERLU_FREE(recvbuf_BC_fwd);
    SUPERLU_FREE(lsum);
    SUPERLU_FREE(x);
#endif


#if ( PROFlevel>=2 )
    {
			float msg_vol_max, msg_vol_sum, msg_cnt_max, msg_cnt_sum;

			MPI_Reduce (&msg_cnt, &msg_cnt_sum,
					1, MPI_FLOAT, MPI_SUM, 0, grid->comm);
			MPI_Reduce (&msg_cnt, &msg_cnt_max,
					1, MPI_FLOAT, MPI_MAX, 0, grid->comm);
			MPI_Reduce (&msg_vol, &msg_vol_sum,
					1, MPI_FLOAT, MPI_SUM, 0, grid->comm);
			MPI_Reduce (&msg_vol, &msg_vol_max,
					1, MPI_FLOAT, MPI_MAX, 0, grid->comm);
			if (!iam) {
				printf ("\tPDGSTRS comm stat:"
						"\tAvg\tMax\t\tAvg\tMax\n"
						"\t\t\tCount:\t%.0f\t%.0f\tVol(MB)\t%.2f\t%.2f\n",
						msg_cnt_sum / Pr / Pc, msg_cnt_max,
						msg_vol_sum / Pr / Pc * 1e-6, msg_vol_max * 1e-6);
			}
		}
#endif

    stat->utime[SOLVE] = SuperLU_timer_() - t1_sol;

#if ( DEBUGlevel>=1 )
    CHECK_MALLOC(iam, "Exit pdgstrs()");
#endif

    return;
} /* PDGSTRS */

