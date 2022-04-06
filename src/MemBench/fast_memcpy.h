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

#ifndef JSTD_MEMORY_FAST_MEMCPY_H_
#define JSTD_MEMORY_FAST_MEMCPY_H_

#include <immintrin.h>

#include <cstdlib>

#include "MemBench/copier.h"

namespace jstd {
namespace memory {

inline uint8_t * align_address(void * ptr, size_t alignment) {
    auto int_ptr = reinterpret_cast<uintptr_t>(ptr);
    auto aligned_int_ptr = ADDR_ALIGNTO(int_ptr, alignment);
    return reinterpret_cast<uint8_t *>(aligned_int_ptr);
}

//------------------------------------------------------------------------------

#ifdef __x86_64__

static inline void _rep_movsb(void *d, const void *s, size_t n) {
    asm volatile("rep movsb"
        : "=D"(d), "=S"(s), "=c"(n)
        : "0"(d), "1"(s), "2"(n)
        : "memory");
}

class RepMovsbCopier : public Copier {
public:
    using PtrT = uint8_t;
    void *alloc(size_t size) override {
        return malloc(size);
    }

    void dealloc(void *ptr) override {
        free(ptr);
    }

    void copy(void *dst, void *src, size_t size) override {
        _rep_movsb(dst, src, size);
    }
};

#endif // __x86_64__

//------------------------------------------------------------------------------

#ifdef __AVX__

static inline void _avx_cpy(void *d, const void *s, size_t n) {
    // d, s -> 32 byte aligned
    // n -> multiple of 32

    auto *dVec = reinterpret_cast<__m256i *>(d);
    const auto *sVec = reinterpret_cast<const __m256i *>(s);
    size_t nVec = n / sizeof(__m256i);
    for (; nVec > 0; nVec--, sVec++, dVec++) {
        const __m256i temp = _mm256_load_si256(sVec);
        _mm256_store_si256(dVec, temp);
    }
}

class AvxCopier : public Copier {
public:
    using PtrT = __m256i;
    constexpr static size_t alignment = sizeof(__m256i);

    void *alloc(size_t size) override {
        return aligned_alloc(alignment, ADDR_ALIGNTO(size, alignment));
    }

    void dealloc(void *ptr) override {
        free(ptr);
    }

    void copy(void *dst, void *src, size_t size) override {
        _avx_cpy(dst, src, ADDR_ALIGNTO(size, alignment));
    }
};

#endif // __AVX__

//------------------------------------------------------------------------------

#ifdef __AVX2__

static inline void _avx_async_cpy(void *d, const void *s, size_t n) {
    // d, s -> 32 byte aligned
    // n -> multiple of 32

    auto *dVec = reinterpret_cast<__m256i *>(d);
    const auto *sVec = reinterpret_cast<const __m256i *>(s);
    size_t nVec = n / sizeof(__m256i);
    for (; nVec > 0; nVec--, sVec++, dVec++) {
        const __m256i temp = _mm256_stream_load_si256(sVec);
        _mm256_stream_si256(dVec, temp);
    }
    _mm_sfence();
}

class AvxAsyncCopier : public Copier {
public:
    using PtrT = __m256i;
    constexpr static size_t alignment = sizeof(__m256i);

    void *alloc(size_t size) override {
        return aligned_alloc(alignment, ADDR_ALIGNTO(size, alignment));
    }

    void dealloc(void *ptr) override {
        free(ptr);
    }

    void copy(void *dst, void *src, size_t size) override {
        _avx_async_cpy(dst, src, ADDR_ALIGNTO(size, alignment));
    }
};

#endif // __AVX2__

//------------------------------------------------------------------------------

#ifdef __AVX2__

static inline void _avx_async_pf_cpy(void *d, const void *s, size_t n) {
    // d, s -> 64 byte aligned
    // n -> multiple of 64

    auto *dVec = reinterpret_cast<__m256i *>(d);
    const auto *sVec = reinterpret_cast<const __m256i *>(s);
    size_t nVec = n / sizeof(__m256i);
    for (; nVec > 2; nVec -= 2, sVec += 2, dVec += 2) {
        // prefetch the next iteration's data
        // by default _mm_prefetch moves the entire cache-lint (64b)
        _mm_prefetch(sVec + 2, _MM_HINT_T0);

        _mm256_stream_si256(dVec, _mm256_load_si256(sVec));
        _mm256_stream_si256(dVec + 1, _mm256_load_si256(sVec + 1));
    }
    _mm256_stream_si256(dVec, _mm256_load_si256(sVec));
    _mm256_stream_si256(dVec + 1, _mm256_load_si256(sVec + 1));
    _mm_sfence();
}

class AvxAsyncPFCopier : public Copier {
public:
    using PtrT = __m256i;
    constexpr static size_t alignment = sizeof(__m256i) * 2;

    void *alloc(size_t size) override {
        return aligned_alloc(alignment, ADDR_ALIGNTO(size, alignment));
    }

    void dealloc(void *ptr) override {
        free(ptr);
    }

