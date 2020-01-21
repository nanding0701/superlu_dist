#include "fompi.h"

#ifdef oneside
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
extern int *recv_size_all;
extern int *recv_size_all_u;
extern double* BC_taskq;
extern double* RD_taskq;
#endif

#ifdef pget
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
extern int *recv_size_all;
extern int *recv_size_all_u;
extern foMPI_Win bc_winl_get;
extern foMPI_Win rd_winl_get;
extern foMPI_Win tmp_rd_winl_get;
extern foMPI_Win tmp_bc_winl_get;
extern int* bc_pget_count;
extern int* rd_pget_count;
extern double *lsum;
extern double *x;
extern double *tmp_buf_bc;
extern double *tmp_buf_rd;
extern int mysendmsg_num;
extern int mysendmsg_num_u;
extern int mysendmsg_num_rd;
extern int mysendmsg_num_urd;
#define RDMA_FLAG_SIZE 3
#endif

