# Comm|Scope

## Build

*Requirement*: `hipcc` available in the path.

1. Clone the Comm|Scope repository
```
git clone --recursive https://github.com/c3sr/comm_scope
```

2. We need to revert commit `8a24547` in libscope, as it causes compilation error (with ROCm 5.7.0)
```
cd comm_scope/thirdparty/libscope/
git checkout 124999dc0017b437adcbebeaded52cf9d973ac28
cd ../../..
```

3. Run CMake
```
mkdir build && cd build
cmake \
    -D CMAKE_CXX_COMPILER=hipcc \
    -D SCOPE_USE_HIP=ON \
    -D SCOPE_ARCH_MI250X=ON \
    ..
```

4. Make
```
make -j8
```

**We provide a basic build script, `build.sh`, reproducing the steps described below.**

## Run

## License
Comm|Scope is released under Apache License 2.0, available at [comm_scope](https://github.com/c3sr/comm_scope/).