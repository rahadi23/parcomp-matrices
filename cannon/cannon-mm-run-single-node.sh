#!/bin/bash

mkdir -p ${PWD}/src
mkdir -p ${PWD}/run
mkdir -p ${PWD}/logs

for N in 256 512 1024 2048 4096
do
  SRC_FILE="${PWD}/src/cannon-mm-n${N}.cpp"

  sed "s|__MATRIX_N_SIZE__|${N}|" ${PWD}/templates/template-cannon-mm.cpp > $SRC_FILE

  for NP in 1 4 16 64
  do
    LOG_FILE="${PWD}/logs/cannon-mm-n${N}-np${NP}.out"
    O_FILE="${PWD}/run/cannon-mm-n${N}-np${NP}.o"

    mpic++ $SRC_FILE -o $O_FILE

    mpirun --hostfile ${HOME}/hostfile -np $NP $O_FILE > $LOG_FILE
  done
done
