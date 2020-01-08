#include "fompi.h"
#include "checksum.h"

extern foMPI_Win bc_winl;
extern foMPI_Win rd_winl;
extern MPI_Comm row_comm;
extern MPI_Comm col_comm;
extern int *BufSize;
extern int *BufSize_rd;
extern int *keep_validBCQindex;
extern int *keep_validRDQindex;
extern int *BufSize_u;
extern int *BufSize_urd;
extern int *keep_validBCQindex_u;
extern int *keep_validRDQindex_u;
extern double onesidecomm_bc;
extern int *recv_size_all;
extern int *recv_size_all_u;
#ifdef pget
extern foMPI_Win bc_winl_get;
extern foMPI_Win rd_winl_get;
extern int* bc_pget_count;
extern int* rd_pget_count;
extern double *lsum;
extern double *x;
#endif

