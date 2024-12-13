#!/bin/sh

set -e

curl https://mvapich.cse.ohio-state.edu/download/mvapich/osu-micro-benchmarks-7.5-1.tar.gz | tar xzf -
cd osu-micro-benchmarks-7.5-1

# load modules (GNU + Cray-MPICH)
ml PrgEnv-gnu rocm/5.7.0 craype-accel-amd-gfx90a cray-mpich
./configure CC=cc CXX=CC --enable-rocm --with-rocm=/opt/rocm-5.7.0

make -j16