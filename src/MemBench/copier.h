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

#ifndef JSTD_MEMORY_COPIER_H_
#define JSTD_MEMORY_COPIER_H_

#include <memory.h>

#include <cstring>
#include <memory>

#include <cstdlib>
#include <thread>
#include <vector>

namespace jstd {
namespace memory {

class Copier {
public:
    virtual ~Copier() = default;
    // Each derived class of `Copier` needs to also add `PtrT` for compatibility
    // with`MTCopier`. See `DefaultCopier` for example.
    virtual void *alloc(size_t) = 0;
    virtual void dealloc(void *) = 0;
    virtual void copy(void *, void *, size_t) = 0;
};

//------------------------------------------------------------------------------

class DefaultCopier : public Copier {
public:
    using PtrT = uint8_t;

    void * alloc(size_t size) override {
        return ::malloc(size);
    }

    void dealloc(void * ptr) override {
        ::free(ptr);
    }

    void copy(void * dst, void * src, size_t size) override {
        std::memcpy(dst, src, size);
    }
};

//------------------------------------------------------------------------------

template <class BaseCopierT, uint32_t nthreads>
class MTCopier : public Copier {
public:
    MTCopier() : base_copier() {}

    void *alloc(size_t size) override {
        return base_copier.alloc(size);
    }

    void dealloc(void *ptr) override {
        base_copier.dealloc(ptr);
    }

    void _copy(void *d, void *s, size_t n) {
        n = SHMALIGN(n, sizeof(typename BaseCopierT::PtrT)) /
            sizeof(typename BaseCopierT::PtrT);
        std::vector<std::thread> threads;
        threads.reserve(nthreads);

        size_t per_worker_quot = (nthreads != 0) ? ((int64_t)n / nthreads) : 0;
        size_t per_worker_rem = (nthreads != 0) ? ((int64_t)n % nthreads) : 0;

        size_t next_start = 0;
        for (uint32_t thread_idx = 0; thread_idx < nthreads; ++thread_idx) {
            const size_t curr_start = next_start;
            next_start += per_worker_quot;
            if (thread_idx < per_worker_rem) {
                ++next_start;
            }
            auto d_thread = reinterpret_cast<typename BaseCopierT::PtrT *>(d) + curr_start;
            auto s_thread = reinterpret_cast<typename BaseCopierT::PtrT *>(s) + curr_start;

            threads.emplace_back(
                &Copier::copy, &base_copier, d_thread, s_thread,
                (next_start - curr_start) * sizeof(typename BaseCopierT::PtrT));
        }
        for (auto &thread : threads) {
            thread.join();
        }
        threads.clear();
    }

    void copy(void *dst, void *src, size_t size) override {
        _copy(dst, src, size, true);
    }

private:
    BaseCopierT base_copier;
};

//------------------------------------------------------------------------------

} // namespace memory
} // namespace jstd

#endif  // JSTD_MEMORY_COPIER_H_
