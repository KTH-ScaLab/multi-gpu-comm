#include "hip/hip_runtime.h"
/*
   STREAM benchmark implementation in CUDA.

COPY:       a(i) = b(i)
SCALE:      a(i) = q*b(i)
SUM:        a(i) = b(i) + c(i)
TRIAD:      a(i) = b(i) + q*c(i)

It measures the memory system on the device.
The implementation is in double precision.

Code based on the code developed by John D. McCalpin
http://www.cs.virginia.edu/stream/FTP/Code/stream.c

Written by: Massimiliano Fatica, NVIDIA Corporation

Further modifications by: Ben Cumming, CSCS; Andreas Herten (JSC/FZJ); Sebastian Achilles (JSC/FZJ)

Further modifications by: Gabin Schieffer (KTH)
*/

#ifdef NTIMES
#if NTIMES<=1
#   define NTIMES  20
#endif
#endif
#ifndef NTIMES
#   define NTIMES  20
#endif

#include <string>
#include <vector>

#include <stdio.h>
#include <float.h>
#include <limits.h>
// #include <unistd.h>
#include <getopt.h>

#include <chrono>

# ifndef MIN
# define MIN(x,y) ((x)<(y)?(x):(y))
# endif
# ifndef MAX
# define MAX(x,y) ((x)>(y)?(x):(y))
# endif

typedef double real;

static double   avgtime[4] = {0}, maxtime[4] = {0},
                mintime[4] = {FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX};


void print_help()
{
  printf(
      "Usage: stream [-s] [-c [-f]] [-n <elements>] [-b <blocksize>]\n\n"
      "  -s, --si\n"
      "        Print results in SI units (by default IEC units are used)\n\n"
      "  -c, --csv\n"
      "        Print results CSV formatted\n\n"
      "  -f, --full\n"
      "        Print all results in CSV\n\n"
      "  -t, --title\n"
      "        Print CSV header\n\n"
      "  -n <elements>, --nelements <element>\n"
      "        Put <elements> values in the arrays\n"
      "        (default: 1<<26)\n\n"
      "  -b <blocksize>, --blocksize <blocksize>\n"
      "        Use <blocksize> as the number of threads in each block\n"
      "        (default: 192)\n"
      );
}

void parse_options(int argc, char** argv, bool& SI, bool& CSV, bool& CSV_full, bool& CSV_header, int& N, int& blockSize)
{
  // Default values
  SI = false;
  CSV = false;
  CSV_full = false;
  CSV_header = false;
  N = 1<<26;
  blockSize = 192;

  static struct option long_options[] = {
    {"si",        no_argument,       0,  's' },
    {"csv",       no_argument,       0,  'c' },
    {"full",      no_argument,       0,  'f' },
    {"title",     no_argument,       0,  't' },
    {"nelements", required_argument, 0,  'n' },
    {"blocksize", required_argument, 0,  'b' },
    {"help",      no_argument,       0,  'h' },
    {0,           0,                 0,  0   }
  };
  int c;
  int option_index = 0;
  while ((c = getopt_long(argc, argv, "scftn:b:h", long_options, &option_index)) != -1)
    switch (c)
    {
      case 's':
        SI = true;
        break;
      case 'c':
        CSV = true;
        break;
      case 'f':
        CSV_full = true;
        break;
      case 't':
        CSV_header = true;
        break;
      case 'n':
        N = std::atoi(optarg);
        break;
      case 'b':
        blockSize = std::atoi(optarg);
        break;
      case 'h':
        print_help();
        std::exit(0);
        break;
      default:
        print_help();
        std::exit(1);
    }
}

  template <typename T>
__global__ void set_array(T * __restrict__ const a, T value, int len)
{
  int idx = threadIdx.x + blockIdx.x * blockDim.x;
  if (idx < len)
    a[idx] = value;
}

  template <typename T>
__global__ void STREAM_Copy(T const * __restrict__ const a, T * __restrict__ const b, int len)
{
  int idx = threadIdx.x + blockIdx.x * blockDim.x;
  if (idx < len)
    b[idx] = a[idx];
}

  template <typename T>
__global__ void STREAM_Scale(T const * __restrict__ const a, T * __restrict__ const b, T scale,  int len)
{
  int idx = threadIdx.x + blockIdx.x * blockDim.x;
  if (idx < len)
    b[idx] = scale * a[idx];
}

  template <typename T>
__global__ void STREAM_Add(T const * __restrict__ const a, T const * __restrict__ const b, T * __restrict__ const c, int len)
{
  int idx = threadIdx.x + blockIdx.x * blockDim.x;
  if (idx < len)
    c[idx] = a[idx] + b[idx];
}

  template <typename T>
