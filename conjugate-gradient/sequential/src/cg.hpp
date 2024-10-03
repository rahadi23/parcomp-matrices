#include <string>
#include <vector>

using vec = std::vector<double>; // vector
using matrix = std::vector<vec>; // matrix (=collection of (row) vectors)

vec buildVec(int N);
matrix buildMatrix(int N);
void print(std::string title, const vec &V);
void print(std::string title, const matrix &A);
vec matrixTimesVector(const matrix &A, const vec &V);
vec vectorCombination(double a, const vec &U, double b, const vec &V);
double innerProduct(const vec &U, const vec &V);
double vectorNorm(const vec &V);
vec conjugateGradientSolver(const matrix &A, const vec &B);