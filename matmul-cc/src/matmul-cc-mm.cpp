#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <sys/time.h>
#include <assert.h>

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

  int **A = NULL, **B = NULL, **C = NULL, **localA = NULL, **localC = NULL;
  double totalTime = 0.0, scatterTime = 0.0, bcastTime = 0.0, gatherTime = 0.0, compTime = 0.0;

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

  MPI_Barrier(MPI_COMM_WORLD);
  totalTime -= MPI_Wtime();

  allocMatrix(&localA, rowsPerTask, N);
  assert(localA != NULL);

  // Scatter
  MPI_Barrier(MPI_COMM_WORLD);
  scatterTime -= MPI_Wtime();

  MPI_Scatter(&(A[0][0]), rowsPerTask * N, MPI_INT, &(localA[0][0]), rowsPerTask * N, MPI_INT, 0, MPI_COMM_WORLD);

  MPI_Barrier(MPI_COMM_WORLD);
  scatterTime += MPI_Wtime();

  // Bcast
  MPI_Barrier(MPI_COMM_WORLD);
  bcastTime -= MPI_Wtime();

  MPI_Bcast(&(B[0][0]), N * N, MPI_INT, 0, MPI_COMM_WORLD);

  MPI_Barrier(MPI_COMM_WORLD);
  bcastTime += MPI_Wtime();

  allocMatrix(&localC, rowsPerTask, N);
  assert(localC != NULL);

  compTime -= MPI_Wtime();
  matrixMultiply(localA, B, rowsPerTask, N, &localC);
  compTime += MPI_Wtime();

  allocMatrix(&C, N, N);
  assert(C != NULL);

  // Gather
  MPI_Barrier(MPI_COMM_WORLD);
  gatherTime -= MPI_Wtime();

  MPI_Gather(&(localC[0][0]), rowsPerTask * N, MPI_INT, &C[0][0], rowsPerTask * N, MPI_INT, 0, MPI_COMM_WORLD);

  MPI_Barrier(MPI_COMM_WORLD);
  gatherTime += MPI_Wtime();

  MPI_Barrier(MPI_COMM_WORLD);
  totalTime += MPI_Wtime();

  fprintf(stdout, "%2d: [INFO] Sctr. time: %.6f\n", rank, scatterTime);
  fprintf(stdout, "%2d: [INFO] Bcst. time: %.6f\n", rank, bcastTime);
  fprintf(stdout, "%2d: [INFO] Gthr. time: %.6f\n", rank, gatherTime);
  fprintf(stdout, "%2d: [INFO] COMM. TIME: %.6f\n", rank, scatterTime + bcastTime + gatherTime);
  fprintf(stdout, "%2d: [INFO] COMP. TIME: %.6f\n", rank, compTime);
  fprintf(stdout, "%2d: [INFO] TOTAL TIME: %.6f\n", rank, totalTime);
  fprintf(stdout, "%2d: [INFO] IDLE  TIME: %.6f\n", rank, totalTime - (scatterTime + bcastTime + gatherTime + compTime));

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

  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();

  return 0;
}
