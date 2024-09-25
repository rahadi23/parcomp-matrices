/**********************************************************************                                                                       * https://gist.github.com/shinde-rahul/0b38d58c3d5d7f7346210cdee90cc3b0               
 * MPI-based matrix multiplication AxB=C                                                                                                                     
 *********************************************************************/


#include "mpi.h"
#define N                 __MATRIX_N_SIZE__      /* number of rows and columns in matrix */
#include <sys/time.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

MPI_Status status;

double a[N][N],b[N],c[N];

main(int argc, char **argv)
{
  int numtasks,taskid,source,dest,rows,offset,i,j;

  struct timeval start, stop;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
  MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

  /*---------------------------- master ----------------------------*/
  if (taskid == 0) {
    for (i=0; i<N; i++) {
      for (j=0; j<N; j++) {
        a[i][j]= 1.0;
      }
      b[i] = 2.0;
    }

    gettimeofday(&start, 0);

    /* send matrix data to the worker tasks */
    rows = N/numtasks;
    offset = 0;

    for (dest=0; dest<numtasks; dest++)
    {
      MPI_Send(&offset, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
      MPI_Send(&rows, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
      MPI_Send(&a[offset][0], rows*N, MPI_DOUBLE,dest,1, MPI_COMM_WORLD);
      MPI_Send(&b, N, MPI_DOUBLE, dest, 1, MPI_COMM_WORLD);
      offset = offset + rows;
    }

    /* wait for results from all worker tasks */
    for (i=0; i<numtasks; i++)
    {
      source = i;
      MPI_Recv(&offset, 1, MPI_INT, source, 2, MPI_COMM_WORLD, &status);
      MPI_Recv(&rows, 1, MPI_INT, source, 2, MPI_COMM_WORLD, &status);
      MPI_Recv(&c[offset], rows, MPI_DOUBLE, source, 2, MPI_COMM_WORLD, &status);
    }

    gettimeofday(&stop, 0);

    printf("Here is the result matrix:\n");
    for (i=0; i<N; i++) {
      printf("%6.2f   ", c[i]);
      printf ("\n");
    }

    fprintf(stdout,"Time = %.6f\n\n",
         (stop.tv_sec+stop.tv_usec*1e-6)-(start.tv_sec+start.tv_usec*1e-6));

  }

  /*---------------------------- worker----------------------------*/
  source = 0;
  MPI_Recv(&offset, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &status);
  MPI_Recv(&rows, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &status);
  MPI_Recv(&a, rows*N, MPI_DOUBLE, source, 1, MPI_COMM_WORLD, &status);
  MPI_Recv(&b, N, MPI_DOUBLE, source, 1, MPI_COMM_WORLD, &status);

  /* Matrix multiplication */
  for (i=0; i<rows; i++) {
    c[i] = 0.0;

    for (j=0; j<N; j++) {
      c[i] = c[i] + a[i][j] * b[j];
    }
  }


  MPI_Send(&offset, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
  MPI_Send(&rows, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
  MPI_Send(&c, rows, MPI_DOUBLE, 0, 2, MPI_COMM_WORLD);

  MPI_Finalize();

  return 0;
}
