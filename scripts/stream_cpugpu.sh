#!/bin/sh
# execute stream_multigpu, with various number of visible GPUs

set -e

# 8GB array of 8-byte elements (double)
elems=1073741824
cmd="./stream_multigpu -n $elems -b 128 -c -f"

header='Copy (Max) / GiB/s, Copy (Min) / GiB/s, Copy (Avg) / GiB/s, Scale (Max) / GiB/s, Scale (Min) / GiB/s, Scale (Avg) / GiB/s, Add (Max) / GiB/s, Add (Min) / GiB/s, Add (Avg) / GiB/s, Triad (Max) / GiB/s, Triad (Min) / GiB/s, Triad (Avg) / GiB/s'

echo '--- testing each GCD ---'
echo "gcd_id,$header"
printf "0,"; HIP_VISIBLE_DEVICES=0 $cmd
printf "1,"; HIP_VISIBLE_DEVICES=1 $cmd
printf "2,"; HIP_VISIBLE_DEVICES=2 $cmd
printf "3,"; HIP_VISIBLE_DEVICES=3 $cmd
printf "4,"; HIP_VISIBLE_DEVICES=4 $cmd
printf "5,"; HIP_VISIBLE_DEVICES=5 $cmd
printf "6,"; HIP_VISIBLE_DEVICES=6 $cmd
printf "7,"; HIP_VISIBLE_DEVICES=7 $cmd

echo '--- testing each GPU (GCD pair) ---'
echo "gcds,$header"
printf "0+1,"; HIP_VISIBLE_DEVICES=0,1 $cmd
printf "2+3,"; HIP_VISIBLE_DEVICES=2,3 $cmd
printf "4+5,"; HIP_VISIBLE_DEVICES=4,5 $cmd
printf "6+7,"; HIP_VISIBLE_DEVICES=6,7 $cmd

echo '--- testing optimal placement (spread) ---'
echo "num_gcds,$header"
printf '1,'; HIP_VISIBLE_DEVICES=0               $cmd
printf '2,'; HIP_VISIBLE_DEVICES=0,4             $cmd
printf '4,'; HIP_VISIBLE_DEVICES=0,2,4,6         $cmd
printf '8,'; HIP_VISIBLE_DEVICES=0,1,2,3,4,5,6,7 $cmd 

