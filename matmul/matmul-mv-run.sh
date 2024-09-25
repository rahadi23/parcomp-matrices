#!/bin/bash

mkdir -p ${PWD}/src
mkdir -p ${PWD}/run
mkdir -p ${PWD}/logs

for N in 256 512 1024 2048 4096
do
  SRC_FILE="${PWD}/src/matmul-mv-n${N}.c"

  sed "s|__MATRIX_N_SIZE__|${N}|" ${PWD}/templates/template-matmul-mv.c > $SRC_FILE

  for NP in 1 2 4 8 16 32
  do
    LOG_FILE="${PWD}/logs/matmul-mv-n${N}-np${NP}.out"
    RUN_FILE="${PWD}/run/matmul-mv-run-n${N}-np${NP}.sh"
    O_FILE="${PWD}/run/matmul-mv-n${N}-np${NP}.o"

    N_NODES=$((NP > 8 ? 8 : NP))
    NODE_LIST="node-01"

    if [ $NP -gt 1 ];
    then
      for NODE in $(seq 2 $N_NODES)
      do
        NODE_LIST="${NODE_LIST},node-0${NODE}"
      done
    fi

    mpicc $SRC_FILE -o $O_FILE

    NUM_PROCESSORS=$((NP+1))

    sed "s|__LOG_NAME__|${LOG_FILE}|" ${PWD}/templates/template-matmul-run.sh > $RUN_FILE
    sed -i "s|__NUM_NODES__|${N_NODES}|" $RUN_FILE
    sed -i "s|__NODE_LIST__|${NODE_LIST}|" $RUN_FILE
    sed -i "s|__NUM_PROCESSORS__|${NUM_PROCESSORS}|" $RUN_FILE
    sed -i "s|__O_FILE__|${O_FILE}|" $RUN_FILE

    chmod +x $RUN_FILE

    sbatch $RUN_FILE
  done
done
