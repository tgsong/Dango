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

#include <cstddef>
#include <vector>
#include "cls/traits.hpp"
#include "allocator.h"

CLS_BEGIN
namespace detail {
template <typename T>
constexpr size_type DEFAULT_SUBARRAY_SIZE =
    sizeof(T) <= 4 ? 64 : sizeof(T) <= 8 ? 32 : sizeof(T) <= 16 ? 16 : sizeof(T) <= 32 ? 8 : 4;
constexpr size_type MIN_PTR_ARRAY_SIZE = 8;

template <typename ContainerT>
inline auto makeView(ContainerT& container)
{
    return gsl::span<typename ContainerT::value_type> {container};
}

template <typename ContainerT>
inline auto makeView(ContainerT& container, size_type offset, size_type count)
{
    return makeView(container).subspan(offset, count);
}

template <typename Iterator>
inline auto makeView(Iterator first, Iterator last)
{
    return gsl::span<typename std::iterator_traits<Iterator>::value_type> {first, last};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SubarrayT
template <typename T, size_type SIZE>
class SubarrayT {
    struct Deletor {
        void operator()(T* p)
        {
            dealloc_memory(ActiveAllocator::get(), p, size_of<T> * SIZE);
        }
    };

    std::unique_ptr<T[], Deletor> m_storage;

public:
    SubarrayT()
    {
        auto p = alloc_array<T>(ActiveAllocator::get(), SIZE);
        if (p == nullptr) {
            throw std::bad_alloc {};
        }

        m_storage.reset(p);
    }

    static void* operator new(size_t n)
    {
        return alloc_memory(ActiveAllocator::get(), static_cast<size_type>(n), align_of<SubarrayT>);
    }

    static void operator delete(void* p)
    {
        dealloc_memory(ActiveAllocator::get(), p, size_of<SubarrayT>);
    }

    template <typename... Args>
    void construct(T* pos, Args&&... args)
    {
#ifndef NDEBUG
        assert(m_storage.get() <= pos && pos < m_storage.get() + SIZE);
#endif
        ::new(pos) T {std::forward<Args>(args)...};
    }

    template <typename... Args>
    void construct(int idx, Args&&... args)
    {
        construct(m_storage.get() + idx, std::forward<Args>(args)...);
    }

    void destruct(T* pos)
    {
#ifndef NDEBUG
        assert(m_storage.get() <= pos && pos < m_storage.get() + SIZE);
#endif
        pos->~T();
    }

    void destruct(int idx)
    {
        destruct(m_storage.get() + idx);
    }

    T* data() { return m_storage.get(); }

    T* begin() { return m_storage.get(); }
    const T* begin() const { return m_storage.get(); }

    T* end() { return m_storage.get() + SIZE; }
    const T* end() const { return m_storage.get() + SIZE; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DequeIterator
template <typename T, typename Pointer, typename Reference, size_type SUBARRAY_SIZE>
struct DequeIterator {
    using this_type         = DequeIterator<T, Pointer, Reference, SUBARRAY_SIZE>;
    using iterator          = DequeIterator<T, T*, T&, SUBARRAY_SIZE>;
    using const_iterator    = DequeIterator<T, const T*, const T&, SUBARRAY_SIZE>;
    using difference_type   = std::ptrdiff_t;
    using iterator_category = std::random_access_iterator_tag;
    using value_type        = T;
    using pointer           = Pointer;
    using reference         = Reference;
    using Subarray          = SubarrayT<T, SUBARRAY_SIZE>;
    using SubarrayPtr       = std::unique_ptr<SubarrayT<T, SUBARRAY_SIZE>>;

    template <typename, typename, typename, size_type>
    friend struct DequeIterator;

    template <typename, size_type>
    friend struct DequeImpl;

    template <typename, size_type>
    friend struct Deque;

    DequeIterator() = default;

    // Some template trick to support construct/assign const_iterator from iterator
    template <typename Iterator = iterator,
        typename = std::enable_if_t<
            std::is_same<Iterator, iterator>::value &&
            std::is_same<this_type, const_iterator>::value>>
    DequeIterator(const iterator& rhs) : m_current {rhs.m_current}, m_subarray {rhs.m_subarray} {}

    template <typename Iterator = iterator,
        typename = std::enable_if_t<
            std::is_same<Iterator, iterator>::value &&
            std::is_same<this_type, const_iterator>::value>>
    DequeIterator& operator=(const iterator& rhs)
    {
        m_current = rhs.m_current;
        m_subarray = rhs.m_subarray;
        return *this;
    }

    auto operator->() const -> pointer { return m_current; }
    auto operator*() const -> reference { return *m_current; }

    this_type& operator++()
    {
        if (++m_current == subEnd()) {
            ++m_subarray;
            m_current = subBegin();
        }

        return *this;
    }

    this_type operator++(int)
    {
        this_type tmp = *this;
        ++(*this);

        return tmp;
    }

    this_type& operator--()
    {
        if (m_current == subBegin()) {
            --m_subarray;
            m_current = subEnd();
        }
        --m_current;

        return *this;
    }

    this_type operator--(int)
    {
        this_type tmp = *this;
        --(*this);

        return tmp;
    }

    this_type& operator+=(difference_type n)
    {
        const auto new_pos = std::distance(subBegin(), m_current) + n;
        if (0 <= new_pos && new_pos < SUBARRAY_SIZE) {
            std::advance(m_current, n);
        } else {
            // This is a branchless implementation which works by offsetting the math to always be
            // in the positive range. However, this algorithm will not work if n >= (2^32 - 2^24) or
            // if SUBARRAY_SIZE >= 2^24.
            static_assert((SUBARRAY_SIZE & (SUBARRAY_SIZE - 1)) == 0, "Subarray size is not power of 2");
            static constexpr difference_type offset = 1 << 24;
            const auto subarray_idx = (offset + new_pos) / SUBARRAY_SIZE - offset / SUBARRAY_SIZE;

            setSubarray(std::next(m_subarray, subarray_idx));
            m_current = std::next(subBegin(), new_pos - subarray_idx * SUBARRAY_SIZE);
        }

        return *this;
    }

    this_type& operator-=(difference_type n)
    {
        *this += -n;

        return *this;
    }

    this_type operator+(difference_type n) const
    {
        this_type tmp = *this;
        tmp += n;

        return tmp;
    }
    this_type operator-(difference_type n) const
    {
        this_type tmp = *this;
        tmp += -n;

        return tmp;
    }

    template <typename U, typename PointerU, typename ReferenceU>
    difference_type operator-(const DequeIterator<U, PointerU, ReferenceU, SUBARRAY_SIZE>& x) const
    {
        return SUBARRAY_SIZE * (m_subarray - x.m_subarray - 1) +
            (m_current - x.subBegin()) + (x.subEnd() - x.m_current);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Comparison
    template <typename U, typename PointerU, typename ReferenceU>
    bool operator==(const DequeIterator<U, PointerU, ReferenceU, SUBARRAY_SIZE>& rhs) const
    {
        return m_current == rhs.m_current;
    }

    template <typename U, typename PointerU, typename ReferenceU>
    bool operator!=(const DequeIterator<U, PointerU, ReferenceU, SUBARRAY_SIZE>& rhs) const
    {
        return m_current != rhs.m_current;
    }

    template <typename U, typename PointerU, typename ReferenceU>
    bool operator<(const DequeIterator<U, PointerU, ReferenceU, SUBARRAY_SIZE>& rhs) const
    {
        return m_subarray == rhs.m_subarray ? m_current < rhs.m_current : m_subarray < rhs.m_subarray;
    }

    template <typename U, typename PointerU, typename ReferenceU>
    bool operator>(const DequeIterator<U, PointerU, ReferenceU, SUBARRAY_SIZE>& rhs) const
    {
        return m_subarray == rhs.m_subarray ? m_current > rhs.m_current : m_subarray > rhs.m_subarray;
    }

    template <typename U, typename PointerU, typename ReferenceU>
    bool operator<=(const DequeIterator<U, PointerU, ReferenceU, SUBARRAY_SIZE>& rhs) const
    {
        return *this == rhs || *this < rhs;
    }

    template <typename U, typename PointerU, typename ReferenceU>
    bool operator>=(const DequeIterator<U, PointerU, ReferenceU, SUBARRAY_SIZE>& rhs) const
    {
        return *this == rhs || *this > rhs;
    }

protected:
    DequeIterator(T* current, SubarrayPtr* subarray) : m_current {current}, m_subarray {subarray} {}

    iterator fromConst() const
    {
        return iterator {m_current, m_subarray};
    }

    // Usually you also want to set m_current after this operation
    void setSubarray(SubarrayPtr* subarray)
    {
        m_subarray = subarray;
    }

    T* subBegin() const
    {
        return m_subarray->get()->begin();
    }

    T* subEnd() const
    {
        return m_subarray->get()->end();
    }

    Subarray* subarray() const
    {
        return m_subarray->get();
    }

protected:
    T* m_current = nullptr;
    SubarrayPtr* m_subarray = nullptr;
};


// Support integer + iterator
template <typename T, typename Pointer, typename Reference, size_type SUBARRAY_SIZE>
auto operator+(std::ptrdiff_t n, const DequeIterator<T, Pointer, Reference, SUBARRAY_SIZE>& x)
{
    return x + n;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DequeImpl
template <typename T, size_type SUBARRAY_SIZE>
class DequeImpl {
public:
    using Subarray    = SubarrayT<T, SUBARRAY_SIZE>;
    using SubarrayPtr = std::unique_ptr<Subarray>;

    using this_type       = DequeImpl<T, SUBARRAY_SIZE>;
    using value_type      = T;
    using pointer         = T*;
    using const_pointer   = const T*;
    using reference       = T&;
    using const_reference = const T&;
    using difference_type = std::ptrdiff_t;
    using iterator        = DequeIterator<T, T*, T&, SUBARRAY_SIZE>;
    using const_iterator  = DequeIterator<T, const T*, const T&, SUBARRAY_SIZE>;
    using reverse_iterator =       std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    enum class Side {FRONT, BACK};

    DequeImpl() = default;
    explicit DequeImpl(size_type n) { init(n); }

    DequeImpl(const DequeImpl&) = delete;
    DequeImpl(DequeImpl&&) = delete;
    DequeImpl& operator=(const DequeImpl&) = delete;
    DequeImpl& operator=(DequeImpl&&) = delete;

    // Destruct elements before memory get deallocated
    ~DequeImpl()
    {
        destruct(m_begin, m_end);
    }

    void assign(size_type n, const value_type& value)
    {
        const auto s = size();
        if (n > s) {
            std::fill(m_begin, m_end, value);
            insert(m_end, n - s, value);
        } else {
            erase(std::next(m_begin, n), m_end);
            std::fill(m_begin, m_end, value);
        }
    }

    template <typename ForwardIter, typename = std::enable_if_t<is_forward_iterator<ForwardIter>::value>>
    void assign(ForwardIter first, ForwardIter last)
    {
        const auto new_size = std::distance(first, last);
        const auto old_size = size();

        if (new_size > old_size) {
            auto mid = std::next(first, old_size);
            std::copy(first, mid, m_begin);
            insert(m_end, mid, last);
        } else {
            auto new_end = std::copy(first, last, m_begin);
            if (new_size < old_size) {
                erase(new_end, m_end);
            }
        }
    }

    void assign(std::initializer_list<value_type> values)
    {
        assign(values.begin(), values.end());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Element access
    auto at(size_type n) -> reference
    {
        if (n < 0 || n >= size()) throw std::out_of_range {"index is out of range!"};
        return *(m_begin + n);
    }
    auto at(size_type n) const -> const_reference
    {
        if (n < 0 || n >= size()) throw std::out_of_range {"index is out of range!"};
        return *(m_begin + n);
    }

    auto operator[](size_type n) -> reference { return *(m_begin + n); }
    auto operator[](size_type n) const -> const_reference { return *(m_begin + n); }

    auto front() -> reference { return *m_begin; }
    auto front() const -> const_reference { return *m_begin; }

    auto back() -> reference { return *(m_end - 1); }
    auto back() const -> const_reference { return *(m_end - 1); }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Iterators
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

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Capacity
    bool empty() const
    {
        return m_begin.m_current == m_end.m_current;
    }

    size_type size() const
    {
        return m_end - m_begin;
    }

    void shrink_to_fit()
    {
        // TODO this method has no effect for now
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers
    void clear()
    {
        destruct(m_begin, m_end);
        for (auto& subarray : makeView(m_begin.m_subarray + 1, m_end.m_subarray)) {
            subarray.reset();
        }
        m_end = m_begin;
    }

    iterator insert(const_iterator pos, const value_type& v)
    {
        return emplace(pos, v);
    }

    iterator insert(const_iterator pos, value_type&& v)
    {
        return emplace(pos, std::move(v));
    }

    iterator insert(const_iterator pos, size_type n, const value_type& v)
    {
        if (pos.m_current == m_begin.m_current) {
            // Insert at beginning
            const auto new_begin = reallocSubarray(n, Side::FRONT);
            std::uninitialized_fill(new_begin, m_begin, v);
            m_begin = new_begin;
        } else if (pos.m_current == m_end.m_current) {
            // Insert at end
            const auto new_end = reallocSubarray(n, Side::BACK);
            std::uninitialized_fill(m_end, new_end, v);
            m_end = new_end;
        } else {
            // TODO insert values in the middle
            throw std::runtime_error {"Insert values in the middle is not implemented yet."};
        }

        return pos.fromConst();
    }

    template <typename ForwardIter, typename = std::enable_if_t<is_forward_iterator<ForwardIter>::value>>
    iterator insert(const_iterator pos, ForwardIter first, ForwardIter last)
    {
        const auto n = std::distance(first, last);

        if (pos.m_current == m_begin.m_current) {
            // Insert at beginning
            const auto new_begin = reallocSubarray(n, Side::FRONT);
            std::uninitialized_copy(first, last, new_begin);
            m_begin = new_begin;
        } else if (pos.m_current == m_end.m_current) {
            // Insert at end
            const auto new_end = reallocSubarray(n, Side::BACK);
            std::uninitialized_copy(first, last, m_end);
            m_end = new_end;
        } else {
            // TODO insert values in the middle
            throw std::runtime_error {"Insert values in the middle is not implemented yet."};
        }

        return pos.fromConst();
    }

    iterator insert(const_iterator pos, std::initializer_list<value_type> values)
    {
        return insert(pos, values.begin(), values.end());
    }

    template <typename... Args>
    iterator emplace(const_iterator pos, Args&&... args)
    {
        if (pos.m_current == m_end.m_current) {
            emplace_back(std::forward<Args>(args)...);
            return m_end - 1;
        }

        if (pos.m_current == m_begin.m_current) {
            emplace_front(std::forward<Args>(args)...);
            return m_begin;
        }

        // TODO Implement emplace in the middle
        throw std::runtime_error {"Insert value in the middle is not implemented yet."};
    }

    iterator erase(const_iterator /*pos*/)
    {
        // TODO implement erase(pos)
        throw std::runtime_error {"erase value in any position is not implemented yet."};
    }

    iterator erase(const_iterator /*first*/, const_iterator /*last*/)
    {
        // TODO implement erase(first, last)
        throw std::runtime_error {"erase values in a range is not implemented yet."};
    }

    void push_back(const value_type& v)
    {
        emplace_back(v);
    }

    void push_back(value_type&& v)
    {
        emplace_back(std::move(v));
    }

    template <typename... Args>
    void emplace_back(Args&&... args)
    {
        if (std::next(m_end.m_current) != m_end.subEnd()) {
            // If we have room in the last subarray
            m_end.subarray()->construct(m_end.m_current++, std::forward<Args>(args)...);
        } else {
            // We need to make a copy because args may come from this container and
            // operation below may change the container
            value_type tmp {std::forward<Args>(args)...};

            // If we are in the last subarray, then we are going to need more
            if (m_end.m_subarray - m_ptr_array.data() + 1 >= ptrArraySize()) {
                reallocPtrArray(1, Side::BACK);
            }

            *std::next(m_end.m_subarray) = makeSubarray();

            m_end.subarray()->construct(m_end.m_current, std::move(tmp));
            m_end.setSubarray(std::next(m_end.m_subarray));
            m_end.m_current = m_end.subBegin();
        }
    }

    void pop_back()
    {
        if (m_end.m_current != m_end.subBegin()) {
            (--m_end.m_current)->~value_type();
        } else {
            (m_end.m_subarray--)->reset();
            m_end.m_current = std::prev(m_end.subEnd());
            m_end.m_current->~value_type();
        }
    }

    void push_front(const value_type& v)
    {
        emplace_front(v);
    }

    void push_front(value_type&& v)
    {
        emplace_front(std::move(v));
    }

    template <typename... Args>
    void emplace_front(Args&&... args)
    {
        if (m_begin.m_current != m_begin.subBegin()) {
            // If we have room in the last subarray
            m_begin.subarray()->construct(--m_begin.m_current, std::forward<Args>(args)...);
        } else {
            // We need to make a copy because args may come from this container and
            // operation below may change the container
            value_type tmp {std::forward<Args>(args)...};

            // If we are in the first subarray, then we are going to need more
            if (m_begin.m_subarray == m_ptr_array.data()) {
                reallocPtrArray(1, Side::FRONT);
            }

            *std::prev(m_begin.m_subarray) = makeSubarray();

            m_begin.setSubarray(std::prev(m_begin.m_subarray));
            m_begin.m_current = std::prev(m_begin.subEnd());
            m_begin.subarray()->construct(m_begin.m_current, std::move(tmp));
        }
    }

    void pop_front()
    {
        if (std::next(m_begin.m_current) != m_begin.subEnd()) {
            (m_begin.m_current++)->~value_type();
        } else {
            m_begin.m_current->~value_type();
            (m_begin.m_subarray++)->reset();
            m_begin.m_current = m_begin.subBegin();
        }
    }

    void resize(size_type n)
    {
        resize(n, value_type {});
    }

    void resize(size_type n, const value_type& v)
    {
        const auto old_size = size();
        if (n < old_size) {
            erase(std::next(m_begin, n), m_end);
        } else {
            insert(m_end, n - old_size, v);
        }
    }

    void swap(this_type& rhs)
    {
        std::swap(m_ptr_array, rhs.m_ptr_array);
        std::swap(m_begin, rhs.m_begin);
        std::swap(m_end, rhs.m_end);
    }

protected:
    SubarrayPtr makeSubarray()
    {
        return std::make_unique<Subarray>();
    }

    void resizePtrArray(size_type n)
    {
        m_ptr_array.resize(n);
        m_ptr_array.shrink_to_fit();
    }

    void reallocPtrArray(size_type ptr_count, Side side)
    {
        const auto num_unused_ptr_front  = std::distance(m_ptr_array.data(), m_begin.m_subarray);
        const auto num_used_ptr          = std::distance(m_begin.m_subarray, m_end.m_subarray + 1);
        const auto num_unused_ptr_back   = (ptrArraySize() - num_unused_ptr_front) - num_used_ptr;
        SubarrayPtr* new_ptr_array_begin = nullptr;

        // If we have enough unused pointers, we could just move them around (Use memmove to take care of overlap)
        if (side == Side::BACK && ptr_count <= num_unused_ptr_front) {
            // If there's a lot of unused pointers at front, move at least half of them
            ptr_count = std::max(ptr_count, num_unused_ptr_front / 2);

            // Move pointers to the left
            new_ptr_array_begin = std::prev(m_begin.m_subarray, ptr_count);
            std::move(m_begin.m_subarray, m_end.m_subarray + 1, new_ptr_array_begin);
        } else if (side == Side::FRONT && ptr_count <= num_unused_ptr_back) {
            // If there's a lot of unused pointers at back, move at least half of them
            ptr_count = std::max(ptr_count, num_unused_ptr_back / 2);

            // Move pointers to the right
            new_ptr_array_begin = std::next(m_begin.m_subarray, ptr_count);
            std::move_backward(m_begin.m_subarray, m_end.m_subarray + 1, std::next(new_ptr_array_begin, num_used_ptr));
        } else {
            // If we really need more subarrays, double the ptr_array capacity or allocate more if needed
            const auto old_ptr_array_size = ptrArraySize();
            const auto new_ptr_array_size = old_ptr_array_size + std::max(old_ptr_array_size, ptr_count);
            resizePtrArray(new_ptr_array_size);

            new_ptr_array_begin = std::next(m_ptr_array.data(), num_unused_ptr_front +
                (side == Side::FRONT ? ptr_count : 0));
            std::move_backward(m_begin.m_subarray, m_end.m_subarray + 1, std::next(new_ptr_array_begin, num_used_ptr));
        }

        // Reset the begin and end iterators
        m_begin.setSubarray(new_ptr_array_begin);
        m_end.setSubarray(std::next(new_ptr_array_begin, num_used_ptr - 1));
    }

    iterator reallocSubarray(size_type capacity, Side side)
    {
        if (side == Side::FRONT) {
            const auto front_capacity = std::distance(m_begin.subBegin(), m_begin.m_current);
            if (front_capacity < capacity) {
                const auto num_subarray_needed = ((capacity - front_capacity) + SUBARRAY_SIZE - 1) / SUBARRAY_SIZE;

                // If there are not enough subarray before
                const auto num_subarray_avail = std::distance(m_ptr_array.data(), m_begin.m_subarray);
                if(num_subarray_needed > num_subarray_avail) {
                    reallocPtrArray(num_subarray_needed - num_subarray_avail, Side::FRONT);
                }

                for(size_type i = 1; i <= num_subarray_needed; ++i) {
                    m_begin.m_subarray[-i] = makeSubarray();
                }
            }

            return m_begin - capacity;
        } else {
            const auto back_capacity = std::distance(m_end.m_current, std::prev(m_end.subEnd()));
            if (back_capacity < capacity) {
                const auto num_subarray_needed = ((capacity - back_capacity) + SUBARRAY_SIZE - 1) / SUBARRAY_SIZE;

                // If there are not enough pointers at back
                const auto num_ptrs_avail = ((m_ptr_array.data() + ptrArraySize()) - m_end.m_subarray) - 1;
                if(num_subarray_needed > num_ptrs_avail) {
                    reallocPtrArray(num_subarray_needed - num_ptrs_avail, Side::BACK);
                }

                for(size_type i = 1; i <= num_subarray_needed; ++i) {
                    m_end.m_subarray[i] = makeSubarray();
                }
            }

            return m_end + capacity;
        }
    }

    void destruct(iterator first, iterator last)
    {
        while (first != last) {
            (first++)->~value_type();
        }
    }

    void init(size_type n)
    {
        const auto new_ptr_array_size = (n / SUBARRAY_SIZE) + 1;

        // Reserve at least 1 uninitialized subarray ptr at both front and end
        const auto reserve_ptr_array_size = std::max(MIN_PTR_ARRAY_SIZE, new_ptr_array_size + 2);
        resizePtrArray(reserve_ptr_array_size);

        auto offset = (reserve_ptr_array_size - new_ptr_array_size) / 2;
        auto ptr_array_view = makeView(m_ptr_array, offset, new_ptr_array_size);
        for (auto& subarray : ptr_array_view) {
            subarray = makeSubarray();
        }

        m_begin.setSubarray(ptr_array_view.data());
        m_begin.m_current = m_begin.subBegin();

        m_end.setSubarray(ptr_array_view.data() + ptr_array_view.size() - 1);
        m_end.m_current = std::next(m_end.subBegin(), n % SUBARRAY_SIZE);
    }

    void initWithValue(const value_type& value)
    {
        for (auto& subarray : makeView(m_begin.m_subarray, m_end.m_subarray)) {
            std::uninitialized_fill(subarray->begin(), subarray->end(), value);
        }
        std::uninitialized_fill(m_end.subBegin(), m_end.m_current, value);
    }

    template <typename Iterator, typename = std::enable_if_t<is_input_iterator<Iterator>::value>>
    void initWithIterator(Iterator first, Iterator last)
    {
        initWithIterator(first, last, typename std::iterator_traits<Iterator>::iterator_category {});
    }

    // iterator may be invalidated after one pass, so we can't use std::distance in this case
    template <typename InputIter>
    void initWithIterator(InputIter first, InputIter last, std::input_iterator_tag)
    {
        init(0);
        while (first != last) {
            push_back(*first++);
        }
    }


    template <typename ForwardIterator>
    void initWithIterator(ForwardIterator first, ForwardIterator last, std::forward_iterator_tag)
    {
        init(std::distance(first, last));
        for (auto& subarray : makeView(m_begin.m_subarray, m_end.m_subarray)) {
            auto next = std::next(first, SUBARRAY_SIZE);
            std::uninitialized_copy(first, next, subarray->begin());
            first = next;
        }

        std::uninitialized_copy(first, last, m_end.subBegin());
    }

    size_type ptrArraySize() const
    {
        return static_cast<size_type>(m_ptr_array.size());
    }

protected:
    // Array of pointers to Subarrays
    std::vector<SubarrayPtr, STLAllocator<SubarrayPtr>> m_ptr_array;
    iterator m_begin;
    iterator m_end;
};

}   // namespace detail

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Deque
template <typename T, size_type SUBARRAY_SIZE = detail::DEFAULT_SUBARRAY_SIZE<T>>
class Deque : public detail::DequeImpl<T, SUBARRAY_SIZE> {
public:
    using this_type = Deque<T, SUBARRAY_SIZE>;
    using base_type = detail::DequeImpl<T, SUBARRAY_SIZE>;
    using typename base_type::value_type;
    using typename base_type::pointer;
    using typename base_type::const_pointer;
    using typename base_type::reference;
    using typename base_type::const_reference;
    using typename base_type::iterator;
    using typename base_type::const_iterator;
    using typename base_type::reverse_iterator;
    using typename base_type::const_reverse_iterator;
    using typename base_type::difference_type;

    Deque() : base_type {0} {}
    Deque(size_type n) : base_type {n} {}

    Deque(size_type n, const value_type& value) : base_type {n}
    {
        this->initWithValue(value);
    }

    template <typename InputIter>
    Deque(InputIter first, InputIter last)
    {
        this->initWithIterator(first, last);
    }

    Deque(std::initializer_list<value_type> values)
    {
        this->initWithIterator(values.begin(), values.end());
    }

    Deque(const this_type& rhs) : base_type { rhs.size() }
    {
        std::uninitialized_copy(rhs.m_begin, rhs.m_end, this->m_begin);
    }

    Deque(this_type&& rhs) : base_type {0}
    {
        swap(rhs);
    }

    this_type& operator=(const this_type& rhs)
    {
        if (&rhs != this) {
            this->assign(rhs.m_begin, rhs.m_end);
        }

        return *this;
    }

    this_type& operator=(this_type&& rhs)
    {
        if (&rhs != this) {
            this->swap(rhs);
        }

        return *this;
    }

    this_type& operator=(std::initializer_list<value_type> rhs)
    {
        this->assign(rhs.begin(), rhs.end());
    }
};
CLS_END
