#!/bin/sh

set -ex

git clone --recursive https://github.com/c3sr/comm_scope 
cd comm_scope

# Checkout a working libscope version
git -C thirdparty/libscope checkout 124999dc0017b437adcbebeaded52cf9d973ac28

mkdir build && cd build
cmake \
    -D CMAKE_CXX_COMPILER=hipcc \
    -D SCOPE_USE_HIP=ON \
    -D SCOPE_ARCH_MI250X=ON \
    ..

make -j8
echo "successfully built $PWD/comm_scope"
