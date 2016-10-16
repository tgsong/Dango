/////////////////////////////////////////////////////////////////////////////////
// The MIT License(MIT)
//
// Copyright (c) 2016 Tiangang Song
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <thread>
#include <mutex>
#include <iostream>
#include <boost/align/aligned_alloc.hpp>
#include <cls/cls_defs.h>

CLS_BEGIN
constexpr size_type MIN_ALIGNMENT = 16;
constexpr size_type PLAT_PTR_SIZE = sizeof(void*);

#ifndef NDEBUG
// Track memory allocation, detecting leak
static thread_local size_type g_allocate_size = 0;
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Allocation helpers
template <typename Alloc>
void* alloc_memory(Alloc* alloc, size_type n, size_type alignment)
{
#ifndef NDEBUG
    g_allocate_size += n;
    // printf("Allocate %d bytes\n", n);
#endif

    if (alignment <= MIN_ALIGNMENT) {
        return alloc->allocate(n);
    }

    return alloc->allocate(n, alignment);
}

template <typename Alloc>
void* alloc_memory(Alloc& alloc, size_type n, size_type alignment)
{
    return alloc_memory(&alloc, n, alignment);
}

template <typename T, typename Alloc>
T* alloc_array(Alloc* alloc, size_type array_size)
{
    return static_cast<T*>(alloc_memory(alloc, array_size * size_of<T>, align_of<T>));
}

template <typename T, typename Alloc>
T* alloc_array(Alloc& alloc, size_type array_size)
{
    return alloc_array<T>(&alloc, array_size);
}

template <typename Alloc>
void dealloc_memory(Alloc* alloc, void* p, size_type n)
{
#ifndef NDEBUG
    g_allocate_size -= n;
    if (g_allocate_size == 0) {
        static std::mutex mtx;
        std::unique_lock<std::mutex> display_lock(mtx);
        std::cout << "Thread id = " << std::this_thread::get_id() << ", No leak occurred.\n";
    }
#endif

    alloc->deallocate(p, n);
}

template <typename Alloc>
void dealloc_memory(Alloc& alloc, void* p, size_type n)
{
    dealloc_memory(&alloc, p, n);
}

template <typename T, typename Alloc>
void dealloc_array(Alloc* alloc, gsl::span<T> array)
{
    dealloc_memory(alloc, array.data(), array.size() * size_of<T>);
}

template <typename T, typename Alloc>
void dealloc_array(Alloc& alloc, gsl::span<T> array)
{
    dealloc_array<T>(&alloc, array);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Allocator interface
class Allocator {
public:
    Allocator() = default;
    Allocator(const Allocator&) = default;
    Allocator(Allocator&&) = default;
    Allocator& operator=(Allocator&&) = default;
    Allocator& operator=(const Allocator&) = default;
    virtual ~Allocator();

    virtual void* allocate(size_type n) = 0;
    virtual void* allocate(size_type n, size_type alignment) = 0;
    virtual void  deallocate(void* p, size_type n) = 0;
};

// The global allocator used in system
class ActiveAllocator {
    static std::unique_ptr<Allocator> m_allocator;

public:
    static Allocator* get() { return m_allocator.get(); }
    static void reset(Allocator* alloc) { m_allocator.reset(alloc); }
    static void reset(std::unique_ptr<Allocator>&& alloc) { m_allocator = std::move(alloc); }
};

// Used in STL types which require an allocator
template <typename T>
class STLAllocator {
public:
    using value_type = T;
    using size_type = cls::size_type;

    T* allocate(size_type n)
    {
        return static_cast<T*>(alloc_memory(ActiveAllocator::get(), size_of<T> * n, align_of<T>));
    }

    void deallocate(T* p, size_type n)
    {
        dealloc_memory(ActiveAllocator::get(), p, size_of<T> * n);
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DefaultAllocator
class DefaultAllocator : public Allocator {
public:
    void* allocate(size_type n) override;
    void* allocate(size_type n, size_type alignment) override;
    void  deallocate(void* p, size_type) override;
};

inline bool operator==(const DefaultAllocator&, const DefaultAllocator&)
{
    return true;
}

inline bool operator!=(const DefaultAllocator&, const DefaultAllocator&)
{
    return false;
}

CLS_END
