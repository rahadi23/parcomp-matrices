#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <sys/time.h>
#include <assert.h>
#include "sstream"
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

  double mpiScatterTime = 0.0, mpiBcastTime = 0.0, mpiGatherTime = 0.0, compTime = 0.0, idleTime = 0.0;
  std::stringstream tl;

  tl << std::setw(2) << rank << ": [INFO] Timeline: ";

  Logger logger(&tl);

  int **A = NULL, **B = NULL, **C = NULL, **localA = NULL, **localC = NULL;

  allocMatrix(&A, N, N);
  assert(A != NULL);

  allocMatrix(&B, N, N);
  assert(B != NULL);

  if (rank == 0)
  {

    fprintf(stdout, "%2d: [INFO] N: %d, NP: %d\n", rank, N, size);

    for (i = 0; i < N; i++)
    {
      for (j = 0; j < N; j++)
      {
        A[i][j] = 1;
        B[i][j] = 2;
      }
    }

    // fprintf(stdout, "%2d: [INFO] Matrix A\n", rank);
    // printMatrix(&A, N, N);

    // fprintf(stdout, "%2d: [INFO] Matrix B\n", rank);
    // printMatrix(&B, N, N);
  }
  logger.log(&compTime, "COMP");

  MPI_Barrier(MPI_COMM_WORLD);
  // totalTime -= MPI_Wtime();
  logger.log(&idleTime, "IDLE");

  allocMatrix(&localA, rowsPerTask, N);
  assert(localA != NULL);
  logger.log(&compTime, "COMP");

  // Scatter
  MPI_Barrier(MPI_COMM_WORLD);
  // mpiScatterTime -= MPI_Wtime();
  logger.log(&idleTime, "IDLE");

  MPI_Scatter(&(A[0][0]), rowsPerTask * N, MPI_INT, &(localA[0][0]), rowsPerTask * N, MPI_INT, 0, MPI_COMM_WORLD);
  logger.log(&mpiScatterTime, "MPI_Scatter");

  MPI_Barrier(MPI_COMM_WORLD);
  // mpiScatterTime += MPI_Wtime();
  logger.log(&idleTime, "IDLE");

  // Bcast
  MPI_Barrier(MPI_COMM_WORLD);
  // mpiBcastTime -= MPI_Wtime();
  logger.log(&idleTime, "IDLE");

  MPI_Bcast(&(B[0][0]), N * N, MPI_INT, 0, MPI_COMM_WORLD);
  logger.log(&mpiBcastTime, "MPI_Bcast");

  MPI_Barrier(MPI_COMM_WORLD);
  // mpiBcastTime += MPI_Wtime();
  logger.log(&idleTime, "IDLE");

  allocMatrix(&localC, rowsPerTask, N);
  assert(localC != NULL);

  // compTime -= MPI_Wtime();
  matrixMultiply(localA, B, rowsPerTask, N, &localC);
  // compTime += MPI_Wtime();

  allocMatrix(&C, N, N);
  assert(C != NULL);
  logger.log(&compTime, "COMP");

  // Gather
  MPI_Barrier(MPI_COMM_WORLD);
  logger.log(&idleTime, "IDLE");
  // mpiGatherTime -= MPI_Wtime();

  MPI_Gather(&(localC[0][0]), rowsPerTask * N, MPI_INT, &C[0][0], rowsPerTask * N, MPI_INT, 0, MPI_COMM_WORLD);
  logger.log(&mpiGatherTime, "MPI_Gather");

  MPI_Barrier(MPI_COMM_WORLD);
  logger.log(&idleTime, "IDLE");
  // mpiGatherTime += MPI_Wtime();

  MPI_Barrier(MPI_COMM_WORLD);
  logger.log(&idleTime, "IDLE");
  // totalTime += MPI_Wtime();

  // if (rank == 0)
  // {
  //   // Print result
  //   fprintf(stdout, "%2d: [INFO] Matrix result\n", rank);
  //   printMatrix(&C, N, N);
  // }

  free(A);
  free(B);
  free(C);
  free(localA);
  free(localC);
  logger.log(&compTime, "COMP");

  MPI_Barrier(MPI_COMM_WORLD);
  logger.log(&idleTime, "IDLE");

  MPI_Finalize();

  double commTime = mpiScatterTime + mpiBcastTime + mpiGatherTime;
  double totalTime = commTime + compTime + idleTime;

  fprintf(stdout, "%2d: [INFO] Sctr. time: %.6f\n", rank, mpiScatterTime);
  fprintf(stdout, "%2d: [INFO] Bcst. time: %.6f\n", rank, mpiBcastTime);
  fprintf(stdout, "%2d: [INFO] Gthr. time: %.6f\n", rank, mpiGatherTime);
  fprintf(stdout, "%2d: [INFO] COMM. TIME: %.6f\n", rank, commTime);
  fprintf(stdout, "%2d: [INFO] COMP. TIME: %.6f\n", rank, compTime);
  fprintf(stdout, "%2d: [INFO] TOTAL TIME: %.6f\n", rank, totalTime);
  fprintf(stdout, "%2d: [INFO] IDLE  TIME: %.6f\n", rank, idleTime);
  std::cout << tl.str() << std::endl;

  return 0;
}
