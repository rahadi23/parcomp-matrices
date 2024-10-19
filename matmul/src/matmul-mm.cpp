#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <sys/time.h>
#include <sstream>
#include "logger.h"

void parseArgs(int argc, char *argv[], int *N)
{
  char *cp;
  long LN;

  // Check for the right number of arguments
  if (argc != 2)
  {
    fprintf(stderr, "[ERROR] Must be run with exactly 1 argument, found %d!\nUsage: %s <N>\n", argc - 1, argv[0]);
    exit(1);
  }

  cp = argv[1];
  if (*cp == 0)
  {
    fprintf(stderr, "[ERROR] Argument is an empty string\n");
    exit(1);
  }

  LN = strtol(cp, &cp, 10);

  if (*cp != 0)
  {
    fprintf(stderr, "[ERROR] Argument '%s' is not an integer -- '%s'\n", argv[1], cp);
    exit(1);
  }

  *N = (int)LN;
}

int allocMatrix(int ***mat, int rows, int cols)
{
  // Allocate rows*cols contiguous items
  int *p = (int *)malloc(sizeof(int *) * rows * cols);
  if (!p)
  {
    return -1;
  }
  // Allocate row pointers
  *mat = (int **)malloc(rows * sizeof(int *));
  if (!mat)
  {
    free(p);
    return -1;
  }

  // Set up the pointers into the contiguous memory
  for (int i = 0; i < rows; i++)
  {
    (*mat)[i] = &(p[i * cols]);
  }
  return 0;
}

void printMatrix(int ***mat, int rows, int cols)
{
  int i, j;

  for (i = 0; i < rows; i++)
  {
    for (j = 0; j < cols; j++)
    {
      printf("%6d  ", (*mat)[i][j]);
    }
    printf("\n");
  }
}

void matrixMultiply(int **A, int **B, int rows, int cols, int ***C)
{
  int k, i, j, val;

  for (k = 0; k < cols; k++)
  {
    for (i = 0; i < rows; i++)
    {
      val = 0;

      for (j = 0; j < cols; j++)
      {
        val += A[i][j] * B[j][k];
      }

      (*C)[i][k] = val;
    }
  }
}

MPI_Status status;

