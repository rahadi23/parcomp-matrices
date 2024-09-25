# MPI Based Matrix Operation Algorithms for Parallel Computing Experiments

These algoritms are primarily used to measure performance, i.e. running time, of various parallel computing environments (multi-cores, multi-nodes, etc.). The `N` matrix dimensions can be defined, while ommiting the content of the matrices themselves.

## Environment Assumptions

These algorithms are developed with the following assumptions:

### Multi-nodes environment

- Using (up to) 8 nodes slurm environment with hostname: `node-01`, `node-02`, ..., `node-08`
- Round-robin distributed in node-first manner instead of core-first

## Algorithm List

### 📂 matmul

[Iterative matrix multiplication algorithm](https://en.wikipedia.org/wiki/Matrix_multiplication_algorithm#Iterative_algorithm)

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

Modified to works on our environments from [andadiana/cannon-algorithm-mpi](https://github.com/andadiana/cannon-algorithm-mpi)
