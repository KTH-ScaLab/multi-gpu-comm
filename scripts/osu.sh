#!/bin/sh
#SBATCH --ntasks-per-node 2
#SBATCH --nodes 1

# NOTE: If the scheduler allows granular GPU-to-process mapping, this 
# option should  be used instead of HIP_VISIBLE_DEVICES used below.

HSA_ENABLE_SDMA=0 MPICH_GPU_SUPPORT_ENABLED=1 HIP_VISIBLE_DEVICES=0,1 \
    srun ./osu_bw -m:1048576 -d rocm D D