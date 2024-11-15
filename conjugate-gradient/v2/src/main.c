#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <assert.h>

#include "helper.h"

#define MATRIX_SIZE 40960
#define MAX_ITER 1000
#define EPS 1.0e-10
#define TOL 1.0e-10

int main(int argc, char *argv[])
{
  int all_rank, all_size, all_k = 0;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &all_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &all_size);

  int mat_size_per_rank = MATRIX_SIZE * MATRIX_SIZE / all_size;
  int vec_size_per_rank = MATRIX_SIZE / all_size;

  float root_beta, all_alpha, root_r_norm_old;
  float all_r_norm = 1.0;

  float *root_A, *root_x;
  float *local_sub_A = (float *)calloc(sizeof(float), mat_size_per_rank);
  float *local_sub_x = (float *)calloc(sizeof(float), vec_size_per_rank);
  float *all_b = (float *)calloc(sizeof(float), MATRIX_SIZE);

  float *root_temp, *root_r;
  float *local_sub_temp = (float *)calloc(sizeof(float), vec_size_per_rank);
  float *local_sub_r = (float *)calloc(sizeof(float), vec_size_per_rank);
  float *all_p = (float *)calloc(sizeof(float), MATRIX_SIZE);

  double root_sys_time = 0.0;

  if (all_rank == 0)
  {
    fprintf(stdout, "[%2d] mat_size: %d, vec_size: %d\n",
            all_rank, mat_size_per_rank, vec_size_per_rank);

    root_A = generate_A(MATRIX_SIZE);
    all_b = generate_b(MATRIX_SIZE);

    root_sys_time -= MPI_Wtime();

    root_x = (float *)calloc(sizeof(float), MATRIX_SIZE);
    root_temp = (float *)calloc(sizeof(float), MATRIX_SIZE);
    root_r = (float *)calloc(sizeof(float), MATRIX_SIZE);

    // Set initial variables
    scalar_mat_vec(-1.0, root_A, root_x, root_temp, MATRIX_SIZE, MATRIX_SIZE);
    vec_plus_vec(all_b, root_temp, root_r, MATRIX_SIZE);
    scalar_vec(1.0, root_r, all_p, MATRIX_SIZE);

    root_r_norm_old = vec_vec(root_r, root_r, MATRIX_SIZE);
  }

  MPI_Scatter(&root_A[0], mat_size_per_rank, MPI_FLOAT,
              &local_sub_A[0], mat_size_per_rank, MPI_FLOAT,
              0, MPI_COMM_WORLD);

  MPI_Bcast(&all_b[0], MATRIX_SIZE, MPI_FLOAT, 0, MPI_COMM_WORLD);

  MPI_Scatter(&root_r[0], vec_size_per_rank, MPI_FLOAT,
              &local_sub_r[0], vec_size_per_rank, MPI_FLOAT,
              0, MPI_COMM_WORLD);

  while ((all_r_norm > EPS) && (all_k < MAX_ITER))
  {
    MPI_Scatter(&root_temp[0], vec_size_per_rank, MPI_FLOAT,
                &local_sub_temp[0], vec_size_per_rank, MPI_FLOAT,
                0, MPI_COMM_WORLD);

    MPI_Bcast(&all_p[0], MATRIX_SIZE, MPI_FLOAT, 0, MPI_COMM_WORLD);

    // temp = A* p (only compute matrix vector product once)
    mat_vec(local_sub_A, all_p, local_sub_temp, vec_size_per_rank, MATRIX_SIZE);

    MPI_Gather(&local_sub_temp[0], vec_size_per_rank, MPI_FLOAT,
               &root_temp[0], vec_size_per_rank, MPI_FLOAT,
               0, MPI_COMM_WORLD);

    // alpha_k = ...
    if (all_rank == 0)
    {
      all_alpha = root_r_norm_old / vec_vec(all_p, root_temp, MATRIX_SIZE);
    }

    MPI_Bcast(&all_alpha, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);

    // r_{k+1} = ...
    scalar_vec(-all_alpha, local_sub_temp, local_sub_temp, vec_size_per_rank);
    vec_plus_vec(local_sub_r, local_sub_temp, local_sub_r, vec_size_per_rank);

    // x_{k+1} = ...
    // &all_p[rank * vec_size_per_rank]: some parts of p (not entirely) based on partition
    scalar_vec(all_alpha, &all_p[all_rank * vec_size_per_rank], local_sub_temp, vec_size_per_rank);
    vec_plus_vec(local_sub_x, local_sub_temp, local_sub_x, vec_size_per_rank);

    MPI_Gather(&local_sub_r[0], vec_size_per_rank, MPI_FLOAT,
               &root_r[0], vec_size_per_rank, MPI_FLOAT,
               0, MPI_COMM_WORLD);

    MPI_Gather(&local_sub_x[0], vec_size_per_rank, MPI_FLOAT,
               &root_x[0], vec_size_per_rank, MPI_FLOAT,
               0, MPI_COMM_WORLD);

    if (all_rank == 0)
    {
      // beta_k = ...
      all_r_norm = vec_vec(root_r, root_r, MATRIX_SIZE);
      root_beta = all_r_norm / root_r_norm_old;

      // p_{k+1} = ...
      scalar_vec(root_beta, all_p, root_temp, MATRIX_SIZE);
      vec_plus_vec(root_r, root_temp, all_p, MATRIX_SIZE);

      root_r_norm_old = all_r_norm;
    }

    MPI_Bcast(&all_r_norm, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);

    all_k++;
  }

  MPI_Barrier(MPI_COMM_WORLD);

  root_sys_time += MPI_Wtime();

  if (all_rank == 0)
  {
    float *root_x_seq = (float *)calloc(sizeof(float), MATRIX_SIZE);

    int root_k_seq = 0;
    float root_r_norm_seq = 1.0;
    double root_sys_time_seq = 0.0;

    root_sys_time_seq -= MPI_Wtime();

    solve_cg_seq(root_A, all_b, root_x_seq,
                 MATRIX_SIZE, MAX_ITER, EPS,
                 &root_k_seq, &root_r_norm_seq);

    root_sys_time_seq += MPI_Wtime();

    int ok = more_or_less_equal(root_x, root_x_seq, MATRIX_SIZE, TOL);
    assert(ok);

    fprintf(stdout, "+--------+--------+--------+--------------+--------------+--------+--------------+--------------+--------+\n");
    fprintf(stdout, "|      N | max_it |     np |     sys_time |       r_norm |     it | seq_sys_time |   seq_r_norm | seq_it |\n");
    fprintf(stdout, "+--------+--------+--------+--------------+--------------+--------+--------------+--------------+--------+\n");

    fprintf(stdout, "| %6d | %6d | %6d | %12.6f | %12.6e | %6d | %12.6f | %12.6e | %6d |\n",
            MATRIX_SIZE, MAX_ITER, all_size,
            root_sys_time, all_r_norm, all_k,
            root_sys_time_seq, root_r_norm_seq, root_k_seq);

    fprintf(stdout, "+--------+--------+--------+--------------+--------------+--------+--------------+--------------+--------+\n");

    free(root_A);
    free(root_x);
    free(root_x_seq);
    free(root_r);
    free(root_temp);
  }

  free(all_b);
  free(all_p);
  free(local_sub_A);
  free(local_sub_x);
  free(local_sub_r);
  free(local_sub_temp);

  MPI_Finalize();

  return 0;
}
