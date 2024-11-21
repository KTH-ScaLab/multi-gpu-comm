# RCCL Collective Tests

## Description

## Build

*Requirement*: ROCm set up on the system.

**We provide a basic build script, `build.sh`, reproducing the steps described below.**

1. Clone the RCCL-Tests repository
```
git clone https://github.com/ROCm/rccl-tests
```

2. Build
```
cd rccl-test && make -j8
```

## Run

The binaries are written in `rccl-tests/build`, one binary is produced for each test.
```
build/./reduce_perf
```

Run the AllReduce test, with two GPUs, for an array of 1048576 bytes.
```
build/./all_reduce_perf -g 2 -b 1048576 -e 1048576
```

## License
RCCL tests are provided under the BSD license. Available at [RCCL-Test](https://github.com/ROCm/rccl-tests).