#!/bin/bash

# Create required directories
mkdir -p ${PWD}/{out,run,logs}

# Src file name
SRC_FILE="${PWD}/src/matmul-cc-mm.cpp"

# Compiled file name
O_FILE="${PWD}/out/matmul-cc-mm.o"

# Compile SRC_FILE and output it to O_FILE
mpic++ $SRC_FILE -o $O_FILE

# Loop through N matrix dimensions
for N in 256 512 1024 2048 4096
do
  # Loop through NP number of processors
  for NP in 1 2 4 8 16 32 64
  do
    # Create padded N (4 digits) and NP (2 digits) for log and run file name
    # Example:
    #   N  = 256 yields PADDED_N  = 0256
    #   NP = 1   yields PADDED_NP = 01
    printf -v PADDED_N "%04d" $N
    printf -v PADDED_NP "%02d" $NP

    # Number of nodes required for corresponding NP
    TASK="matmul-mv-dev-n${PADDED_N}-np${PADDED_NP}"

    # Log file name
    LOG_FILE="${PWD}/logs/${TASK}.out"

    # Host file name
    HOST_FILE="${PWD}/run/hostfile"
    
    # Generate HOST_FILE by copying template-hostfile
    cp ${PWD}/templates/template-hostfile $HOST_FILE

    # Run O_FILE the corresponding configurations
    echo "🏃 ${TASK}..."
    mpirun --hostfile -np $NP $O_FILE $N | tee $LOG_FILE
    echo "✅ ${TASK}"
  done
done
