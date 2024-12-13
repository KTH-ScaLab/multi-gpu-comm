#!/bin/sh
#SBATCH --ntasks-per-node 8
#SBATCH --nodes 1

set -ex

# NOTE: If the scheduler allows granular GPU-to-process mapping, this 
# option should  be used instead of HIP_VISIBLE_DEVICES used below.

# MPI collectives
ops="reduce bcast allreduce reduce_scatter allgather"
n=1048576 # 1MB

echo "# size: $n"

function run_exp() {
    for op in $ops; do
        cmd="./osu_$op -m$n:$n -d rocm D D"

        HIP_VISIBLE_DEVICE=0,1             srun -n2 $cmd
        HIP_VISIBLE_DEVICE=0,1,6           srun -n3 $cmd
        HIP_VISIBLE_DEVICE=0,1,6,7         srun -n4 $cmd
        HIP_VISIBLE_DEVICE=0,1,6,7,2       srun -n5 $cmd
        HIP_VISIBLE_DEVICE=0,1,6,7,2,3     srun -n6 $cmd
        HIP_VISIBLE_DEVICE=0,1,6,7,2,3,4   srun -n7 $cmd
        HIP_VISIBLE_DEVICE=0,1,6,7,3,2,4,6 srun -n8 $cmd
    done
}

export MPICH_GPU_SUPPORT_ENABLED=1

echo "# --- HSA_ENABLE_SDMA=1 ---"
export HSA_ENABLE_SDMA=1
run_exp

echo "# --- HSA_ENABLE_SDMA=0 ---"
export HSA_ENABLE_SDMA=0
run_exp

