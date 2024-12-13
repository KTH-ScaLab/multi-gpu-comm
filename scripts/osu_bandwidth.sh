#!/bin/sh
#SBATCH --ntasks-per-node 2
#SBATCH --nodes 1

# NOTE: If the scheduler allows granular GPU-to-process mapping, this 
# option should  be used instead of HIP_VISIBLE_DEVICES used below.

n=$((1048576 * 1024)) # 1GB
cmd="./osu_bw -m$n:$n -d rocm D D"
filter_cmd="tail -n1 | awk -F' ' '{print \$NF}'"

export MPICH_GPU_SUPPORT_ENABLED=1

echo "# size: $n"
echo "# format: gcd_dst,bw_mbs"

echo "# --- HSA_ENABLE_SDMA=1 ---"
for gcd in $(seq 1 7); do
    echo -n "$gcd,"
    HSA_ENABLE_SDMA=1 HIP_VISIBLE_DEVICES=0,$gcd \
        srun $cmd | $filter_cmd
done

echo "# --- HSA_ENABLE_SDMA=0 ---"
for gcd in $(seq 1 7); do
    echo -n "$gcd,"
    HSA_ENABLE_SDMA=0 HIP_VISIBLE_DEVICES=0,$gcd \
        srun $cmd | $filter_cmd
done

