#!/bin/bash

mkdir -p ${PWD}/{out,run,logs}

SRC_FILE="${PWD}/src/matmul-mm.cpp"
O_FILE="${PWD}/out/matmul-mm.o"

mpic++ $SRC_FILE -o $O_FILE

for N in 256 512 1024 2048 4096
do
  for NP in 1 2 4 8 16 32 64
  do
    LOG_FILE="${PWD}/logs/matmul-mm-single-node-n${N}-np${NP}.out"
    O_FILE="${PWD}/out/matmul-mm-single-node-n${N}-np${NP}.o"
    HOST_FILE="${PWD}/run/hostfile"

    cp ${PWD}/templates/template-hostfile $HOST_FILE

    mpirun --hostfile $HOST_FILE -np $NP $O_FILE $N > $LOG_FILE
  done
done
