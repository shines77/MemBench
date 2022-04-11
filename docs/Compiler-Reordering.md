# Compiler Assembly Reordering

## 1. load/store order

### 1.1 [StackOverFlow]: [Wrong gcc generated assembly ordering, results in performance hit](https://stackoverflow.com/questions/25778302/wrong-gcc-generated-assembly-ordering-results-in-performance-hit)

[中文版]: [错误的 gcc 生成的装配顺序, 导致性能损失](https://qa.1r1g.com/sf/ask/1804481171/#answer-2460585011)

I have got the following code, which copies data from memory to `DMA buffer`:

```cpp
#include <stdlib.h>
#include <immintrin.h>

#define likely(x)   __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

#if (__clang_major__+0 >= 3)
#define IS_ALIGNED(x, n) ((void *)(x))
#elif (__GNUC__+0 >= 4)
#define IS_ALIGNED(x, n) __builtin_assume_aligned((x), (n))
#else
#define IS_ALIGNED(x, n) ((void *)(x))
#endif

typedef __m256i __m256i_aligned __attribute__((aligned (32)));

void do_copy(register          __m256i_aligned *dst,
             register volatile __m256i_aligned *src,
             register          __m256i_aligned *end)
{
    do {
        register const __m256i m0 = src[0];
        register const __m256i m1 = src[1];
        register const __m256i m2 = src[2];
        register const __m256i m3 = src[3];

        __asm__ __volatile__ ("");

        _mm256_stream_si256( dst,     m0 );
        _mm256_stream_si256( dst + 1, m1 );
        _mm256_stream_si256( dst + 2, m2 );
        _mm256_stream_si256( dst + 3, m3 );

        __asm__ __volatile__ ("");

        src += 4;
        dst += 4;

    } while (likely(src < end));
}

void copy(void *dst, const void *src, const size_t bytes)
{
    if (bytes < 128)
        return;

    do_copy(IS_ALIGNED(dst, 32),
            IS_ALIGNED(src, 32),
            IS_ALIGNED((void *)((char *)src + bytes), 32));
}
```

Anyway, using compiler-only barriers that stop compiler reordering but don't emit a barrier instruction is one way to stop it. In this case, it's a way of hitting the compiler over the head and saying "stupid compiler, don't do that". I don't think you should normally need to do this everywhere, but clearly you can't trust gcc with write-combining stores (where ordering really matters). So it's probably a good idea to look at the asm at least with the compiler you're developing with when using NT loads and/or stores. [I've reported this for gcc](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=69622). Richard Biener points out that `-fno-schedule-insns2` is a sort-of workaround.

### 1.2 [gcc.gnu.org]: [Bug 69622 - compiler reordering of non-temporal (write-combining) stores produces significant performance hit](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=69622)

```text
A workaround is -fno-schedule-insns2.  I suppose the compiler is trying to increase the distance of the loads and stores (in a greedy way) to reduce
the impact on load latency in the general premise of moving loads up and
stores down.
```

### 1.3 [StackOverFlow]: [what is a store buffer?](https://stackoverflow.com/questions/11105827/what-is-a-store-buffer)

```text
An invalidate queue is more like a store buffer, but it's part of the memory system, not the CPU. Basically it is a queue that keeps track of invalidations and ensures that they complete properly so that a cache can take ownership of a cache line so it can then write that line. A load queue is a speculative structure that keeps track of in-flight loads in the out of order processor. For example, the following can occur

CPU speculatively issue a load from X
That load was in program order after a store to Y, but the address of Y is not resolved yet, so the store does not proceed.
Y is resolved and it turns out to be equal to X. At the time that the store to Y is resolved, that store searches the load queue for speculative loads that have issued, but are present after the store to Y in program order. It will notice the load to X (which is equal to Y) and have to squash those instructions starting with load X and following.
A store buffer is a speculative structure that exists in the CPU, just like the load queue and is for allowing the CPU to speculate on stores. A write combining buffer is part of the memory system and essentially takes a bunch of small writes (think 8 byte writes) and packs them into a single larger transaction (a 64-byte cache line) before sending them to the memory system. These writes are not speculative and are part of the coherence protocol. The goal is to save bus bandwidth. Typically, a write combining buffer is used for uncached writes to I/O devices (often for graphics cards). It's typical in I/O devices to do a bunch of programming of device registers by doing 8 byte writes and the write combining buffer allows those writes to be combined into larger transactions when shipping them out past the cache.
```
