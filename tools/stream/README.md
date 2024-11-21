# STREAM variant

## Description
This benchmark is a variant of the STREAM benchmark, adapted as a benchmark for 
AMD GPUs, in particular in a multi-GPU system.

Two benchmarks are provided:
- CPU-GPU: with data placed in host memory, STREAM kernels are launched on the 
  visible GPUs.
- GPU-GPU: data is placed on one GPU, and STREAM kernels are launched on another GPU.

## Build
`make`

## Run
### CPU-GPU

This benchmark allocates one host-pinned array per visible GPU in the system.  
Then, all visible GPUs on the system execute in parallel the four STREAM kernels 
(Copy, Scale, Add, Triad). Both source and destination data manipulated by those 
kernels are located on CPU memory. The measurements are reported as average 
values over all GPUs.

```
                                   CPU memory
GPU kernel                           _____                            
 ______            load             |     |
|      |<-------------------------- | src |
| add  |                            |     |
|______|--------------------------> | dst |
                 store result       |_____|
```

By default, the benchmark will see all GPUs available on the system, and execute 
the benchmark on each of them. In the particular case of MI250X, one GCD counts 
as one GPU. The example below will allocate 1 MB (131072 double) of host-pinned 
memory per GPU, and launch the benchmark on each of the GPUs:
```
./stream_cpugpu -n 131072
```

To limit the number of GPUs for which the benchmark is run, the 
`HIP_VISIBLE_DEVICES` environment variable can be used. In the example below, we 
only use GPUs 0 and 4:
```
HIP_VISIBLE_DEVICES=0,4 ./stream_cpugpu -n 131072
```

### GPU-GPU
In the GPU-GPU benchmark, a similar approach is used. Destination and source
arrays are allocated on one GPU, and the STREAM kernels (Copy, Scale, Add,
Triad) are executed on another GPU. 

```
GPU #0                          GPU #1 memory
kernel                              _____
 _____            load             |     |
|     |<-------------------------- | src |
| add |                            |     |
|_____|--------------------------> | dst |
               store result        |_____|
```

Data is allocated on the GPU designated by the environment variable `GCD_ALLOC`
and the kernels are launched on the GPU designetd by the environment variable
`GCD_EXEC`.  In the following example, the data is placed on GPU `1` and the
kernels are launched on GPU `0`
```
GCD_ALLOC=1 GCD_EXEC=0 ./stream_gpugpu -n 131072
```

*Note*: if none of those environment variables are set, data is allocated on the
first visible GPU (`0`), and kernels are launched on the first visible GPU. Such
behavior allows running the benchmark as STREAM-like GPU benchmark.

## License
This benchmark is adapted from 
[cuda-stream](https://github.com/bcumming/cuda-stream).
