# Exit on any error
set -e

# Create required directories
mkdir -p ${PWD}/{run,logs}

# Target
TARGET=cg

# Make
make clean
make

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

    # Task name
    TASK="cg-dev-n${PADDED_N}-np${PADDED_NP}"

    # Log file name
    LOG_FILE="${PWD}/logs/${TASK}.log"
    
    # Run O_FILE the corresponding configurations
    echo "🏃 ${TASK}..."
    mpirun --use-hwthread-cpus $HOST_FILE -np $NP $TARGET.o $N $N 1000 -1 | tee $LOG_FILE
    echo "✅ ${TASK}"
  done
done
