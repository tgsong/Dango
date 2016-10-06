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

#include <vector>
#include "allocator.h"

_CLS_BEGIN
namespace detail {

template <typename T, size_t SIZE>
class SubarrayT {
    T* m_storage;

public:
    using View = gsl::span<T, SIZE>;

    SubarrayT()
    {
        auto p = alloc_array<T>(get_allocator(), SIZE);
        if (p == nullptr) {
            throw std::bad_alloc {};
        }

        m_storage = p;
    }

    ~SubarrayT()
    {
        dealloc_array<T>(get_allocator(), View {m_storage, SIZE});
    }

    SubarrayT(const SubarrayT&) = delete;
    SubarrayT(SubarrayT&&) = delete;
    SubarrayT& operator=(const SubarrayT&) = delete;
    SubarrayT& operator=(SubarrayT&&) = delete;

    static void* operator new(size_t size)
    {
        return alloc_memory(get_allocator(), size, alignof(SubarrayT));
    }

    static void operator delete(void* mem)
    {
        dealloc_memory(get_allocator(), mem, sizeof(SubarrayT));
    }

    template <typename... Args>
    void construct(int idx, Args&&... args)
    {
        ::new(m_storage + idx) T {std::forward<Args>(args)...};
    }

    void destruct(int idx)
    {
        m_storage[idx].~T();
    }

    View view()
    {
        return View {m_storage, SIZE};
    }
};

template <typename T, typename Pointer, typename Reference, size_t SUBARRAY_SIZE>
struct DequeIterator {
    using Subarray = SubarrayT<T, SUBARRAY_SIZE>;

    using this_type         = DequeIterator<T, Pointer, Reference, SUBARRAY_SIZE>;
    using iterator          = DequeIterator<T, T*, T&, SUBARRAY_SIZE>;
    using const_iterator    = DequeIterator<T, const T*, const T&, SUBARRAY_SIZE>;
    using difference_type   = ptrdiff_t;
    using iterator_category = std::random_access_iterator_tag;
    using value_type        = T;
    using pointer           = T*;
    using reference         = T&;

    pointer   operator->() const { return m_current; }
    reference operator*() const { return *m_current; }

    T* m_current = nullptr;
};

template <typename T, size_t SUBARRAY_SIZE, typename Alloc>
class DequeImpl {
    using Subarray = SubarrayT<T, SUBARRAY_SIZE>;

    using value_type      = T;
    using allocator_type  = Alloc;
    using size_type       = size_t;
    using difference_type = ptrdiff_t;
    using iterator        = DequeIterator<T, T*, T&, SUBARRAY_SIZE>;
    using const_iterator  = DequeIterator<T, const T*, const T&, SUBARRAY_SIZE>;

    // Array of pointers to Subarrays
    std::vector<std::unique_ptr<Subarray>> m_ptr_array;

public:
    void destruct(iterator first, iterator last)
    {
        while (first != last)
        {
            first->~value_type();
        }
    }
};

}   // namespace detail

_CLS_END