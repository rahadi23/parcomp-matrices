#!/bin/bash

# Create required directories
mkdir -p ${PWD}/{out,run,logs}

# Src file name
SRC_FILE="${PWD}/src/matmul-mv.cpp"

# Compiled file name
O_FILE="${PWD}/out/matmul-mv.o"

# Compile SRC_FILE and output it to O_FILE
mpic++ $SRC_FILE -o $O_FILE

# Loop through N matrix dimensions
for N in 256 512 1024 2048 4096
do
  # Loop through NP number of processors
  for NP in 1 2 4 8
  do
    # Create padded N (4 digits) and NP (2 digits) for log and run file name
    # Example:
    #   N  = 256 yields PADDED_N  = 0256
    #   NP = 1   yields PADDED_NP = 01
    printf -v PADDED_N "%04d" $N
    printf -v PADDED_NP "%02d" $NP

    # Log file name
    LOG_FILE="${PWD}/logs/matmul-mv-dev-n${PADDED_N}-np${PADDED_NP}.out"

    # Run O_FILE the corresponding configurations
    # If --use-hwthread-cpus is specified on the mpirun command line,
    # then Open MPI will attempt to discover the number of hardware threads on the node,
    # and use that as the number of slots available. 
    mpirun --use-hwthread-cpus -np $NP $O_FILE $N > $LOG_FILE
  done
done