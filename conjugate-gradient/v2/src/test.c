#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  int rank, size;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  float A = 0.0;

  if (rank == 0)
  {
    A = 1.0;
  }

  MPI_Bcast(&A, 1, MPI_FLOAT, 1, MPI_COMM_WORLD);

  fprintf(stdout, "[%d] A: %f\n", rank, A);

  MPI_Finalize();

  return 0;
}
