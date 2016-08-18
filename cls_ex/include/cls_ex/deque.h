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

#include "allocator.h"

_CLS_BEGIN
template <typename T>
constexpr size_t DEFAULT_SUBARRAY_SIZE =
    sizeof(T) <= 4 ? 64 : sizeof(T) <= 8 ? 32 : sizeof(T) <= 16 ? 16 : sizeof(T) <= 32 ? 8 : 4;
constexpr size_t MIN_PTR_ARRAY_SIZE = 8;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DequeIterator
template <typename T, typename Pointer, typename Reference, size_t SUBARRAY_SIZE>
struct DequeIterator {
    using this_type         = DequeIterator<T, Pointer, Reference, SUBARRAY_SIZE>;
    using iterator          = DequeIterator<T, T*, T&, SUBARRAY_SIZE>;
    using const_iterator    = DequeIterator<T, const T*, const T&, SUBARRAY_SIZE>;
    using difference_type   = ptrdiff_t;
    using iterator_category = std::random_access_iterator_tag;
    using value_type        = T;
    using pointer           = T*;
    using reference         = T&;

    using Subarray = gsl::span<T, SUBARRAY_SIZE>;

    DequeIterator() = default;
    DequeIterator(const iterator& x)
        : m_sub_begin {x.m_sub_begin}, m_sub_end {x.m_sub_end}, m_current {x.m_current}, m_subarray {x.m_subarray} {}

    pointer   operator->() const;
    reference operator*() const;

    this_type& operator++();
    this_type  operator++(int);

    this_type& operator--();
    this_type  operator--(int);

    this_type& operator+=(difference_type n);
    this_type& operator-=(difference_type n);

    this_type operator+(difference_type n) const;
    this_type operator-(difference_type n) const;

protected:
    template <typename, typename, typename, size_t>
    friend struct DequeIterator;

    template <typename, typename, size_t>
    friend struct DequeBase;

    template <typename, typename, size_t>
    friend class Deque;

    template <typename U, typename PointerA, typename ReferenceA, typename PointerB, typename ReferenceB, size_t U_SUBARRAY_SIZE>
    friend bool operator==(const DequeIterator<U, PointerA, ReferenceA, U_SUBARRAY_SIZE>&,
                           const DequeIterator<U, PointerB, ReferenceB, U_SUBARRAY_SIZE>&);

    template <typename U, typename PointerA, typename ReferenceA, typename PointerB, typename ReferenceB, size_t U_SUBARRAY_SIZE>
    friend bool operator!=(const DequeIterator<U, PointerA, ReferenceA, U_SUBARRAY_SIZE>&,
                           const DequeIterator<U, PointerB, ReferenceB, U_SUBARRAY_SIZE>&);

    // Do we need this?
    template <typename U, typename PointerU, typename ReferenceU, size_t U_SUBARRAY_SIZE>
    friend bool operator!=(const DequeIterator<U, PointerU, ReferenceU, U_SUBARRAY_SIZE>&,
                           const DequeIterator<U, PointerU, ReferenceU, U_SUBARRAY_SIZE>&);

    template <typename U, typename PointerA, typename ReferenceA, typename PointerB, typename ReferenceB, size_t U_SUBARRAY_SIZE>
    friend bool operator< (const DequeIterator<U, PointerA, ReferenceA, U_SUBARRAY_SIZE>&,
                           const DequeIterator<U, PointerB, ReferenceB, U_SUBARRAY_SIZE>&);

    template <typename U, typename PointerA, typename ReferenceA, typename PointerB, typename ReferenceB, size_t U_SUBARRAY_SIZE>
    friend bool operator> (const DequeIterator<U, PointerA, ReferenceA, U_SUBARRAY_SIZE>&,
                           const DequeIterator<U, PointerB, ReferenceB, U_SUBARRAY_SIZE>&);

    template <typename U, typename PointerA, typename ReferenceA, typename PointerB, typename ReferenceB, size_t U_SUBARRAY_SIZE>
    friend bool operator<=(const DequeIterator<U, PointerA, ReferenceA, U_SUBARRAY_SIZE>&,
                           const DequeIterator<U, PointerB, ReferenceB, U_SUBARRAY_SIZE>&);

    template <typename U, typename PointerA, typename ReferenceA, typename PointerB, typename ReferenceB, size_t U_SUBARRAY_SIZE>
    friend bool operator>=(const DequeIterator<U, PointerA, ReferenceA, U_SUBARRAY_SIZE>&,
                           const DequeIterator<U, PointerB, ReferenceB, U_SUBARRAY_SIZE>&);

    template <typename U, typename PointerA, typename ReferenceA, typename PointerB, typename ReferenceB, size_t U_SUBARRAY_SIZE>
    friend auto operator-(const DequeIterator<U, PointerA, ReferenceA, U_SUBARRAY_SIZE>&,
                          const DequeIterator<U, PointerB, ReferenceB, U_SUBARRAY_SIZE>&) -> ptrdiff_t;

protected:
    struct Increment{};
    struct Decrement{};
    struct FromConst{};

    DequeIterator(Subarray* subarray, T* current) : m_current(current), m_sub_begin(subarray->data()),
                                                    m_sub_end(current + SUBARRAY_SIZE), m_subarray(subarray) {}
    DequeIterator(const const_iterator& x, FromConst) : m_current(x.m_current), m_sub_begin(x.m_sub_begin), m_sub_end(x.m_sub_end),
                                                        m_subarray(x.m_subarray) {}
    DequeIterator(const iterator& x, Increment) : DequeIterator(x) { ++(*this); }
    DequeIterator(const iterator& x, Decrement) : DequeIterator(x) { --(*this); }

    void setSubarray(Subarray* subarray)
    {
        m_subarray = subarray;
        m_sub_begin = m_subarray->data();
        m_sub_end = std::next(m_sub_begin, SUBARRAY_SIZE);
    }

    void setSubarray(Subarray& subarray)
    {
        setSubarray(&subarray);
    }

    // true means that value_type has the type_trait has_trivial_relocate, false means it does not
    this_type copy(const iterator& first, const iterator& last, std::true_type);
    this_type copy(const iterator& first, const iterator& last, std::false_type);

    void copy_backward(const iterator& first, const iterator& last, std::true_type);
    void copy_backward(const iterator& first, const iterator& last, std::false_type);