int main(int argc, char *argv[])
{
  int rank, size, N, i, j, k, dest, source;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  parseArgs(argc, argv, &N);

  int rowsPerTask = N / size;
  int rowOffset = rowsPerTask;

  std::stringstream tl;
  tl << std::setw(2) << rank << ": [INFO] Timeline: ";

  double compTime = 0;
  double mpiSendTime = 0.0;
  double mpiRecvTime = 0.0;
  double mpiSendRecvTime = 0.0;

  // start profiling
  Logger logger(&tl);

  int **A = NULL, **B = NULL, **C = NULL, **localA = NULL, **localB = NULL, **localC = NULL;

  if (rank == 0)
  {
    fprintf(stdout, "%2d: [INFO] N: %d, NP: %d\n", rank, N, size);

    struct timeval start, stop;

    allocMatrix(&A, N, N);
    allocMatrix(&B, N, N);

    for (i = 0; i < N; i++)
    {
      for (j = 0; j < N; j++)
      {
        A[i][j] = i + 1;
        B[i][j] = 1;
      }
    }
    logger.log(&compTime, "COMP");

    // fprintf(stdout, "%2d: [INFO] Matrix A\n", rank);
    // printMatrix(N, N, A);

    // fprintf(stdout, "%2d: [INFO] Matrix B\n", rank);
    // printMatrix(N, N, B);

    gettimeofday(&start, 0);

    // Send task (tag=1*)
    for (dest = 1; dest < size; dest++)
    {
      MPI_Send(&rowOffset, 1, MPI_INT, dest, 10, MPI_COMM_WORLD);
      MPI_Send(&(A[rowOffset][0]), rowsPerTask * N, MPI_INT, dest, 11, MPI_COMM_WORLD);
      MPI_Send(&(B[0][0]), N * N, MPI_INT, dest, 12, MPI_COMM_WORLD);

      // fprintf(stdout, "%2d: [INFO] Task sent to %d (rows %d, N %d)\n", rank, dest, rowsPerTask, N);

      rowOffset += rowsPerTask;
    }
    logger.log(&mpiSendTime, "MPI_Send");

    allocMatrix(&localA, rowsPerTask, N);
    allocMatrix(&localB, N, N);
    logger.log(&compTime, "COMP");

    // Send & receive task to/from self (rank 0)
    MPI_Sendrecv(&(A[0][0]), rowsPerTask * N, MPI_INT, 0, 13, &(localA[0][0]), rowsPerTask * N, MPI_INT, 0, 13, MPI_COMM_WORLD, &status);
    MPI_Sendrecv(&(B[0][0]), N * N, MPI_INT, 0, 14, &(localB[0][0]), N * N, MPI_INT, 0, 14, MPI_COMM_WORLD, &status);
    logger.log(&mpiSendRecvTime, "MPI_Sendrecv");

    allocMatrix(&C, N, N);
    logger.log(&compTime, "COMP");

    // Receive result (tag=2*)
    for (source = 1; source < size; source++)
    {
      MPI_Recv(&rowOffset, 1, MPI_INT, source, 20, MPI_COMM_WORLD, &status);
      MPI_Recv(&(C[rowOffset][0]), rowsPerTask * N, MPI_INT, source, 21, MPI_COMM_WORLD, &status);

      // fprintf(stdout, "%2d: [INFO] Result received from %d\n", rank, source);
    }
    logger.log(&mpiRecvTime, "MPI_Recv");

    allocMatrix(&localC, rowsPerTask, N);

    // Multiply in rank 0
    matrixMultiply(localA, localB, rowsPerTask, N, &localC);
    logger.log(&compTime, "COMP");

    // Send & receive result to/from self (rank 0)
    MPI_Sendrecv(&(localC[0][0]), rowsPerTask * N, MPI_INT, 0, 22, &(C[0][0]), rowsPerTask * N, MPI_INT, 0, 22, MPI_COMM_WORLD, &status);
    logger.log(&mpiSendRecvTime, "MPI_Sendrecv");

    gettimeofday(&stop, 0);

    // Print result
    // fprintf(stdout, "%2d: [INFO] Matrix result\n", rank);
    // printMatrix(&C, N, N);

    fprintf(stdout, "%2d: [INFO] Sys time: %.6f\n", rank, (stop.tv_sec + stop.tv_usec * 1e-6) - (start.tv_sec + start.tv_usec * 1e-6));
  }

  if (rank > 0)
  {
    allocMatrix(&A, rowsPerTask, N);
    allocMatrix(&B, N, N);
    logger.log(&compTime, "COMP");

    // Receive task (tag=1)
    MPI_Recv(&rowOffset, 1, MPI_INT, 0, 10, MPI_COMM_WORLD, &status);
    MPI_Recv(&(A[0][0]), rowsPerTask * N, MPI_INT, 0, 11, MPI_COMM_WORLD, &status);
    MPI_Recv(&(B[0][0]), N * N, MPI_INT, 0, 12, MPI_COMM_WORLD, &status);
    logger.log(&mpiRecvTime, "MPI_Recv");

    // fprintf(stdout, "%2d: [INFO] Task received (rows %d, N %d)\n", rank, rowsPerTask, N);

    allocMatrix(&C, rowsPerTask, N);

    // Run task
    matrixMultiply(A, B, rowsPerTask, N, &C);
    logger.log(&compTime, "COMP");

    // Send result (tag=2)
    MPI_Send(&rowOffset, 1, MPI_INT, 0, 20, MPI_COMM_WORLD);
    MPI_Send(&(C[0][0]), rowsPerTask * N, MPI_INT, 0, 21, MPI_COMM_WORLD);
    logger.log(&mpiSendTime, "MPI_Send");

    // fprintf(stdout, "%2d: [INFO] Result sent\n", rank);
  }

  MPI_Finalize();

  double mpiTime = mpiSendTime + mpiRecvTime + mpiSendRecvTime;
  double totalTime = compTime + mpiTime;

  std::cout << std::setw(2) << rank << ": [INFO] Send  time: " << std::setprecision(6) << mpiTime << std::endl;
  std::cout << std::setw(2) << rank << ": [INFO] Recv  time: " << std::setprecision(6) << mpiTime << std::endl;
  std::cout << std::setw(2) << rank << ": [INFO] SndR. time: " << std::setprecision(6) << mpiTime << std::endl;
  std::cout << std::setw(2) << rank << ": [INFO] COMM. TIME: " << std::setprecision(6) << mpiTime << std::endl;
  std::cout << std::setw(2) << rank << ": [INFO] COMP. TIME: " << std::setprecision(6) << compTime << std::endl;
  std::cout << std::setw(2) << rank << ": [INFO] TOTAL TIME: " << std::setprecision(6) << totalTime << std::endl;
  std::cout << tl.str() << std::endl;

  return 0;
}
