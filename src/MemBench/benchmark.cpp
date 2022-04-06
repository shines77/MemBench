/*==============================================================================

 MIT License

Copyright (c) 2022 Guo XiongHhui (shines77)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
==============================================================================*/

//
// From: https://squadrick.dev/journal/going-faster-than-memcpy.html
//

#include "MemBench/fast_memcpy.h"
#include <benchmark/benchmark.h>

#include <cstring>

#define START_RANGE   (1 << 5)    // 32 bytes
#define END_RANGE     (1 << 23)   // 8 MB
#define SHM_RANGE     START_RANGE, END_RANGE

template <class CopierT>
void CopyBench(benchmark::State & state) {  // NOLINT
    CopierT copier;
    size_t size = state.range(0);
    auto * src = copier.alloc(size);
    auto * dst = copier.alloc(size);
    std::memset(src, 'x', size);
    for (auto _ : state) {
        copier.copy(dst, src, size);
        benchmark::DoNotOptimize(dst);
    }
    copier.dealloc(src);
    copier.dealloc(dst);
    state.SetBytesProcessed(size * static_cast<int64_t>(state.iterations()));
}

#define SHM_COPY_BENCHMARK(c) \
    BENCHMARK_TEMPLATE(CopyBench, c)->Range(SHM_RANGE);

SHM_COPY_BENCHMARK(jstd::memory::DefaultCopier);

#ifdef __x86_64__
SHM_COPY_BENCHMARK(jstd::memory::RepMovsbCopier);
#endif

#ifdef __AVX__
SHM_COPY_BENCHMARK(jstd::memory::AvxCopier);
SHM_COPY_BENCHMARK(jstd::memory::AvxUnrollCopier);
#endif

#ifdef __AVX2__
SHM_COPY_BENCHMARK(jstd::memory::AvxAsyncCopier);
SHM_COPY_BENCHMARK(jstd::memory::AvxAsyncPFCopier);
SHM_COPY_BENCHMARK(jstd::memory::AvxAsyncUnrollCopier);
SHM_COPY_BENCHMARK(jstd::memory::AvxAsyncPFUnrollCopier);
#endif

BENCHMARK_MAIN();
