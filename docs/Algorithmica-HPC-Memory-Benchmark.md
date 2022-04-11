# Algorithmica - HPC - Memory Benchmark

## 1. xxxxx

### 1.1 Algorithmica / HPC

|| [https://en.algorithmica.org/hpc](https://en.algorithmica.org/hpc) || [github.com repository](https://github.com/algorithmica-org/algorithmica) ||

* [RAM & CPU Caches](https://en.algorithmica.org/hpc/cpu-cache/)

  * [Memory Bandwidth](https://en.algorithmica.org/hpc/cpu-cache/bandwidth/)

    * BlockSize = 8, 16, 32, 64, ... 1024, 2048, ... 16KB, 32KB, ... 1MB, 2MB, ... 32MB, 64MB, ... 256MB

    * benchmark function:

      * system function(memcpy, memset)
      * SIMD (MMX, SSE, AVX)
      * SIMD non-temporal

    * benchmark type:

      * read only
      * write only: fill (ZeroMemery, memset)

        ```cpp
        for (int i = 0; i < N; i++)
            a[i] = 0;
        ```

      * copy：dest, src 不会重叠
      * move: dest, src 会重叠
      * sum:

        ```cpp
        for (int i = 0; i < N; i++)
            sum += a[i];
        ```

      * sum: a[i] = a[i] + b[i]
      * scale: a[i] = k * b[i]
      * product: a[i] = a[i] * b[i]
      * triad: a[i] = b[i] + c[i]

    * aligned:

      * aligned
      * not aligned

  * [Memory Latency](https://en.algorithmica.org/hpc/cpu-cache/latency/)

    ```text
    Compared to linear iteration, it is much slower — by multiple orders of magnitude — to visit all elements of an array this way. Not only does it make SIMD impossible, but it also stalls the pipeline, creating a large traffic jam of instructions, all waiting for a single piece of data to be fetched from the memory.
    ```

    ```cpp
    int p[N], q[N];

    // generating a random permutation
    iota(p, p + N, 0);
    random_shuffle(p, p + N);

    // this permutation may contain multiple cycles,
    // so instead we use it to construct another permutation with a single cycle
    int k = p[N - 1];
    for (int i = 0; i < N; i++) {
        k = q[k] = p[i];
    }

    for (int t = 0; t < K; t++) {
        for (int i = 0; i < N; i++) {
            k = q[k];
        }
    }
    ```

  * [Memory Sharing](https://en.algorithmica.org/hpc/cpu-cache/sharing/)

  * [Prefetching](https://en.algorithmica.org/hpc/cpu-cache/prefetching/)

* SIMD Parallelism

  * [Loading and Writing Data](https://en.algorithmica.org/hpc/simd/moving/)
    * Aligned Loads and Stores
    * Extract and Insert
    * Making Constants
    * Broadcast
    * Mapping to Arrays
    * Non-Contiguous Load

        ```text
        Later SIMD extensions added special “gather” and “scatter instructions that read/write data non-sequentially using arbitrary array indices. These don’t work 8 times faster though and are usually limited by the memory rather than the CPU, but they are still helpful for certain applications such as sparse linear algebra.

        Gather is available since AVX2, and various scatter instructions are available since AVX512.
        ```

* [Integer Division](https://en.algorithmica.org/hpc/arithmetic/division/)

    Check out [libdivide](https://github.com/ridiculousfish/libdivide) and [GMP](https://gmplib.org/) for more general implementations of optimized integer division.

    ```cpp
    uint32_t y;

    uint64_t m = uint64_t(-1) / y + 1; // ceil(2^64 / d)

    uint32_t mod(uint32_t x) {
        uint64_t lowbits = m * x;
        return ((__uint128_t) lowbits * y) >> 64; 
    }

    uint32_t div(uint32_t x) {
        return ((__uint128_t) m * x) >> 64;
    }
    ```

* [Converting integers to decimal strings faster with AVX-512](https://lemire.me/blog/2022/03/28/converting-integers-to-decimal-strings-faster-with-avx-512/)

    We rely on the observation that you can compute directly the quotient and the remainder of the division using a series of multiplications and shifts ([Lemire et al. 2019](https://arxiv.org/abs/1902.01961)).
