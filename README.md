# MPI Based Matrix Operation Algorithms for Parallel Computing Experiments

These algoritms are primarily used to measure performance, i.e. running time, of various parallel computing environments (multi-cores, multi-nodes, etc.). The `N` matrix dimensions can be defined, while ommiting the content of the matrices themselves.

## Environment Assumptions

These algorithms are developed with the following assumptions:

### Multi-nodes environment

- Using (up to) 8 nodes slurm environment with hostname: `node-01`, `node-02`, ..., `node-08`
- Round-robin distributed in core-first manner instead of node-first

## Algorithm List

### 📂 matmul

[Iterative matrix multiplication algorithm](https://en.wikipedia.org/wiki/Matrix_multiplication_algorithm#Iterative_algorithm) using peer-to-peer methods (`MPI_Send`, `MPI_Recv`)

```
Input: matrices A and B
Let C be a new matrix of the appropriate size
For i from 1 to n:
    For j from 1 to p:
        Let sum = 0
        For k from 1 to m:
            Set sum ← sum + Aik × Bkj
        Set Cij ← sum
Return C
```

### 📂 matmul-cc

[Iterative matrix multiplication algorithm](https://en.wikipedia.org/wiki/Matrix_multiplication_algorithm#Iterative_algorithm) using Collective Communication methods (`MPI_Scatter`, `MPI_Gather`)

```
Input: matrices A and B
Let C be a new matrix of the appropriate size
For i from 1 to n:
    For j from 1 to p:
        Let sum = 0
        For k from 1 to m:
            Set sum ← sum + Aik × Bkj
        Set Cij ← sum
Return C
```

### 📂 conjugate-gradient

Matrix x vector equation solver with [Conjugate gradient method](https://en.wikipedia.org/wiki/Conjugate_gradient_method).

#### Definition

Suppose we want to solve the system of linear equations

$$
    {\displaystyle \mathbf {A} \mathbf {x} =\mathbf {b} }
$$

for the vector ${\displaystyle \mathbf {x} }$, where the known ${\displaystyle n\times n}$ matrix ${\displaystyle \mathbf {A} }$ is symmetric (i.e., ${\displaystyle \mathbf {A}^\mathbf{T} =\mathbf{A}}$), positive-definite (i.e. ${\displaystyle \mathbf{x}^\mathbf{T}\mathbf{A}\mathbf{x} > 0}$ for all non-zero vectors ${\displaystyle \mathbf {x} }$ in ${\displaystyle \mathbf{R}^\mathbf{n}}$), and real, and ${\displaystyle \mathbf {b} }$ is known as well. We denote the unique solution of this system by ${\displaystyle \mathbf {x} _{\\*}}$.

Modified to work on our environments from [akjain90/MPI-parallelized-conjugate-gradient-solver](https://github.com/akjain90/MPI-parallelized-conjugate-gradient-solver)

### 📂 cannon

[Cannon's distributed algorithm for matrix multiplication](https://en.wikipedia.org/wiki/Cannon%27s_algorithm)

> When multiplying two n×n matrices A and B, we need n×n processing nodes p arranged in a 2D grid.

```
// PE(i , j)
k := (i + j) mod N;
a := a[i][k];
b := b[k][j];
c[i][j] := 0;
for (l := 0; l < N; l++) {
    c[i][j] := c[i][j] + a * b;
        concurrently {
            send a to PE(i, (j + N − 1) mod N);
            send b to PE((i + N − 1) mod N, j);
        } with {
            receive a' from PE(i, (j + 1) mod N);
            receive b' from PE((i + 1) mod N, j );
        }
    a := a';
    b := b';
}
```

Modified to work on our environments from [andadiana/cannon-algorithm-mpi](https://github.com/andadiana/cannon-algorithm-mpi)
