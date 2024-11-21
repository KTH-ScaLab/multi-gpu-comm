#!/bin/sh

set -xe

git clone https://github.com/ROCm/rccl-tests

cd rccl-tests && make -j8
