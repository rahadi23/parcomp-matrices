#!/bin/bash
#SBATCH -p batch
#SBATCH -o __LOG_NAME__
#SBATCH -N __NUM_NODES__
#SBATCH --nodelist=__NODE_LIST__

mpirun --mca btl_tcp_if_exclude docker0,lo -np __NUM_PROCESSORS__ __O_FILE__ __MATRIX_N__
