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

#include <boost/align/aligned_alloc.hpp>
#include <cls/cls_defs.h>

_CLS_BEGIN
constexpr size_t MIN_ALIGNMENT = 16;
constexpr size_t PLAT_PTR_SIZE = sizeof(void*);

template <typename T>
struct has_trivial_relocate : std::integral_constant<bool, std::is_pod<T>::value && !std::is_volatile<T>::value> {};

class Allocator
{
public:
    virtual void* allocate(size_t n) = 0;
    virtual void* allocate(size_t n, size_t alignment) = 0;
    virtual void  deallocate(void* p, size_t n) = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DefaultAllocator
class DefaultAllocator : public Allocator
{
public:
    void* allocate(size_t n) override
    {
        return boost::alignment::aligned_alloc(MIN_ALIGNMENT, n);
    }

    void* allocate(size_t n, size_t alignment) override
    {
        return boost::alignment::aligned_alloc(std::max(MIN_ALIGNMENT, alignment), n);
    }

    void  deallocate(void* p, size_t n) override
    {
        boost::alignment::aligned_free(p);
    }
};

inline bool operator==(const DefaultAllocator& a, const DefaultAllocator& b)
{
    return true;
}

inline bool operator!=(const DefaultAllocator& a, const DefaultAllocator& b)
{
    return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename Alloc>
void* alloc_memory(Alloc& alloc, size_t n, size_t alignment, size_t align_offset = 0)
{
    if (alignment <= MIN_ALIGNMENT) {
        return alloc.allocate(n);
    }

    return alloc.allocate(n, alignment);
}

template <typename T, typename Alloc>
T* alloc_array(Alloc& alloc, size_t array_size)
{
    return static_cast<T*>(alloc_memory(alloc, array_size * sizeof(T), alignof(T)));;
};

template <typename Alloc>
void dealloc_memory(Alloc& alloc, void* p, size_t n)
{
    alloc.deallocate(p, n);
}

template <typename T, typename Alloc>
void dealloc_array(Alloc& alloc, gsl::span<T> array)
{
    dealloc_memory(alloc, array.data(), array.size() * sizeof(T));
};

template<typename Alloc>
Allocator* get_allocator()
{
    static Alloc alloc;
    return &alloc;
}

inline Allocator* get_default_allocator()
{
    return get_allocator<DefaultAllocator>();
}
_CLS_END