    T*  m_current        = nullptr;   // Pointer to current element
    T*  m_sub_begin      = nullptr;   // Pointer to beginning of current subarray
    T*  m_sub_end        = nullptr;   // Pointer to end of current subarray, equal to m_sub_begin + SUBARRAY_SIZE
    Subarray* m_subarray = nullptr;   // Pointer to current subarray
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DequeIterator
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T, typename Pointer, typename Reference, size_t SUBARRAY_SIZE>
auto DequeIterator<T, Pointer, Reference, SUBARRAY_SIZE>::operator->() const -> pointer
{
    return m_current;
}

template <typename T, typename Pointer, typename Reference, size_t SUBARRAY_SIZE>
auto DequeIterator<T, Pointer, Reference, SUBARRAY_SIZE>::operator*() const -> reference
{
    return *m_current;
}

template <typename T, typename Pointer, typename Reference, size_t SUBARRAY_SIZE>
auto DequeIterator<T, Pointer, Reference, SUBARRAY_SIZE>::operator++() -> this_type&
{
    if (++m_current == m_sub_end) {
        m_sub_begin = (++m_subarray)->data();
        m_sub_end = std::next(m_sub_begin, SUBARRAY_SIZE);
        m_current = m_sub_begin;
    }

    return *this;
}

template <typename T, typename Pointer, typename Reference, size_t SUBARRAY_SIZE>
auto DequeIterator<T, Pointer, Reference, SUBARRAY_SIZE>::operator++(int) -> this_type
{
    this_type tmp {*this};
    ++(*this);

    return tmp;
}

template <typename T, typename Pointer, typename Reference, size_t SUBARRAY_SIZE>
auto DequeIterator<T, Pointer, Reference, SUBARRAY_SIZE>::operator--() -> this_type&
{
    if (m_current == m_sub_begin) {
        m_sub_begin = (--m_subarray)->data();
        m_sub_end = std::next(m_sub_begin, SUBARRAY_SIZE);
        m_current = m_sub_end;
    }
    --m_current;

    return *this;
}

template <typename T, typename Pointer, typename Reference, size_t SUBARRAY_SIZE>
auto DequeIterator<T, Pointer, Reference, SUBARRAY_SIZE>::operator--(int) -> this_type
{
    this_type tmp {*this};
    --(*this);

    return tmp;
}

template <typename T, typename Pointer, typename Reference, size_t SUBARRAY_SIZE>
auto DequeIterator<T, Pointer, Reference, SUBARRAY_SIZE>::operator+=(difference_type n) -> this_type&
{
    const auto new_pos = std::distance(m_sub_begin, m_current) + n;

    if (0 <= new_pos && new_pos < SUBARRAY_SIZE) {
        std::advance(m_current, n);
    } else {
        // This is a branchless implementation which works by offsetting the math to always be
        // in the positive range. However, this algorithm will not work if n >= (2^32 - 2^24) or
        // if SUBARRAY_SIZE >= 2^24.
        static_assert((SUBARRAY_SIZE & SUBARRAY_SIZE - 1) == 0, "Subarray size is not power of 2");
        static constexpr difference_type offset = 1 << 24;
        const difference_type subarray_idx = (offset + new_pos) / SUBARRAY_SIZE - offset / SUBARRAY_SIZE;

        setSubarray(std::next(m_subarray, subarray_idx));
        m_current = std::next(m_sub_begin, new_pos - subarray_idx * SUBARRAY_SIZE);
    }

    return *this;
}

template <typename T, typename Pointer, typename Reference, size_t SUBARRAY_SIZE>
auto DequeIterator<T, Pointer, Reference, SUBARRAY_SIZE>::operator-=(difference_type n) -> this_type&
{
    *this += -n;

    return *this;
}

template <typename T, typename Pointer, typename Reference, size_t SUBARRAY_SIZE>
auto DequeIterator<T, Pointer, Reference, SUBARRAY_SIZE>::operator+(difference_type n) const -> this_type
{
    this_type tmp {*this};
    tmp += n;

    return tmp;
}

template <typename T, typename Pointer, typename Reference, size_t SUBARRAY_SIZE>
auto DequeIterator<T, Pointer, Reference, SUBARRAY_SIZE>::operator-(difference_type n) const -> this_type
{
    this_type tmp {*this};
    tmp += -n;

    return tmp;
}

template <typename U, typename PointerA, typename ReferenceA, typename PointerB, typename ReferenceB, size_t SUBARRAY_SIZE>
inline bool operator==(const DequeIterator<U, PointerA, ReferenceA, SUBARRAY_SIZE>& a,
                       const DequeIterator<U, PointerB, ReferenceB, SUBARRAY_SIZE>& b)
{
    return a.m_current == b.m_current;
}

template <typename U, typename PointerA, typename ReferenceA, typename PointerB, typename ReferenceB, size_t SUBARRAY_SIZE>
inline bool operator!=(const DequeIterator<U, PointerA, ReferenceA, SUBARRAY_SIZE>& a,
                       const DequeIterator<U, PointerB, ReferenceB, SUBARRAY_SIZE>& b)
{
    return a.m_current != b.m_current;
}

template <typename U, typename PointerU, typename ReferenceU, size_t SUBARRAY_SIZE>
inline bool operator!=(const DequeIterator<U, PointerU, ReferenceU, SUBARRAY_SIZE>& a,
                       const DequeIterator<U, PointerU, ReferenceU, SUBARRAY_SIZE>& b)
{
    return a.m_current != b.m_current;
}

template <typename U, typename PointerA, typename ReferenceA, typename PointerB, typename ReferenceB, size_t SUBARRAY_SIZE>
inline bool operator< (const DequeIterator<U, PointerA, ReferenceA, SUBARRAY_SIZE>& a,
                       const DequeIterator<U, PointerB, ReferenceB, SUBARRAY_SIZE>& b)
{
    return a.m_subarray == b.m_subarray ? a.m_current < b.m_current : a.m_subarray < b.m_subarray;
}

template <typename U, typename PointerA, typename ReferenceA, typename PointerB, typename ReferenceB, size_t SUBARRAY_SIZE>
inline bool operator> (const DequeIterator<U, PointerA, ReferenceA, SUBARRAY_SIZE>& a,
                       const DequeIterator<U, PointerB, ReferenceB, SUBARRAY_SIZE>& b)
{
    return a.m_subarray == b.m_subarray ? a.m_current > b.m_current : a.m_subarray > b.m_subarray;
}

template <typename U, typename PointerA, typename ReferenceA, typename PointerB, typename ReferenceB, size_t SUBARRAY_SIZE>
inline bool operator<=(const DequeIterator<U, PointerA, ReferenceA, SUBARRAY_SIZE>& a,
                       const DequeIterator<U, PointerB, ReferenceB, SUBARRAY_SIZE>& b)
{
    return a.m_subarray == b.m_subarray ? a.m_current <= b.m_current : a.m_subarray <= b.m_subarray;
}

template <typename U, typename PointerA, typename ReferenceA, typename PointerB, typename ReferenceB, size_t SUBARRAY_SIZE>
inline bool operator>=(const DequeIterator<U, PointerA, ReferenceA, SUBARRAY_SIZE>& a,
                       const DequeIterator<U, PointerB, ReferenceB, SUBARRAY_SIZE>& b)
{
    return a.m_subarray == b.m_subarray ? a.m_current >= b.m_current : a.m_subarray >= b.m_subarray;
}

template <typename T, typename Pointer, typename Reference, size_t SUBARRAY_SIZE>
inline auto operator+(ptrdiff_t n, const DequeIterator<T, Pointer, Reference, SUBARRAY_SIZE>& x)
{
    return x + n;
}

template <typename U, typename PointerA, typename ReferenceA, typename PointerB, typename ReferenceB, size_t SUBARRAY_SIZE>
inline auto operator-(const DequeIterator<U, PointerA, ReferenceA, SUBARRAY_SIZE>& a,
                      const DequeIterator<U, PointerB, ReferenceB, SUBARRAY_SIZE>& b) -> ptrdiff_t
{
    return SUBARRAY_SIZE * (a.m_subarray - b.m_subarray - 1) + (a.m_current - a.m_sub_begin) + (b.m_sub_end - b.m_current);
}

template <typename T, typename Pointer, typename Reference, size_t SUBARRAY_SIZE>
auto DequeIterator<T, Pointer, Reference, SUBARRAY_SIZE>::copy(
    const iterator& first, const iterator& last, std::true_type) -> this_type
{
    if ((first.m_sub_begin == last.m_sub_begin) && (first.m_sub_begin == m_sub_begin)) {
        memmove(m_current, first.m_current, static_cast<size_t>(last.m_current - first.m_current));
        return *this + (last.m_current - first.m_current);
    }

    return std::copy(first, last, *this);
}

template <typename T, typename Pointer, typename Reference, size_t SUBARRAY_SIZE>
auto DequeIterator<T, Pointer, Reference, SUBARRAY_SIZE>::copy(
    const iterator& first, const iterator& last, std::false_type) -> this_type
{
    return std::copy(first, last, *this);
}

template <typename T, typename Pointer, typename Reference, size_t SUBARRAY_SIZE>
void DequeIterator<T, Pointer, Reference, SUBARRAY_SIZE>::copy_backward(
    const iterator& first, const iterator& last, std::true_type)
{
    if ((first.m_sub_begin == last.m_sub_begin) && (first.m_sub_begin == m_sub_begin)) {
        memmove(m_current - (last.m_current - first.m_current), first.m_current, static_cast<size_t>(last.m_current - first.m_current));
    } else {
        std::copy_backward(first, last, *this);
    }
}

template <typename T, typename Pointer, typename Reference, size_t SUBARRAY_SIZE>
void DequeIterator<T, Pointer, Reference, SUBARRAY_SIZE>::copy_backward(
    const iterator& first, const iterator& last, std::false_type)
{
    std::copy_backward(first, last, *this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DequeBase
template <typename T, typename Alloc, size_t SUBARRAY_SIZE>
struct DequeBase {
    using Subarray = gsl::span<T, SUBARRAY_SIZE>;   // Subarray buffer that holds data
    using PtrArray = gsl::span<Subarray>;           // Array of pointers to buffer

#ifndef NDEBUG
    static_assert(sizeof(Subarray) == sizeof(T*), "Subarray size is wrong!");
    static_assert(sizeof(PtrArray) == sizeof(T**) + sizeof(size_t), "PtrArray size is wrong!");
#endif

    using value_type      = T;
    using allocator_type  = Alloc;
    using size_type       = size_t;
    using difference_type = ptrdiff_t;
    using iterator        = DequeIterator<T, T*, T&, SUBARRAY_SIZE>;
    using const_iterator  = DequeIterator<T, const T*, const T&, SUBARRAY_SIZE>;

    enum class Side {FRONT, BACK};

    explicit DequeBase(allocator_type* allocator) : m_allocator {allocator} {}
    explicit DequeBase(size_type n) : DequeBase(n, get_default_allocator()) {}
    DequeBase(size_type n, allocator_type* allocator) : m_allocator {allocator} { doInit(n); }

    virtual ~DequeBase()
    {
        if (!m_ptr_array.empty()) {
            freeContent();
        }
    }

    auto get_allocator() const -> const allocator_type&
    {
        return *m_allocator;
    }
    auto get_allocator() -> allocator_type&
    {
        return *m_allocator;
    }
    void set_allocator(allocator_type* allocator)
    {
        if (m_allocator != allocator) {
            if (!m_ptr_array.empty() && (m_begin.m_subarray == m_end.m_subarray)) {
                freeContent();

                m_allocator = allocator;
                doInit(0);
            } else {
                throw std::runtime_error {"Attemp to change allocator after allocating elements."};
            }
        }
    }

    void test()
    {
        doInit(3 * SUBARRAY_SIZE);
        doReallocPtrArray(2, Side::BACK);
        doReallocPtrArray(2, Side::FRONT);
        doReallocPtrArray(5, Side::BACK);
    }

protected:
    void freeContent();

    Subarray doAllocateSubarray();
    void doFreeSubarray(Subarray& sub_array);
    void doFreeSubarrays(Subarray* first, Subarray* last);

    PtrArray doAllocatePtrArray(size_type n);
    void doFreePtrArray(PtrArray& ptr_array);

    iterator doReallocSubarray(size_type additional_capacity, Side side);
    void doReallocPtrArray(size_type num_unused_ptr_needed, Side side);

    void doInit(size_type n);

    PtrArray m_ptr_array {};
    iterator m_begin {};
    iterator m_end {};
    allocator_type* m_allocator;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T, typename Alloc, size_t SUBARRAY_SIZE>
void DequeBase<T, Alloc, SUBARRAY_SIZE>::freeContent()
{
    doFreeSubarrays(m_begin.m_subarray, std::next(m_end.m_subarray));
    doFreePtrArray(m_ptr_array);
}

template <typename T, typename Alloc, size_t SUBARRAY_SIZE>
auto DequeBase<T, Alloc, SUBARRAY_SIZE>::doAllocateSubarray() -> Subarray
{
    auto p = alloc_array<value_type>(*m_allocator, SUBARRAY_SIZE);
    if (p == nullptr) {
        throw std::bad_alloc {};
    }

    return Subarray {p, static_cast<typename Subarray::index_type>(SUBARRAY_SIZE)};
}

template <typename T, typename Alloc, size_t SUBARRAY_SIZE>
void DequeBase<T, Alloc, SUBARRAY_SIZE>::doFreeSubarray(Subarray& sub_array)
{
    dealloc_array<value_type>(*m_allocator, sub_array);
}

template <typename T, typename Alloc, size_t SUBARRAY_SIZE>
void DequeBase<T, Alloc, SUBARRAY_SIZE>::doFreeSubarrays(Subarray* first, Subarray* last)
{
    while (first < last) {
        doFreeSubarray(*first++);
    }
}

template <typename T, typename Alloc, size_t SUBARRAY_SIZE>
auto DequeBase<T, Alloc, SUBARRAY_SIZE>::doAllocatePtrArray(size_type n) -> PtrArray
{
    auto p = alloc_array<Subarray>(*m_allocator, n);
    if (p == nullptr) {
        throw std::bad_alloc {};
    }

    return PtrArray {p, static_cast<typename PtrArray::index_type>(n)};
}

template <typename T, typename Alloc, size_t SUBARRAY_SIZE>
void DequeBase<T, Alloc, SUBARRAY_SIZE>::doFreePtrArray(PtrArray& ptr_array)
{
    dealloc_array<Subarray>(*m_allocator, ptr_array);
}

template <typename T, typename Alloc, size_t SUBARRAY_SIZE>
auto DequeBase<T, Alloc, SUBARRAY_SIZE>::doReallocSubarray(size_type additional_capacity, Side side) -> iterator
{
    if (side == Side::FRONT) {
        const auto front_capacity = static_cast<size_type>(std::distance(m_begin.m_sub_begin, m_begin.m_current));

        if (front_capacity < additional_capacity) {
            const auto num_subarray_needed = static_cast<difference_type>(((additional_capacity - front_capacity) + SUBARRAY_SIZE - 1) / SUBARRAY_SIZE);

            // If there are not enough pointers at front
            const auto num_ptrs_avail = m_begin.m_subarray - m_ptr_array.data();
            if(num_subarray_needed > num_ptrs_avail) {
                doReallocPtrArray(static_cast<size_type>(num_subarray_needed - num_ptrs_avail), Side::FRONT);
            }

            for(difference_type i = 1; i <= num_subarray_needed; ++i) {
                m_begin.m_subarray[-i] = doAllocateSubarray();
            }
        }

        return m_begin - additional_capacity;
    } else {
        const size_type back_capacity = static_cast<size_type>(std::distance(m_end.m_current, std::prev(m_end.m_sub_end)));

        if (back_capacity < additional_capacity) {
            const auto num_subarray_needed = static_cast<difference_type>(((additional_capacity - back_capacity) + SUBARRAY_SIZE - 1) / SUBARRAY_SIZE);

            // If there are not enough pointers at back
            const auto num_ptrs_avail = ((m_ptr_array.data() + m_ptr_array.size()) - m_end.m_subarray) - 1;
            if(num_subarray_needed > num_ptrs_avail) {
                doReallocPtrArray(static_cast<size_type>(num_subarray_needed - num_ptrs_avail), Side::BACK);
            }

            for(difference_type i = 1; i <= num_subarray_needed; ++i) {
                m_end.m_subarray[i] = doAllocateSubarray();
            }
        }

        return m_end + additional_capacity;
    }
}

template <typename T, typename Alloc, size_t SUBARRAY_SIZE>
void DequeBase<T, Alloc, SUBARRAY_SIZE>::doReallocPtrArray(size_type num_unused_ptr_needed, Side side)
{
    const size_type num_unused_ptr_front      = static_cast<size_type>(std::distance(m_ptr_array.data(), m_begin.m_subarray));
    const size_type num_used_ptr              = static_cast<size_type>(std::distance(m_begin.m_subarray, m_end.m_subarray)) + 1;
    const size_type num_used_ptr_size         = num_used_ptr * sizeof(void*);
    const size_type num_unused_ptr_back       = (m_ptr_array.size() - num_unused_ptr_front) - num_used_ptr;
    Subarray*       new_used_ptr_array_begin  = nullptr;

    // If we have enough unused pointers, we could just move them around (Use memmove to take care of overlap)
    if (side == Side::BACK && num_unused_ptr_needed <= num_unused_ptr_front) {
        // Balance unused space
        auto min_num_ptr_to_move = num_unused_ptr_front / 2;
        num_unused_ptr_needed = std::max(num_unused_ptr_needed, min_num_ptr_to_move);

        new_used_ptr_array_begin = std::prev(m_begin.m_subarray, num_unused_ptr_needed);
        memmove(new_used_ptr_array_begin, m_begin.m_subarray, num_used_ptr_size);
    } else if (side == Side::FRONT && num_unused_ptr_needed <= num_unused_ptr_back) {
        // Balance unused space
        auto min_num_ptr_to_move = num_unused_ptr_back / 2;
        num_unused_ptr_needed = std::max(num_unused_ptr_needed, min_num_ptr_to_move);

        new_used_ptr_array_begin = std::next(m_begin.m_subarray, num_unused_ptr_needed);
        memmove(new_used_ptr_array_begin, m_begin.m_subarray, num_used_ptr_size);
    } else { // In this case we will have to do a reallocation.
        // Double the ptr_array capacity or allocate more if needed
        const size_type old_ptr_array_size = static_cast<size_type>(m_ptr_array.size());
        const size_type new_ptr_array_size = old_ptr_array_size + std::max(old_ptr_array_size, num_unused_ptr_needed);
        auto new_ptr_array= doAllocatePtrArray(new_ptr_array_size);

        new_used_ptr_array_begin = std::next(new_ptr_array.data(),
                                             num_unused_ptr_front + (side == Side::FRONT ? num_unused_ptr_needed : 0));

        // Copy and free the old content
        if (!m_ptr_array.empty()) {
            memcpy(new_used_ptr_array_begin, m_begin.m_subarray, num_used_ptr_size);
        }
        doFreePtrArray(m_ptr_array);

        m_ptr_array = new_ptr_array;
    }

    // Reset the begin and end iterators
    m_begin.setSubarray(new_used_ptr_array_begin);
    m_end.setSubarray(std::next(new_used_ptr_array_begin, num_used_ptr - 1));
}

template <typename T, typename Alloc, size_t SUBARRAY_SIZE>
void DequeBase<T, Alloc, SUBARRAY_SIZE>::doInit(size_type n)
{
    const size_type new_ptr_array_size = static_cast<size_type>((n / SUBARRAY_SIZE) + 1);

    // Reserve at least 1 uninitialized subarray ptr at both front and end
    const size_type reserve_ptr_array_size = std::max(MIN_PTR_ARRAY_SIZE, new_ptr_array_size + 2);
    m_ptr_array = doAllocatePtrArray(reserve_ptr_array_size);

    auto start = (reserve_ptr_array_size - new_ptr_array_size) / 2;
    auto inited_subarrays = m_ptr_array.subspan(start, new_ptr_array_size);
    for (auto& subarray : inited_subarrays) {
        subarray = doAllocateSubarray();
    }

    // No front() and back() for gsl::span?
    m_begin.setSubarray(*inited_subarrays.begin());
    m_begin.m_current = m_begin.m_sub_begin;

    m_end.setSubarray(*std::prev(inited_subarrays.end()));
    m_end.m_current = std::next(m_end.m_sub_begin, n % SUBARRAY_SIZE);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Deque
template <typename T, typename Alloc = Allocator, size_t SUBARRAY_SIZE = DEFAULT_SUBARRAY_SIZE<T>>
class Deque : public DequeBase<T, Alloc, SUBARRAY_SIZE> {
public:
    using base_type = DequeBase<T, Alloc, SUBARRAY_SIZE>;
    using this_type = Deque<T, Alloc, SUBARRAY_SIZE>;
    using value_type =             T;
    using pointer =                T*;
    using const_pointer =          const T*;
    using reference =              T&;
    using const_reference =        const T&;
    using iterator =               DequeIterator<T, T*, T&, SUBARRAY_SIZE>;
    using const_iterator =         DequeIterator<T, const T*, const T&, SUBARRAY_SIZE>;
    using reverse_iterator =       std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using size_type =              typename base_type::size_type;
    using difference_type =        typename base_type::difference_type;
    using allocator_type =         typename base_type::allocator_type;

    using Subarray = typename base_type::Subarray;
    using PtrArray = typename base_type::PtrArray;
    using Side     = typename base_type::Side;
    using base_type::m_ptr_array;
    using base_type::m_begin;
    using base_type::m_end;
    using base_type::m_allocator;
    using base_type::doAllocateSubarray;
    using base_type::doFreeSubarray;
    using base_type::doFreeSubarrays;
    using base_type::doAllocatePtrArray;
    using base_type::doFreePtrArray;
    using base_type::doReallocSubarray;
    using base_type::doReallocPtrArray;

    Deque();
    explicit Deque(allocator_type* allocator);
    explicit Deque(size_type n, allocator_type* alloc = get_default_allocator());
    explicit Deque(size_type n, const value_type& value, allocator_type* alloc = get_default_allocator());

    Deque(const this_type& other);
    Deque(this_type&& other);

    template <typename InputIterator>
    Deque(InputIterator first, InputIterator last, allocator_type* alloc = get_default_allocator());
    Deque(std::initializer_list<value_type> values, allocator_type* alloc = get_default_allocator());

    ~Deque();

    this_type& operator=(const this_type& rhs);
    this_type& operator=(this_type&& rhs);
    this_type& operator=(std::initializer_list<value_type> rhs);

    void swap(Deque& other);

    // It turns out that the C++ std::deque<int, int> specifies a two argument
    // version of assign that takes (int size, int value). These are not
    // iterators, so we need to do a template compiler trick to do the right thing.
    void assign(size_type n, const value_type& value)
    {
        doAssignValues(n, value);
    }

    template <typename InputIterator>
    void assign(InputIterator first, InputIterator last)
    {
        doAssign(first, last, std::is_integral<InputIterator>());
    }

    void assign(std::initializer_list<value_type> values)
    {
        doAssign(values.begin(), values.end(), std::false_type {});
    }

    auto begin() -> iterator { return m_begin; }
    auto begin() const -> const_iterator { return m_begin; }
    auto cbegin() const -> const_iterator { return m_begin; }

    auto end() -> iterator { return m_end; }
    auto end() const -> const_iterator  { return m_end; }
    auto cend() const -> const_iterator  { return m_end; }

    auto rbegin() -> reverse_iterator  { return reverse_iterator {m_end}; }
    auto rbegin() const -> const_reverse_iterator { return const_reverse_iterator {m_end}; }
    auto crbegin() const -> const_reverse_iterator { return const_reverse_iterator {m_end}; }

    auto rend() -> reverse_iterator { return reverse_iterator {m_begin}; }
    auto rend() const -> const_reverse_iterator { return const_reverse_iterator {m_begin}; }
    auto crend() const -> const_reverse_iterator { return const_reverse_iterator {m_begin}; }

    bool empty() const
    {
        return m_begin.m_current == m_end.m_current;
    }

    size_type size() const
    {
        return static_cast<size_type>(std::distance(m_begin, m_end));
    }

    void resize(size_type n, const value_type& value)
    {
        const size_type curr_size = size();

        if (n > curr_size) {
            insert(m_end, n - curr_size, value);
        } else {
            erase(std::next(m_begin, n), m_end);
        }
    }

    void resize(size_type n)
    {
        resize(n, value_type {});
    }

    void shrink_to_fit()
    {
        // TODO, shrink_to_fit(), doing nothing for now
    }

    void set_capacity(size_type n)
    {
        if (n == 0) {
            this_type tmp {m_allocator};
            doSwap(tmp);
        } else if (n < size()) {
            resize(n);
        }
    }

    auto operator[](size_type n) -> reference
    {
        return *(m_begin + n);
    }

    auto operator[](size_type n) const -> const_reference
    {
        return *(m_begin + n);
    }

    auto at(size_type n) -> reference
    {
        return *(m_begin + n);
    }
    auto at(size_type n) const -> const_reference
    {
        return *(m_begin + n);
    }

    auto front() -> reference
    {
        return *m_begin;
    }

    auto front() const -> const_reference
    {
        return *m_begin;
    }

    auto back() -> reference
    {
        return *iterator {m_end, typename iterator::Decrement {}};
    }

    auto back() const -> const_reference
    {
        return *iterator {m_end, typename iterator::Decrement {}};
    }

    void push_front(const value_type& value)
    {
        emplace_front(value);
    }

    void push_front(value_type&& value)
    {
        emplace_front(std::move(value));
    }

    auto push_front() -> reference
    {
        emplace_front(value_type {});
        return *m_begin;
    }

    void push_back(const value_type& value)
    {
        emplace_back(value);
    }

    void push_back(value_type&& value)
    {
        emplace_back(std::move(value));
    }

    auto push_back() -> reference
    {
        emplace_back(value_type {});
        return *iterator {m_end, typename iterator::Decrement {}};
    }

    void pop_front()
    {
        if (std::next(m_begin.m_current) != m_begin.m_sub_end) {
            (m_begin.m_current++)->~value_type();
        } else {
            m_begin.m_current->~value_type();
            doFreeSubarray(*m_begin.m_subarray);
            m_begin.setSubarray(std::next(m_begin.m_subarray));
            m_begin.m_current = m_begin.m_sub_begin;
        }
    }

    void pop_back()
    {
        if (m_end.m_current != m_end.m_sub_begin) {
            (--m_end.m_current)->~value_type();
        } else {
            doFreeSubarray(*m_end.m_subarray);
            m_end.setSubarray(std::prev(m_end.m_subarray));
            m_end.m_current = std::prev(m_end.m_sub_end);
            m_end.m_current->~value_type();
        }
    }

    template<class... Args>
    iterator emplace(const_iterator position, Args&&... args)
    {
        if (position.m_current == m_end.m_current) {
            emplace_back(std::forward<Args>(args)...);
            return iterator {m_end, typename iterator::Decrement {}};
        }

        if (position.m_current == m_begin.m_current) {
            emplace_front(std::forward<Args>(args)...);
            return m_begin;
        }

        value_type value {std::forward<Args>(args)...};
        iterator iter_pos {position, typename iterator::FromConst {}};
        const difference_type i = iter_pos - m_begin;
        if (i < static_cast<difference_type>(size() / 2)) {
            emplace_front(*m_begin);

            iter_pos = m_begin + i;

            const iterator new_pos {iter_pos, typename iterator::Increment {}};
            const iterator old_beg {m_begin, typename iterator::Increment {}};
            const iterator old_beg_next {old_beg, typename iterator::Increment {}};

            old_beg.copy(old_beg_next, new_pos, has_trivial_relocate<value_type>());
        } else {
            emplace_back(*iterator {m_end, typename iterator::Decrement {}});

            iter_pos = m_begin + i;

            iterator old_back {m_end, typename iterator::Decrement {}};
            const iterator old_back_prev {old_back, typename iterator::Decrement {}};

            old_back.copy_backward(iter_pos, old_back_prev, has_trivial_relocate<value_type>());
        }

        *iter_pos = std::move(value);

        return iter_pos;
    }

    template<class... Args>
    void emplace_front(Args&&... args)
    {
        // We have room in the first subarray
        if (m_begin.m_current != m_begin.m_sub_begin) {
            ::new(--m_begin.m_current) value_type {std::forward<Args>(args)...};
        } else {
            // Need to make a temporary copy first, because args may be a value_type that comes from within our
            // container and the operations below may change the container.
            value_type value {std::forward<Args>(args)...};

            // We are at the beginning of first subarray
            if (m_begin.m_subarray == m_ptr_array.data()) {
                doReallocPtrArray(1, Side::FRONT);
            }

            *std::prev(m_begin.m_subarray) = doAllocateSubarray();

            m_begin.setSubarray(std::prev(m_begin.m_subarray));
            m_begin.m_current = std::prev(m_begin.m_sub_end);
            ::new(m_begin.m_current) value_type {std::move(value)};
        }
    }

    template<class... Args>
    void emplace_back(Args&&... args)
    {
        // We have room in the last subarray
        if (std::next(m_end.m_current) != m_end.m_sub_end) {
            ::new(m_end.m_current++) value_type {std::forward<Args>(args)...};
        } else {
            // Need to make a temporary copy first, because args may be a value_type that comes from within our
            // container and the operations below may change the container.
            value_type value {std::forward<Args>(args)...};

            // We are at the end of last subarray
            if (std::distance(m_ptr_array.data(), m_end.m_subarray) + 1 >= m_ptr_array.size()) {
                doReallocPtrArray(1, Side::BACK);
            }

            *std::next(m_end.m_subarray) = doAllocateSubarray();

            ::new(m_end.m_current) value_type {std::move(value)};
            m_end.setSubarray(std::next(m_end.m_subarray));
            m_end.m_current = m_end.m_sub_begin;
        }
    }

    iterator insert(const_iterator position, const value_type& value)
    {
        return emplace(position, value);
    }

    iterator insert(const_iterator position, value_type&& value)
    {
        return emplace(position, std::move(value));
    }

    void insert(const_iterator position, size_type n, const value_type& value)
    {
        doInsertValues(position, n, value);
    }

    template <typename InputIterator>
    void insert(const_iterator position, InputIterator first, InputIterator last)
    {
        doInsert(position, first, last, std::is_integral<InputIterator>());
    }

    iterator insert(const_iterator position, std::initializer_list<value_type> values)
    {
        const auto i = std::distance(m_begin, position);
        doInsert(position, values.begin(), values.end(), std::false_type {});

        return std::next(m_begin, i);
    }

    iterator erase(const_iterator position)
    {
        iterator iter_pos(position, typename iterator::FromConst {});
        iterator iter_next(iter_pos, typename iterator::Increment {});
        const difference_type i = iter_pos - m_begin;

        if (i < static_cast<difference_type>(size() / 2)) {
            iter_next.copy_backward(m_begin, iter_pos, has_trivial_relocate<value_type>());
            pop_front();
        } else {
            iter_pos.copy(iter_next, m_end, has_trivial_relocate<value_type>());
            pop_back();
        }

        return m_begin + i;
    }

    iterator erase(const_iterator first, const_iterator last)
    {
        iterator it_first(first, typename iterator::FromConst());
        iterator it_last(last, typename iterator::FromConst());

        // If not erasing everything
        if((it_first != m_begin) || (it_last != m_end)) {
            const difference_type n(it_last - it_first);
            const difference_type i(it_first - m_begin);

            // Should we move the front entries forward or the back entries backward? We divide the range in half
            if(i < static_cast<difference_type>((size() - n) / 2)) {
                const iterator it_new_begin = std::next(m_begin, n);
                Subarray* const subarray_begin= m_begin.m_subarray;

                it_last.copy_backward(m_begin, it_first, has_trivial_relocate<value_type>());

                for(; m_begin != it_new_begin; ++m_begin) {
                    m_begin.m_current->~value_type();
                }

                doFreeSubarrays(subarray_begin, it_new_begin.m_subarray);
            } else {    // Else we will be moving back entries backward
                iterator it_new_end = std::prev(m_end, n);
                Subarray* const subarray_end = it_new_end.m_subarray + 1;

                it_first.copy(it_last, m_end, has_trivial_relocate<value_type>());

                for(iterator iter = it_new_end; iter != m_end; ++iter) {
                    iter.m_current->~value_type();
                }

                doFreeSubarrays(subarray_end, m_end.m_subarray + 1);

                m_end = it_new_end;
            }

            return std::next(m_begin, i);
        }

        clear();
        return m_end;
    }

    reverse_iterator erase(reverse_iterator position)
    {
        return reverse_iterator {erase((++position).base())};
    }

    reverse_iterator erase(reverse_iterator first, reverse_iterator last)
    {
        return reverse_iterator {erase(last.base(), first.base())};
    }

    void clear()
    {
        if (m_begin.m_subarray != m_end.m_subarray) {
            for (auto p1 = m_begin.m_current; p1 < m_begin.m_sub_end; ++p1) {
                p1->~value_type();
            }
            for(auto p2 = m_end.m_sub_begin; p2 < m_end.m_current; ++p2) {
                p2->~value_type();
            }
            doFreeSubarray(*m_end.m_subarray); // Leave mItBegin with a valid subarray.
        } else {
            for (auto p = m_begin.m_current; p < m_end.m_current; ++p) {
                p->~value_type();
            }
        }

        for (auto subarray = std::next(m_begin.m_subarray); subarray < m_end.m_subarray; ++subarray) {
            for (auto& ele : *subarray) {
                ele.~value_type();
            }
            doFreeSubarray(*subarray);
        }

        m_end = m_begin;
    }

protected:
    template <typename Integer>
    void doInit(Integer n, Integer value, std::true_type);
    template <typename InputIterator>
    void doInit(InputIterator first, InputIterator last, std::false_type);

    template <typename InputIterator>
    void doInitFromIterator(InputIterator first, InputIterator last, std::input_iterator_tag);
    template <typename ForwardIterator>
    void doInitFromIterator(ForwardIterator first, ForwardIterator last, std::forward_iterator_tag);

    void doFillInit(const value_type& value);

    template <typename Integer>
    void doAssign(Integer count, Integer value, std::true_type);
    template <typename InputIterator>
    void doAssign(InputIterator first, InputIterator last, std::false_type);
    void doAssignValues(size_type n, const value_type& value);

    template <typename Integer>
    void doInsert(const const_iterator& position, Integer n, Integer value, std::true_type)
    {
        doInsertValues(position, static_cast<size_type>(n), static_cast<value_type>(value));
    }

    template <typename InputIterator>
    void doInsert(const const_iterator& position, const InputIterator& first, const InputIterator& last, std::false_type)
    {
        doInsertFromIterator(position, first, last, typename std::iterator_traits<InputIterator>::iterator_category {});
    }

    template <typename InputIterator>
    void doInsertFromIterator(const_iterator position, const InputIterator& first, const InputIterator& last, std::forward_iterator_tag)
    {
        const size_type n = static_cast<size_type>(std::distance(first, last));

        if (position.m_current == m_begin.m_current) {      // Inserting at beginning
            const iterator iter_new_beg {doReallocSubarray(n, Side::FRONT)};
            std::uninitialized_copy(first, last, iter_new_beg);
            m_begin = iter_new_beg;
        } else if (position.m_current == m_end.m_current) { // Inserting at end
            const iterator iter_new_end {doReallocSubarray(n, Side::BACK)};
            std::uninitialized_copy(first, last, m_end);
            m_end = iter_new_end;
        } else {
            // TODO insert values in the middle
            throw std::runtime_error {"Insert values in the middle is not implemented."};
        }
    }

    void doInsertValues(const_iterator position, size_type n, const value_type& value)
    {
        if (position.m_current == m_begin.m_current) {      // Inserting at beginning
            const iterator iter_new_beg {doReallocSubarray(n, Side::FRONT)};
            std::uninitialized_fill(iter_new_beg, m_begin, value);
            m_begin = iter_new_beg;
        } else if (position.m_current == m_end.m_current) { // Inserting at end
            const iterator iter_new_end {doReallocSubarray(n, Side::BACK)};
            std::uninitialized_fill(m_end, iter_new_end, value);
            m_end = iter_new_end;
        } else {
            // TODO insert values in the middle
            throw std::runtime_error {"Insert values in the middle is not implemented."};
        }
    }

    void doSwap(this_type& other);
};

template <typename T, typename Alloc, size_t SUBARRAY_SIZE>
Deque<T, Alloc, SUBARRAY_SIZE>::Deque()
    : base_type {static_cast<size_type>(0)}
{}

template <typename T, typename Alloc, size_t SUBARRAY_SIZE>
Deque<T, Alloc, SUBARRAY_SIZE>::Deque(allocator_type* allocator)
    : base_type {0, allocator}
{}

template <typename T, typename Alloc, size_t SUBARRAY_SIZE>
Deque<T, Alloc, SUBARRAY_SIZE>::Deque(size_type n, allocator_type* alloc)
    : Deque {n, value_type {}, alloc}
{}

template <typename T, typename Alloc, size_t SUBARRAY_SIZE>
Deque<T, Alloc, SUBARRAY_SIZE>::Deque(size_type n, const value_type& value, allocator_type* alloc)
    : base_type {n, alloc}
{
    doFillInit(value);
}

template <typename T, typename Alloc, size_t SUBARRAY_SIZE>
Deque<T, Alloc, SUBARRAY_SIZE>::Deque(const this_type& other)
    : base_type {other.size(), other.m_allocator}
{
    std::uninitialized_copy(other.m_begin, other.m_end, m_begin);
}

template <typename T, typename Alloc, size_t SUBARRAY_SIZE>
Deque<T, Alloc, SUBARRAY_SIZE>::Deque(this_type&& other)
    : base_type {0, other.m_allocator}
{
    swap(other);
}

template <typename T, typename Alloc, size_t SUBARRAY_SIZE>
Deque<T, Alloc, SUBARRAY_SIZE>::Deque(std::initializer_list<value_type> values, allocator_type* alloc)
    : base_type {alloc}
{
    doInit(values.begin(), values.end(), std::false_type {});
}

template <typename T, typename Alloc, size_t SUBARRAY_SIZE>
template <typename InputIterator>
Deque<T, Alloc, SUBARRAY_SIZE>::Deque(InputIterator first, InputIterator last, allocator_type* alloc)
    : base_type {alloc}
{
    doInit(first, last, std::is_integral<InputIterator>());
}

template <typename T, typename Alloc, size_t SUBARRAY_SIZE>
Deque<T, Alloc, SUBARRAY_SIZE>::~Deque()
{
    for (auto current = m_begin; current != m_end; ++current) {
        current.m_current->~value_type();
    }
}

template <typename T, typename Alloc, size_t SUBARRAY_SIZE>
auto Deque<T, Alloc, SUBARRAY_SIZE>::operator=(const this_type& rhs) -> this_type&
{
    if (&rhs != this) {
        doAssign(rhs.m_begin, rhs.m_end, std::false_type {});
    }

    return *this;
}

template <typename T, typename Alloc, size_t SUBARRAY_SIZE>
auto Deque<T, Alloc, SUBARRAY_SIZE>::operator=(this_type&& rhs) -> this_type&
{
    if (&rhs != this) {
        set_capacity(0);
        swap(rhs);
    }

    return *this;
}

template <typename T, typename Alloc, size_t SUBARRAY_SIZE>
auto Deque<T, Alloc, SUBARRAY_SIZE>::operator=(std::initializer_list<value_type> rhs) -> this_type&
{
    doAssign(rhs.begin(), rhs.end(), std::false_type {});

    return *this;
}

template <typename T, typename Alloc, size_t SUBARRAY_SIZE>
void Deque<T, Alloc, SUBARRAY_SIZE>::swap(Deque& other)
{
    if (*m_allocator == *other.m_allocator) {
        doSwap(other);
    } else {
        const this_type tmp(*this);
        *this = other;
        other = tmp;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Internal functions
template <typename T, typename Alloc, size_t SUBARRAY_SIZE>
template <typename Integer>
void Deque<T, Alloc, SUBARRAY_SIZE>::doInit(Integer n, Integer value, std::true_type)
{
    base_type::doInit(n);
    doFillInit(value);
}

template <typename T, typename Alloc, size_t SUBARRAY_SIZE>
template <typename InputIterator>
void Deque<T, Alloc, SUBARRAY_SIZE>::doInit(InputIterator first, InputIterator last, std::false_type)
{
    doInitFromIterator(first, last, typename std::iterator_traits<InputIterator>::iterator_category {});
}

template <typename T, typename Alloc, size_t SUBARRAY_SIZE>
template <typename InputIterator>
void Deque<T, Alloc, SUBARRAY_SIZE>::doInitFromIterator(InputIterator first, InputIterator last, std::input_iterator_tag)
{
    base_type::doInit(0);

    for (; first != last; ++first) {
        push_back(*first);
    }
}

template <typename T, typename Alloc, size_t SUBARRAY_SIZE>
template <typename ForwardIterator>
void Deque<T, Alloc, SUBARRAY_SIZE>::doInitFromIterator(ForwardIterator first, ForwardIterator last, std::forward_iterator_tag)
{
    using non_const_iterator_type = typename std::remove_const<ForwardIterator>::type;
    using non_const_value_type    = typename std::remove_const<value_type>::type;

    const size_type n = static_cast<size_type>(std::distance(first, last));
    base_type::doInit(n);

    Subarray* current_subarray = nullptr;
    for (current_subarray = m_begin.m_subarray; current_subarray < m_end.m_subarray; ++current_subarray) {
        auto current = first;
        std::advance(current, SUBARRAY_SIZE);
        std::uninitialized_copy(static_cast<non_const_iterator_type>(first),
                                static_cast<non_const_iterator_type>(current),
                                static_cast<non_const_value_type*>(current_subarray->data()));
        first = current;
    }

    std::uninitialized_copy(static_cast<non_const_iterator_type>(first),
                            static_cast<non_const_iterator_type>(last),
                            static_cast<non_const_value_type*>(m_end.m_sub_begin));
}

template <typename T, typename Alloc, size_t SUBARRAY_SIZE>
void Deque<T, Alloc, SUBARRAY_SIZE>::doFillInit(const value_type& value)
{
    auto subarray = m_begin.m_subarray;

    while (subarray < m_end.m_subarray) {
        std::uninitialized_fill(subarray->begin(), subarray->end(), value);
        ++subarray;
    }
    std::uninitialized_fill(m_end.m_sub_begin, m_end.m_current, value);
}

template <typename T, typename Alloc, size_t SUBARRAY_SIZE>
template <typename Integer>
void Deque<T, Alloc, SUBARRAY_SIZE>::doAssign(Integer count, Integer value, std::true_type)
{
    doAssignValues(static_cast<size_type>(count), static_cast<value_type>(value));
}

template <typename T, typename Alloc, size_t SUBARRAY_SIZE>
template <typename InputIterator>
void Deque<T, Alloc, SUBARRAY_SIZE>::doAssign(InputIterator first, InputIterator last, std::false_type)
{
    const size_type n = static_cast<size_type>(std::distance(first, last));
    const size_type s = size();

    if (n > s) {
        auto end = std::next(first, s);
        std::copy(first, end, m_begin);
        insert(m_end, end, last);
    } else {
        auto end = std::copy(first, last, m_begin);
        if (n < s) {
            erase(end, m_end);
        }
    }
}

template <typename T, typename Alloc, size_t SUBARRAY_SIZE>
void Deque<T, Alloc, SUBARRAY_SIZE>::doAssignValues(size_type n, const value_type& value)
{
    const size_type s = size();
    if (n > s) {
        std::fill(m_begin, m_end, value);
        insert(m_end, n - s, value);
    } else {
        erase(std::next(m_begin, n), m_end);
        std::fill(m_begin, m_end, value);
    }
}

template <typename T, typename Alloc, size_t SUBARRAY_SIZE>
void Deque<T, Alloc, SUBARRAY_SIZE>::doSwap(this_type& other)
{
    std::swap(m_ptr_array, other.m_ptr_array);
    std::swap(m_begin, other.m_begin);
    std::swap(m_end, other.m_end);
    std::swap(m_allocator, other.m_allocator);
}

_CLS_END