__global__ void STREAM_Triad(T const * __restrict__ a, T const * __restrict__ b, T * __restrict__ const c, T scalar, int len)
{
  int idx = threadIdx.x + blockIdx.x * blockDim.x;
  if (idx < len)
    c[idx] = a[idx] + scalar * b[idx];
}

int main(int argc, char** argv)
{
  int j,k;
  double times[4][NTIMES];
  real scalar;
  std::chrono::steady_clock::time_point start_time, end_time;
  std::vector<std::string> label{"Copy:      ", "Scale:     ", "Add:       ", "Triad:     "};

  // Parse arguments
  bool SI, CSV, CSV_full, CSV_header;
  int N, blockSize;
  parse_options(argc, argv, SI, CSV, CSV_full, CSV_header, N, blockSize);
  
  int num_gpus;
  hipGetDeviceCount(&num_gpus);

  if (!CSV) {
    printf("STREAM Benchmark implementation in CUDA\n");
    printf("Array size (%s precision) = %7.2f MB\n", sizeof(double)==sizeof(real)?"double":"single", double(N)*double(sizeof(real))/1.e6);
    printf("%d GPUs detected.\n", num_gpus);
  }

  real **d_a, **d_b, **d_c;
  d_a = new real*[num_gpus];
  d_b = new real*[num_gpus];
  d_c = new real*[num_gpus];
  
  /* Compute execution configuration */
  dim3 dimBlock(blockSize);
  dim3 dimGrid(N/dimBlock.x );
  if( N % dimBlock.x != 0 ) dimGrid.x+=1;

  if (!CSV) {
    printf("Using %d threads per block, %d blocks\n",dimBlock.x,dimGrid.x);

    if (SI)
      printf("Output in SI units (KB = 1000 B)\n");
    else
      printf("Output in IEC units (KiB = 1024 B)\n");
  }

  for (int i = 0; i < num_gpus; i++) {
    /* Create allocations, three for each GPU */
    hipSetDevice(i);

    /* Allocate memory on device */
    hipHostMalloc((void**)&d_a[i], sizeof(real)*N);
    hipHostMalloc((void**)&d_b[i], sizeof(real)*N);
    hipHostMalloc((void**)&d_c[i], sizeof(real)*N);

    /* Initialize memory on the device */
    hipLaunchKernelGGL(HIP_KERNEL_NAME(set_array<real>), dimGrid, dimBlock, 0, 0, d_a[i], 2.f, N);
    hipLaunchKernelGGL(HIP_KERNEL_NAME(set_array<real>), dimGrid, dimBlock, 0, 0, d_b[i], .5f, N);
    hipLaunchKernelGGL(HIP_KERNEL_NAME(set_array<real>), dimGrid, dimBlock, 0, 0, d_c[i], .5f, N);
  }
  /*  --- MAIN LOOP --- repeat test cases NTIMES times --- */

  scalar=3.0f;
  for (k=0; k<NTIMES; k++)
  {
    start_time = std::chrono::steady_clock::now();
    for(int i = 0; i < num_gpus; i++) {
      hipSetDevice(i);
      hipLaunchKernelGGL(HIP_KERNEL_NAME(STREAM_Copy<real>), dimGrid, dimBlock, 0, 0,
                         d_a[i], d_c[i], N);
    }
    for(int i = 0; i < num_gpus; i++) {
      hipSetDevice(i);
      hipDeviceSynchronize();
    }
    end_time = std::chrono::steady_clock::now();
    times[0][k] = std::chrono::duration_cast<std::chrono::duration<double>>(end_time - start_time).count();

    start_time = std::chrono::steady_clock::now();
    for(int i = 0; i < num_gpus; i++) {
      hipSetDevice(i);
      hipLaunchKernelGGL(HIP_KERNEL_NAME(STREAM_Scale<real>), dimGrid, dimBlock, 0, 0,
                         d_b[i], d_c[i], scalar, N);
    }
    for(int i = 0; i < num_gpus; i++) {
      hipSetDevice(i);
      hipDeviceSynchronize();
    }
    end_time = std::chrono::steady_clock::now();
    times[1][k] = std::chrono::duration_cast<std::chrono::duration<double>>(end_time - start_time).count();

    start_time = std::chrono::steady_clock::now();
    for(int i = 0; i < num_gpus; i++) {
      hipSetDevice(i);
      hipLaunchKernelGGL(HIP_KERNEL_NAME(STREAM_Add<real>), dimGrid, dimBlock, 0, 0,
                         d_a[i], d_b[i], d_c[i], N);
    }
    for(int i = 0; i < num_gpus; i++) {
      hipSetDevice(i);
      hipDeviceSynchronize();
    }
    end_time = std::chrono::steady_clock::now();
    times[2][k] = std::chrono::duration_cast<std::chrono::duration<double>>(end_time - start_time).count();

    start_time = std::chrono::steady_clock::now();
    for(int i = 0; i < num_gpus; i++) {
      hipSetDevice(i);
      hipLaunchKernelGGL(HIP_KERNEL_NAME(STREAM_Triad<real>), dimGrid, dimBlock, 0, 0,
                         d_b[i], d_c[i], d_a[i], scalar, N);
    }
    for(int i = 0; i < num_gpus; i++) {
      hipSetDevice(i);
      hipDeviceSynchronize();
    }
    end_time = std::chrono::steady_clock::now();
    times[3][k] = std::chrono::duration_cast<std::chrono::duration<double>>(end_time - start_time).count();
  }

  /*  --- SUMMARY --- */

  hipError_t err = hipPeekAtLastError();
  if(err) {
    printf("HIP error occured: %s\n", hipGetErrorString(err));
    exit(1);
  }

  for (k=1; k<NTIMES; k++) /* note -- skip first iteration */
  {
    for (j=0; j<4; j++)
    {
      avgtime[j] = avgtime[j] + times[j][k];
      mintime[j] = MIN(mintime[j], times[j][k]);
      maxtime[j] = MAX(maxtime[j], times[j][k]);
    }
  }
  for (j=0; j<4; j++)
    avgtime[j] = avgtime[j]/(double)(NTIMES-1);

  double bytes[4] = {
    2 * sizeof(real) * (double)N * num_gpus,
    2 * sizeof(real) * (double)N * num_gpus,
    3 * sizeof(real) * (double)N * num_gpus,
    3 * sizeof(real) * (double)N * num_gpus
  };

  // Use right units
  const double G = SI ? 1.e9 : static_cast<double>(1<<30);
  std::string gbpersec = SI ? "GB/s" : "GiB/s";

  if (!CSV) {
    printf("\nFunction      Rate %s  Avg time(s)  Min time(s)  Max time(s)\n", gbpersec.c_str() );
    printf("-----------------------------------------------------------------\n");
    for (j=0; j<4; j++) {
      printf("%s%11.4f     %11.8f  %11.8f  %11.8f\n", label[j].c_str(),
          bytes[j]/mintime[j] / G,
          avgtime[j],
          mintime[j],
          maxtime[j]);
    }
  } else {
    if (CSV_full) {
      if (CSV_header)
        printf("Copy (Max) / %s, Copy (Min) / %s, Copy (Avg) / %s, Scale (Max) / %s, Scale (Min) / %s, Scale (Avg) / %s, Add (Max) / %s, Add (Min) / %s, Add (Avg) / %s, Triad (Max) / %s, Triad (Min) / %s, Triad (Avg) / %s\n",
            gbpersec.c_str(), gbpersec.c_str(), gbpersec.c_str(),
            gbpersec.c_str(), gbpersec.c_str(), gbpersec.c_str(),
            gbpersec.c_str(), gbpersec.c_str(), gbpersec.c_str(),
            gbpersec.c_str(), gbpersec.c_str(), gbpersec.c_str()
            );
      printf(
          "%0.4f,%0.4f,%0.4f,%0.4f,%0.4f,%0.4f,%0.4f,%0.4f,%0.4f,%0.4f,%0.4f,%0.4f\n",
          bytes[0]/mintime[0] / G, bytes[0]/maxtime[0] / G, bytes[0]/(avgtime[0])/ G,
          bytes[1]/mintime[1] / G, bytes[1]/maxtime[1] / G, bytes[1]/(avgtime[1]) / G,
          bytes[2]/mintime[2] / G, bytes[2]/maxtime[2] / G, bytes[2]/(avgtime[2]) / G,
          bytes[3]/mintime[3] / G, bytes[3]/maxtime[3] / G, bytes[3]/(avgtime[3]) / G
          );
    }
    else {
      if (CSV_header)
        printf("Copy (Max) / %s, Scale (Max) / %s, Add (Max) / %s, Triad (Max) / %s\n", gbpersec.c_str(), gbpersec.c_str(), gbpersec.c_str(), gbpersec.c_str());
      printf(
          "%0.4f,%0.4f,%0.4f,%0.4f\n",
          bytes[0]/mintime[0] / G,
          bytes[1]/mintime[1] / G,
          bytes[2]/mintime[2] / G,
          bytes[3]/mintime[3] / G
          );
    }
  }

  /* Free memory on host */
  for (int i = 0; i < num_gpus; i++) {
    hipFree(d_a[i]);
    hipFree(d_b[i]);
    hipFree(d_c[i]);
  }

  delete[] d_a;
  delete[] d_b;
  delete[] d_c;
}
