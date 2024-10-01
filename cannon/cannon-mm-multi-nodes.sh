#!/bin/bash

mkdir -p ${PWD}/{out,run,logs}

SRC_FILE="${PWD}/src/cannon-mm.cpp"
O_FILE="${PWD}/out/cannon-mm.o"

mpic++ $SRC_FILE -o $O_FILE

for N in 256 512 1024 2048 4096
do
  for NP in 1 4 16 64
  do
    printf -v PADDED_N "%04d" $N
    printf -v PADDED_NP "%02d" $NP

    LOG_FILE="${PWD}/logs/cannon-mm-multi-nodes-n${PADDED_N}-np${PADDED_NP}.out"
    RUN_FILE="${PWD}/run/cannon-mm-multi-nodes-n${PADDED_N}-np${PADDED_NP}.sh"

    N_NODES=$(((NP - 1) / 8 + 1))
    NODE_LIST=""

    for NODE in $(seq -f "node-%02g" 1 8)
    do
      if [ $NODE_LIST == "" ];
      then
        NODE_LIST=$NODE
      else
        NODE_LIST="${NODE_LIST},${NODE}"
      fi
    done

    sed "s|__LOG_NAME__|${LOG_FILE}|" ${PWD}/templates/template-cannon.sh > $RUN_FILE
    sed -i "s|__NUM_NODES__|${N_NODES}|" $RUN_FILE
    sed -i "s|__NODE_LIST__|${NODE_LIST}|" $RUN_FILE
    sed -i "s|__NUM_PROCESSORS__|${NP}|" $RUN_FILE
    sed -i "s|__O_FILE__|${O_FILE}|" $RUN_FILE
    sed -i "s|__MATRIX_N__|${N}|" $RUN_FILE

    chmod +x $RUN_FILE

    sbatch $RUN_FILE
  done
done