    void copy(void *dst, void *src, size_t size) override {
        _avx_async_pf_cpy(dst, src, ADDR_ALIGNTO(size, alignment));
    }
};

#endif // __AVX2__

//------------------------------------------------------------------------------

#ifdef __AVX__

static inline void _avx_cpy_unroll(void *d, const void *s, size_t n) {
    // d, s -> 128 byte aligned
    // n -> multiple of 128

    auto *dVec = reinterpret_cast<__m256i *>(d);
    const auto *sVec = reinterpret_cast<const __m256i *>(s);
    size_t nVec = n / sizeof(__m256i);
    for (; nVec > 0; nVec -= 4, sVec += 4, dVec += 4) {
        _mm256_store_si256(dVec, _mm256_load_si256(sVec));
        _mm256_store_si256(dVec + 1, _mm256_load_si256(sVec + 1));
        _mm256_store_si256(dVec + 2, _mm256_load_si256(sVec + 2));
        _mm256_store_si256(dVec + 3, _mm256_load_si256(sVec + 3));
    }
}

class AvxUnrollCopier : public Copier {
public:
    using PtrT = __m256i;
    constexpr static size_t alignment = 4 * sizeof(__m256i);

    void *alloc(size_t size) override {
        return aligned_alloc(alignment, ADDR_ALIGNTO(size, alignment));
    }

    void dealloc(void *ptr) override {
        free(ptr);
    }

    void copy(void *dst, void *src, size_t size) override {
        _avx_cpy_unroll(dst, src, ADDR_ALIGNTO(size, alignment));
    }
};

#endif // __AVX__

//------------------------------------------------------------------------------

#ifdef __AVX2__

static inline void _avx_async_cpy_unroll(void *d, const void *s, size_t n) {
    // d, s -> 128 byte aligned
    // n -> multiple of 128

    auto *dVec = reinterpret_cast<__m256i *>(d);
    const auto *sVec = reinterpret_cast<const __m256i *>(s);
    size_t nVec = n / sizeof(__m256i);
    for (; nVec > 0; nVec -= 4, sVec += 4, dVec += 4) {
        _mm256_stream_si256(dVec, _mm256_stream_load_si256(sVec));
        _mm256_stream_si256(dVec + 1, _mm256_stream_load_si256(sVec + 1));
        _mm256_stream_si256(dVec + 2, _mm256_stream_load_si256(sVec + 2));
        _mm256_stream_si256(dVec + 3, _mm256_stream_load_si256(sVec + 3));
    }
    _mm_sfence();
}

class AvxAsyncUnrollCopier : public Copier {
public:
    using PtrT = __m256i;
    constexpr static size_t alignment = 4 * sizeof(__m256i);

    void *alloc(size_t size) override {
        return aligned_alloc(alignment, ADDR_ALIGNTO(size, alignment));
    }

    void dealloc(void *ptr) override {
        free(ptr);
    }

    void copy(void *dst, void *src, size_t size) override {
        _avx_async_cpy_unroll(dst, src, ADDR_ALIGNTO(size, alignment));
    }
};

#endif // __AVX2__

//------------------------------------------------------------------------------

#ifdef __AVX2__

static inline void _avx_async_pf_cpy_unroll(void *d, const void *s, size_t n) {
    // d, s -> 128 byte aligned
    // n -> multiple of 128

    auto *dVec = reinterpret_cast<__m256i *>(d);
    const auto *sVec = reinterpret_cast<const __m256i *>(s);
    size_t nVec = n / sizeof(__m256i);
    for (; nVec > 4; nVec -= 4, sVec += 4, dVec += 4) {
        // prefetch data for next iteration
        _mm_prefetch(sVec + 4, _MM_HINT_T0);
        _mm_prefetch(sVec + 6, _MM_HINT_T0);
        _mm256_stream_si256(dVec, _mm256_load_si256(sVec));
        _mm256_stream_si256(dVec + 1, _mm256_load_si256(sVec + 1));
        _mm256_stream_si256(dVec + 2, _mm256_load_si256(sVec + 2));
        _mm256_stream_si256(dVec + 3, _mm256_load_si256(sVec + 3));
    }
    _mm256_stream_si256(dVec, _mm256_load_si256(sVec));
    _mm256_stream_si256(dVec + 1, _mm256_load_si256(sVec + 1));
    _mm256_stream_si256(dVec + 2, _mm256_load_si256(sVec + 2));
    _mm256_stream_si256(dVec + 3, _mm256_load_si256(sVec + 3));
    _mm_sfence();
}

class AvxAsyncPFUnrollCopier : public Copier {
public:
    using PtrT = __m256i;
    constexpr static size_t alignment = 4 * sizeof(__m256i);

    void *alloc(size_t size) override {
        return aligned_alloc(alignment, ADDR_ALIGNTO(size, alignment));
    }

    void dealloc(void *ptr) override {
        free(ptr);
    }

    void copy(void *dst, void *src, size_t size) override {
        _avx_async_pf_cpy_unroll(dst, src, ADDR_ALIGNTO(size, alignment));
    }
};

#endif // __AVX2__

//------------------------------------------------------------------------------

} // namespace memory
} // namespace jstd

#endif  // JSTD_MEMORY_FAST_MEMCPY_H_
