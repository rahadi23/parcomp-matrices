#!/bin/bash

mkdir -p ${PWD}/src
mkdir -p ${PWD}/run
mkdir -p ${PWD}/logs

for N in 256 512 1024 2048 4096
do
  SRC_FILE="${PWD}/src/cannon-mm-n${N}.cpp"

  sed "s|__MATRIX_N_SIZE__|${N}|" ${PWD}/templates/template-cannon-mm.cpp > $SRC_FILE

  for NP in 1 4 16
  do
    LOG_FILE="${PWD}/logs/cannon-mm-n${N}-np${NP}.out"
    RUN_FILE="${PWD}/run/cannon-mm-run-n${N}-np${NP}.sh"
    O_FILE="${PWD}/run/cannon-mm-n${N}-np${NP}.o"

    N_NODES=$((NP > 8 ? 8 : NP))
    NODE_LIST="node-01"

    if [ $NP -gt 1 ];
    then
      for NODE in $(seq 2 $N_NODES)
      do
        NODE_LIST="${NODE_LIST},node-0${NODE}"
      done
    fi

    mpic++ $SRC_FILE -o $O_FILE

    sed "s|__LOG_NAME__|${LOG_FILE}|" ${PWD}/templates/template-cannon-mm-run.sh > $RUN_FILE
    sed -i "s|__NUM_NODES__|${N_NODES}|" $RUN_FILE
    sed -i "s|__NODE_LIST__|${NODE_LIST}|" $RUN_FILE
    sed -i "s|__NUM_PROCESSORS__|${NP}|" $RUN_FILE
    sed -i "s|__O_FILE__|${O_FILE}|" $RUN_FILE

    chmod +x $RUN_FILE

    sbatch $RUN_FILE
  done
done
