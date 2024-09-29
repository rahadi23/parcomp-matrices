#!/bin/bash

mkdir -p ${PWD}/{out,run,logs}

SRC_FILE="${PWD}/src/cannon-mm.cpp"
O_FILE="${PWD}/out/cannon-mm.o"

mpic++ $SRC_FILE -o $O_FILE

for N in 256 512 1024 2048 4096
do
  for NP in 1 4 16 64
  do
    LOG_FILE="${PWD}/logs/cannon-mm-multi-nodes-n${N}-np${NP}.out"
    RUN_FILE="${PWD}/run/cannon-mm-multi-nodes-n${N}-np${NP}.sh"

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

    sed "s|__LOG_NAME__|${LOG_FILE}|" ${PWD}/templates/template-cannon.sh > $RUN_FILE
    sed -i "s|__NUM_NODES__|${N_NODES}|" $RUN_FILE
    sed -i "s|__NODE_LIST__|${NODE_LIST}|" $RUN_FILE
    sed -i "s|__NUM_PROCESSORS__|${NP}|" $RUN_FILE
    sed -i "s|__O_FILE__|${O_FILE}|" $RUN_FILE

    chmod +x $RUN_FILE

    sbatch $RUN_FILE
  done
done
