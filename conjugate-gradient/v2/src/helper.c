// Sequential version of CG
// Author: Tim Lebailly

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <time.h>

#include "helper.h"

/*
 * Generates a dense PSD symmetric SIZE x SIZE matrix.
 */
float *generate_A(int N)
{
  int i, j;
  float *A = malloc(sizeof(float) * N * N);

  float temp;

  for (i = 0; i < N; i++)
  {
    for (j = 0; j <= i; j++)
    {
      temp = (float)rand() / RAND_MAX;
      if (i == j)
      {
        A(i, j, N) = temp + N;
      }
      else
      {
        A(i, j, N) = temp;
        A(j, i, N) = temp;
      }
    }
  }

  return A;
}

/*
 * Generates a random vector of size SIZE
 */
float *generate_b(int N)
{
  int i;
  float *b = malloc(sizeof(float) * N);
  for (i = 0; i < N; i++)
  {
    b[i] = (float)rand() / RAND_MAX;
  }
  return b;
}

/*
 * Prints a formated matrix
 * Input: pointer to 1D-array-stored matrix (row major)
 */
void print_mat(float *A, int rows, int cols)
{
  int i;
  for (i = 0; i < rows * cols; i++)
  {
    printf("%.3e ", A[i]);
    if ((i + 1) % cols == 0)
    {
      printf("\n");
    }
  }
  printf("\n");
}

/*
 * Prints a formated vector
 * Input: pointer to 1D-array-stored vector
 */
void print_vec(float *b, int N)
{
  printf("__begin_vector__\n");
  int i;
  for (i = 0; i < N; i++)
  {
    if (b[i] > 0)
    {
      printf("+%.6e\n", b[i]);
    }
    else
    {
      printf("%.6e\n", b[i]);
    }
  }
  printf("__end_vector__\n");
}

/*
 * Computes a matrix vector product
 * Input: pointer to 1D-array-stored matrix (row major), 1D-array-stored vector
 * Stores the product in memory at the location of the pointer out
 */
void mat_vec(float *A, float *b, float *out, int rows, int cols)
{
  int i, j;
  for (i = 0; i < rows; i++)
  {
    out[i] = 0;
    for (j = 0; j < cols; j++)
    {
      out[i] += A(i, j, cols) * b(j);
    }
  }
}

/*
 * Computes the scalar product of 2 vectors
 * Input: pointer to 1D-array-stored vector, pointer 1D-array-stored vector
 * Output: float scalar product
 */
float vec_vec(float *vec1, float *vec2, int N)
{
  int i;
  float product = 0;
  for (i = 0; i < N; i++)
  {
    product += vec1[i] * vec2[i];
  }
  return product;
}

/*
 * Computes the sum of 2 vectors
 * Input: pointer to 1D-array-stored vector, pointer to 1D-array-stored vector
 * Stores the sum in memory at the location of the pointer out
 */
void vec_plus_vec(float *vec1, float *vec2, float *out, int N)
{
  int i;
  for (i = 0; i < N; i++)
  {
    out[i] = vec1[i] + vec2[i];
  }
}

/*
 * Computes a scalar vector product
 * Input: scalar, pointer to 1D-array-stored vector
 * Stores the product in memory at the location of the pointer out
 */
void scalar_vec(float alpha, float *vec2, float *out, int N)
{
  int i;
  for (i = 0; i < N; i++)
  {
    out[i] = alpha * vec2[i];
  }
}

/*
 * Computes a scalar matrix vector product
 * Input: scalar, pointer to 1D-array-stored matrix (row major), pointer to 1D-array-stored vector
 * Stores the product in memory at the location of the pointer out
 */
void scalar_mat_vec(float alpha, float *A, float *b, float *out, int rows, int cols)
{
  int i, j;
  for (i = 0; i < rows; i++)
  {
    out[i] = 0;
    for (j = 0; j < cols; j++)
    {
      out[i] += alpha * A(i, j, cols) * b(j);
    }
  }
}

/*
 * Computes the 2-norm of a vector
 * Input: pointer to 1D-array-stored vector
 * Output: value of the norm of the vector
 */
float norm2d(float *a, int N)
{
  return sqrt(vec_vec(a, a, N));
}

/*
 * Checks if 2 vectors or matrices are equal up to some precision
 * Input: 2 1D-array-stored vector or 2 1D-array-stored matrices
 * Output: true if same (up to some precision) else false
 */
int more_or_less_equal(float *a, float *b, int N, float TOL)
{
  int i;
  for (i = 0; i < N; i++)
  {
    if (fabs(a[i] - b[i]) > TOL)
    {
      return 0;
    }
  }
  return 1;
}

void solve_cg_seq(float *A, float *b, float *x, int N, int max_iter, float eps, int *metrics_iter, float *metrics_r_norm)
{
  // Initialize temporary variables
  float *p = (float *)calloc(sizeof(float), N);
  float *r = (float *)calloc(sizeof(float), N);
  float *temp = (float *)calloc(sizeof(float), N);
  float beta, alpha, rNormOld = 0.0;
  float rNorm = 1.0;
  int k = 0;

  // Set initial variables
  scalar_mat_vec(-1.0, A, x, temp, N, N);
  vec_plus_vec(b, temp, r, N);
  scalar_vec(1.0, r, p, N);
  rNormOld = vec_vec(r, r, N);

  while ((rNorm > eps) && (k < max_iter))
  {
    // temp = A* p (only compute matrix vector product once)
    mat_vec(A, p, temp, N, N);
    // alpha_k = ...
    alpha = rNormOld / vec_vec(p, temp, N);
    // r_{k+1} = ...
    scalar_vec(-alpha, temp, temp, N);
    vec_plus_vec(r, temp, r, N);
    // x_{k+1} = ...
    scalar_vec(alpha, p, temp, N);
    vec_plus_vec(x, temp, x, N);
    // beta_k = ...
    rNorm = vec_vec(r, r, N);
    beta = rNorm / rNormOld;
    // p_{k+1} = ...
    scalar_vec(beta, p, temp, N);
    vec_plus_vec(r, temp, p, N);
    // set rOld to r
    rNormOld = rNorm;
    k++;
  }

  *metrics_iter = k;
  *metrics_r_norm = rNorm;

  free(p);
  free(r);
  free(temp);
}
