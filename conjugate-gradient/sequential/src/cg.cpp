#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <vector>
#include <algorithm>
#include <numeric>
#include <sys/time.h>
#include "cg.hpp"

const double NEAR_ZERO = 1.0e-10; // interpretation of "zero"

int main(int argc, char *argv[])
{
  char *cp;
  long LN;
  int N;

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

  N = (int)LN;

  fprintf(stdout, "Matrix N = %d\n", N);

  struct timeval start, stop;

  // matrix A = matrix(N, vec(N, 1.6));
  // vec B = vec(N, 10.2);
  matrix A = buildMatrix(N);
  vec B = buildVec(N);

  gettimeofday(&start, 0);
  vec X = conjugateGradientSolver(A, B);
  gettimeofday(&stop, 0);

  fprintf(stdout, "Time = %.6f\n", (stop.tv_sec + stop.tv_usec * 1e-6) - (start.tv_sec + start.tv_usec * 1e-6));

  // std::cout << "Solves AX = B" << std::endl;
  // print("\nA:", A);
  // print("\nB:", B);
  // print("\nX:", X);
  // print("\nCheck AX:", matrixTimesVector(A, X));
}

//======================================================================

vec buildVec(int N)
{
  vec V;

  for (int i = 0; i < N; i++)
  {
    V.push_back(rand());
  }

  return V;
}

matrix buildMatrix(int N)
{
  matrix M;

  for (int i = 0; i < N; i++)
  {
    vec V;
    for (int j = 0; j < N; j++)
    {
      V.push_back(rand());
    }
    M.push_back(V);
  }

  return M;
}

void print(std::string title, const vec &V)
{
  std::cout << title << '\n';

  int n = V.size();
  for (int i = 0; i < n; i++)
  {
    double x = V[i];
    // if (abs(x) < NEAR_ZERO)
    //   x = 0.0;
    std::cout << x << '\t';
  }
  std::cout << '\n';
}

//======================================================================

void print(std::string title, const matrix &A)
{
  std::cout << title << '\n';

  int m = A.size(), n = A[0].size(); // A is an m x n matrix
  for (int i = 0; i < m; i++)
  {
    for (int j = 0; j < n; j++)
    {
      double x = A[i][j];
      // if (abs(x) < NEAR_ZERO)
      //   x = 0.0;
      std::cout << x << '\t';
    }
    std::cout << '\n';
  }
}

//======================================================================

vec matrixTimesVector(const matrix &A, const vec &V) // Matrix times vector
{
  int n = A.size();
  vec C(n);
  for (int i = 0; i < n; i++)
    C[i] = innerProduct(A[i], V);
  return C;
}

//======================================================================

vec vectorCombination(double a, const vec &U, double b, const vec &V) // Linear combination of vectors
{
  int n = U.size();
  vec W(n);
  for (int j = 0; j < n; j++)
    W[j] = a * U[j] + b * V[j];
  return W;
}

//======================================================================

double innerProduct(const vec &U, const vec &V) // Inner product of U and V
{
  return inner_product(U.begin(), U.end(), V.begin(), 0.0);
}

//======================================================================

double vectorNorm(const vec &V) // Vector norm
{
  return sqrt(innerProduct(V, V));
}

//======================================================================

vec conjugateGradientSolver(const matrix &A, const vec &B)
{
  double TOLERANCE = 1.0e-10;

  int n = A.size();
  vec X(n, 0.0);

  vec R = B;
  vec P = R;
  int k = 0;

  while (k < n)
  {
    vec R_old = R; // Store previous residual
    vec AP = matrixTimesVector(A, P);

    // double alpha = innerProduct(R, R) / std::max(innerProduct(P, AP), NEAR_ZERO);
    double alpha = innerProduct(R, R) / innerProduct(P, AP);
    X = vectorCombination(1.0, X, alpha, P);   // Next estimate of solution
    R = vectorCombination(1.0, R, -alpha, AP); // Residual

    if (vectorNorm(R) < TOLERANCE)
      break; // Convergence test

    // double beta = innerProduct(R, R) / std::max(innerProduct(R_old, R_old), NEAR_ZERO);
    double beta = innerProduct(R, R) / innerProduct(R_old, R_old);
    P = vectorCombination(1.0, R, beta, P); // Next gradient
    k++;
  }

  return X;
}