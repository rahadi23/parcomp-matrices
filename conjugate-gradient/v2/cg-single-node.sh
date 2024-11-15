#!/bin/bash

# Exit on any error
set -e

# Create required directories
mkdir -p ${PWD}/{run,logs}

# Target
TARGET=${PWD}/out/cg.o

# Make
make clean
make

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

    # Task name
    TASK="cg-single-node-n${PADDED_N}-np${PADDED_NP}"

    # Log file name
    LOG_FILE="${PWD}/logs/${TASK}.log"
    
    # Host file name
    HOST_FILE="${PWD}/run/hostfile"

    # Generate HOST_FILE by copying template-hostfile
    cp ${PWD}/templates/template-hostfile $HOST_FILE

    # Run O_FILE the corresponding configurations
    echo "🏃 ${TASK}..."
    mpirun --hostfile $HOST_FILE -np $NP $TARGET $N 1000 1.0e-10 1.0e-10 | tee $LOG_FILE
    echo "✅ ${TASK}"
  done
done
