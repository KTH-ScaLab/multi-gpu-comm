#!/bin/sh
# execute streamp2p: execute on GCD0, allocate on GCDX

NEIGHBORS_GCD='0 1 2 6'

for gcd in $NEIGHBORS_GCD; do
    # 1MB array of elements size 8 bytes
    elems=131072

    echo "GCD_ALLOC=$gcd"
    echo 'size,Copy (Max) / GiB/s, Copy (Min) / GiB/s, Copy (Avg) / GiB/s, Scale (Max) / GiB/s, Scale (Min) / GiB/s, Scale (Avg) / GiB/s, Add (Max) / GiB/s, Add (Min) / GiB/s, Add (Avg) / GiB/s, Triad (Max) / GiB/s, Triad (Min) / GiB/s, Triad (Avg) / GiB/s'

    # 1MB -> 8GB
    for i in $(seq 14); do
        # 8 bytes per element
        printf "$((elems * 8)),"

        GCD_ALLOC=$gcd GCD_EXEC=0 ./stream_gpugpu -c -f -n $elems -b 128
        # HIP_VISIBLE_DEVICES="0,$gcd" ./stream_gpugpu -c -f -n $elems -b 128

        elems=$((elems*2))
    done
done

