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

    # Log file name
    LOG_FILE="${PWD}/logs/cg-multi-nodes-n${PADDED_N}-np${PADDED_NP}.log"

    # Run filename
    RUN_FILE="${PWD}/run/cg-multi-nodes-n${PADDED_N}-np${PADDED_NP}.sh"

    # Number of nodes required for corresponding NP
    N_NODES=$(((NP - 1) / 8 + 1))

    # Identify currently idle node(s) to be used
    IDLE_NODES=$(sinfo-1 -t I -o %n -h)

    # Count of idle node(s)
    IDLE_NODES_CNT=$(echo $IDLE_NODES | grep -o "\n" | wc -l)

    # Populate the nodes
    NODE_LIST=""

    # Added node list count
    NODE_LIST_CNT=0

    # Loop through node names
    for NODE in $(seq -f "node-%02g" 1 8)
    do
      # Check if idle nodes count is sufficient to run the configuration
      if [[ $IDLE_NODES_CNT -lt $N_NODES ]];
      then
        # The currently idle nodes count is insufficient, fallback to sequential node assignment
        if [[ $NODE == "node-01" ]];
        then
          echo "[WARN] Insufficient Idle Node(s). Requested: $N_NODES, Idle: $IDLE_NODES_CNT, Using sequential nodes assignment for $RUN_FILE"
        fi

        # Add NODE to the NODE_LIST
        NODE_LIST="${NODE_LIST},${NODE}"

        # Increment node list count
        NODE_LIST_CNT=$((NODE_LIST_CNT + 1))
      else
        # The currently idle nodes count is sufficient
        if [[ $NODE == "node-01" ]];
        then
          echo "[INFO] Using idle nodes assignment for $RUN_FILE"
        fi

         # Check if the current NODE is IDLE
        if [[ $IDLE_NODES == *"$NODE"* ]];
        then
          # NODE is IDLE, so add it to the NODE_LIST
          NODE_LIST="${NODE_LIST},${NODE}"

          # Increment node list count
          NODE_LIST_CNT=$((NODE_LIST_CNT + 1))
        fi
      fi

      # Check if NODE_LIST_CNT already satisfies N_NODES
      if [[ $NODE_LIST_CNT -eq $N_NODES ]];
      then
        # Break the loop
        break
      fi
    done

    # Trim leading "," from previous loop (if any)
    NODE_LIST=$(echo $NODE_LIST | sed "s|^,||g")

    # Generate RUN_FILE by replacing some placeholders in the template file
    sed "s|__LOG_NAME__|${LOG_FILE}|" ${PWD}/templates/template-cg.sh > $RUN_FILE
    sed -i "s|__NUM_NODES__|${N_NODES}|" $RUN_FILE
    sed -i "s|__NODE_LIST__|${NODE_LIST}|" $RUN_FILE
    sed -i "s|__NUM_PROCESSORS__|${NP}|" $RUN_FILE
    sed -i "s|__TARGET__|${TARGET}|" $RUN_FILE
    sed -i "s|__MATRIX_SIZE__|${N}|g" $RUN_FILE
    sed -i "s|__MAX_ITER__|1000|g" $RUN_FILE
    sed -i "s|__EPS__|1.0e-10|g" $RUN_FILE
    sed -i "s|__TOL__|1.0e-10|g" $RUN_FILE

    # Add execute permission to RUN_FILE
    chmod +x $RUN_FILE

    # Add RUN_FILE to slurm queue
    sbatch $RUN_FILE
  done
done
