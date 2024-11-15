#ifndef CG_SEQUENTIAL_H_
#define CG_SEQUENTIAL_H_

#define A(row, col, N) (A[(row) * N + (col)])
#define b(x) (b[(x)])

float *generate_A(int N);
float *generate_b(int N);
void print_mat(float *A, int rows, int cols);
void print_vec(float *b, int N);

int more_or_less_equal(float *a, float *b, int N, float TOL);
void scalar_mat_vec(float alpha, float *A, float *b, float *out, int rows, int cols);
void vec_plus_vec(float *vec1, float *vec2, float *out, int N);
void mat_vec(float *A, float *b, float *out, int rows, int cols);
void scalar_vec(float alpha, float *vec2, float *out, int N);
float vec_vec(float *vec1, float *vec2, int N);

void solve_cg_seq(float *A, float *b, float *x, int N, int max_iter, float eps, int *metrics_iter, float *metrics_r_norm);

#endif /* CG_SEQUENTIAL_H_ */
