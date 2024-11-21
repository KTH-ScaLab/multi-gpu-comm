#!/bin/bash

function run_test {
    code=$1
    test_name=$2

    ./comm_scope \
        --benchmark_filter="$test_name/0/0/log2(N)*" \
        --benchmark_out=$code.csv \
        --benchmark_out_format=csv
}

### Pageable ###
run_test pageable_h2d Comm_hipMemcpyAsync_PageableToGPU
run_test pageable_d2h Comm_hipMemcpyAsync_GPUToPageable

### Pinned ###
run_test pinned_h2d Comm_hipMemcpyAsync_PinnedToGPU 
run_test pinned_d2h Comm_hipMemcpyAsync_GPUToPinned

### Managed (zero copy) ###
export HSA_XNACK=0
run_test zerocopy_h2d Comm_implicit_managed_HostWrGPU_fine 
run_test zerocopy_d2h Comm_implicit_managed_GPURdHost_fine 

### Managed (page migration) ###
export HSA_XNACK=1
run_test pagemigration_h2d Comm_implicit_managed_GPURdHost_fine
run_test pagemigration_d2h Comm_implicit_managed_HostWrGPU_fine 