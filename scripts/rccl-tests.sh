#!/bin/sh
#SBATCH --nodes 1
#SBATCH --tasks-per-node 1
#SBATCH --gpus-per-task 8

# NOTE: If the scheduler allows granular GPU-to-process mapping, this
# option should  be used instead of HIP_VISIBLE_DEVICES used below.

n=1048576 # 1MB
ops="reduce broadcast all_reduce reduce_scatter all_gather"

echo "# size: $n"

for op in $ops; do
    echo "# --- Operation: $op ---"
    for n_gpu in $(seq 2 8); do
        echo "n_gpu=$n_gpu"
        srun ./${op}_perf -g $n_gpu -b $n -e $n
    done
done
