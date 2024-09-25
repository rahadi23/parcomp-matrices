#!/bin/bash

mkdir -p ${PWD}/src
mkdir -p ${PWD}/run
mkdir -p ${PWD}/logs

for N in 256 512 1024 2048 4096
do
  SRC_FILE="${PWD}/src/matmul-mm-n${N}.c"

  sed "s|__MATRIX_N_SIZE__|${N}|" ${PWD}/templates/template-matmul-mm.c > $SRC_FILE

  for NP in 1 2 4 8 16 32
  do
    LOG_FILE="${PWD}/logs/matmul-mm-n${N}-np${NP}.out"
    O_FILE="${PWD}/run/matmul-mm-n${N}-np${NP}.o"

    mpicc $SRC_FILE -o $O_FILE

    NUM_PROCESSORS=$((NP + 1))

    mpirun --hostfile ${HOME}/hostfile -np $NUM_PROCESSORS $O_FILE > $LOG_FILE
  done
done
