#!/bin/bash

mkdir -p ${PWD}/{out,run,logs}

SRC_FILE="${PWD}/src/matmul-mv.cpp"
O_FILE="${PWD}/out/matmul-mv.o"

mpic++ $SRC_FILE -o $O_FILE

for N in 256 512 1024 2048 4096
do
  for NP in 1 2 4 8 16 32 64
  do
    LOG_FILE="${PWD}/logs/matmul-mv-multi-nodes-n${N}-np${NP}.out"
    RUN_FILE="${PWD}/run/matmul-mv-multi-nodes-n${N}-np${NP}.sh"

    N_NODES=$(((NP - 1) / 8 + 1))
    NODE_LIST=""

    for NODE in $(seq 1 $N_NODES)
    do
      if [ $NODE -eq 1 ];
      then
        NODE_LIST="node-0${NODE}"
      else
        NODE_LIST="${NODE_LIST},node-0${NODE}"
      fi
    done

    sed "s|__LOG_NAME__|${LOG_FILE}|" ${PWD}/templates/template-matmul.sh > $RUN_FILE
    sed -i "s|__NUM_NODES__|${N_NODES}|" $RUN_FILE
    sed -i "s|__NODE_LIST__|${NODE_LIST}|" $RUN_FILE
    sed -i "s|__NUM_PROCESSORS__|${NP}|" $RUN_FILE
    sed -i "s|__O_FILE__|${O_FILE}|" $RUN_FILE
    sed -i "s|__MATRIX_N__|${N}|" $RUN_FILE

    chmod +x $RUN_FILE

    sbatch $RUN_FILE
  done
done
