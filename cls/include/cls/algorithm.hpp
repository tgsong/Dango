/////////////////////////////////////////////////////////////////////////////////
// The MIT License(MIT)
//
// Copyright (c) 2014 Tiangang Song
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

#ifndef CLS_ALGORITHM_HPP
#define CLS_ALGORITHM_HPP

#include <algorithm>
#include <numeric>
#include <functional>
#include "traits.hpp"

CLS_BEGIN
//////////////////////////////////////////////////////////////////////////////////////////
// Bring all overloaded std functions to current namespace
using std::all_of;
using std::any_of;
using std::none_of;
using std::for_each;
using std::count;
using std::count_if;
using std::mismatch;
using std::equal;
using std::find;
using std::find_if;
using std::find_if_not;
using std::find_end;
using std::find_first_of;
using std::adjacent_find;
using std::search;
using std::search_n;
using std::copy;
using std::copy_if;
using std::copy_backward;
using std::move;
using std::move_backward;
using std::fill;
using std::transform;
using std::generate;
using std::remove;
using std::remove_if;
using std::remove_copy;
using std::remove_copy_if;
using std::replace;
using std::replace_if;
using std::replace_copy;
using std::replace_copy_if;
using std::reverse;
using std::reverse_copy;
using std::rotate;
using std::rotate_copy;
using std::shuffle;
using std::unique;
using std::unique_copy;
using std::is_sorted;
using std::is_sorted_until;
using std::sort;
using std::partial_sort;
using std::max_element;
using std::min_element;
using std::minmax_element;
using std::iota;
using std::accumulate;
using std::inner_product;

//////////////////////////////////////////////////////////////////////////////////////////
// Helper function return the size of a container
template<typename Container,
         typename U = enable_if_t<is_container<Container>::value>>
inline auto container_size(Container&& container) -> decltype(container.size())
{
    return container.size();
}

template<typename T, size_t N>
inline size_t container_size(T(&)[N])
{
    return N;
}

// Helpler function that can print contents of a STL container
template<typename T,
         template<typename Elem, typename Alloc = std::allocator<Elem>> class Container,
         typename U = enable_if_t<is_container<Container<T>>::value>>
inline std::ostream& operator<<(std::ostream& os, const Container<T>& c)
{
    os << "[";
    auto first = std::begin(c);
    auto last  = next(first, std::distance(first, std::end(c)) - 1);
    while (first != last) {
        os << *first++ << ", ";
    }
    os << *last << "]";

    return os;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Non-modifying sequence operations
template<typename Container, typename Func,
         typename U = enable_if_t<is_container<Container>::value>>
inline bool all_of(Container&& container, Func func)
{
    return all_of(std::begin(container), std::end(container), func);
}

template<typename Container, typename Func,
         typename U = enable_if_t<is_container<Container>::value>>
inline bool any_of(Container&& container, Func func)
{
    return any_of(std::begin(container), std::end(container), func);
}

template<typename Container, typename Func,
         typename U = enable_if_t<is_container<Container>::value>>
inline bool none_of(Container&& container, Func func)
{
    return none_of(std::begin(container), std::end(container), func);
}

template<typename Container, typename Func,
         typename U = enable_if_t<is_container<Container>::value>>
inline void for_each(Container&& container, Func func)
{
    for_each(std::begin(container), std::end(container), func);
}

// Initializer list overload
template<typename T, typename Func>
inline void for_each(std::initializer_list<T>&& container, Func func)
{
    for_each(std::begin(container), std::end(container), func);
}

template<typename Container, typename T,
         typename U = enable_if_t<is_container<Container>::value>>
inline auto count(Container&& container, const T& value) ->
iterator_difference_t<decltype(std::begin(container))>
{
    return count(std::begin(container), std::end(container), value);
}

template<typename Container, typename UPred,
         typename U = enable_if_t<is_container<Container>::value>>
inline auto count_if(Container&& container, UPred p) ->
iterator_difference_t<decltype(std::begin(container))>
{
    return count_if(std::begin(container), std::end(container), p);
}

template<typename Container1, typename Container2,
         typename U = enable_if_t<is_container<Container1>::value &&
                                  is_container<Container2>::value>>
inline auto mismatch(Container1& container1, Container2& container2) ->
std::pair<decltype(std::begin(container1)), decltype(std::begin(container2))>
{
    return mismatch(std::begin(container1), std::end(container1), std::begin(container2));
}

template<typename Container1, typename Container2, typename BPred,
         typename U = enable_if_t<is_container<Container1>::value &&
                                  is_container<Container2>::value>>
inline auto mismatch(Container1& container1, Container2& container2, BPred p) ->
std::pair<decltype(std::begin(container1)), decltype(std::begin(container2))>
{
    return mismatch(std::begin(container1), std::end(container1), std::begin(container2), p);
}

template<typename Container1, typename Container2,
         typename U = enable_if_t<is_container<Container1>::value &&
                                  is_container<Container2>::value>>
inline bool equal(Container1&& container1, Container2&& container2)
{
    return equal(std::begin(container1), std::end(container1),std::begin(container2));
}

template<typename Container1, typename Container2, typename BPred,
         typename U = enable_if_t<is_container<Container1>::value &&
                                  is_container<Container2>::value>>
inline bool equal(Container1&& container1, Container2&& container2, BPred p)
{
    return equal(std::begin(container1), std::end(container1), std::begin(container2), p);
}

// Initializer list overload
template<typename T, typename Container,
         typename U = enable_if_t<is_container<Container>::value>>
inline bool equal(Container&& container1, std::initializer_list<T>&& container2)
{
    return equal(std::begin(container1), std::end(container1),std::begin(container2));
}

template<typename T, typename Container,
         typename U = enable_if_t<is_container<Container>::value>>
inline bool equal(std::initializer_list<T>&& container1, Container&& container2)
{
    return equal(std::begin(container1), std::end(container1),std::begin(container2));
}

template<typename T, typename Container, typename BPred,
         typename U = enable_if_t<is_container<Container>::value>>
inline bool equal(Container&& container1, std::initializer_list<T>&& container2, BPred p)
{
    return equal(std::begin(container1), std::end(container1), std::begin(container2), p);
}

template<typename T, typename Container, typename BPred,
         typename U = enable_if_t<is_container<Container>::value>>
inline bool equal(std::initializer_list<T>&& container1, Container&& container2, BPred p)
{
    return equal(std::begin(container1), std::end(container1), std::begin(container2), p);
}

template<typename Container, typename T,
         typename U = enable_if_t<is_container<Container>::value>>
inline auto find(Container& container, const T& value) -> decltype(std::begin(container))
{
    return find(std::begin(container), std::end(container), value);
}

template<typename Container, typename UPred,
         typename U = enable_if_t<is_container<Container>::value>>
inline auto find_if(Container& container, UPred p) -> decltype(std::begin(container))
{
    return find_if(std::begin(container), std::end(container), p);
}

template<typename Container, typename UPred,
         typename U = enable_if_t<is_container<Container>::value>>
inline auto find_if_not(Container& container, UPred p) ->
decltype(std::begin(container))
{
    return find_if_not(std::begin(container), std::end(container), p);
}

template<typename Container1, typename Container2,
         typename U = enable_if_t<is_container<Container1>::value &&
                                  is_container<Container2>::value>>
inline auto find_end(Container1& container1, Container2&& container2) ->
decltype(std::begin(container1))
{
    return find_end(std::begin(container1), std::end(container1),
                    std::begin(container2), std::end(container2));
}

template<typename Container1, typename Container2, typename BPred,
         typename U = enable_if_t<is_container<Container1>::value &&
                                  is_container<Container2>::value>>
inline auto find_end(Container1& container1, Container2&& container2, BPred p) ->
decltype(std::begin(container1))
{
    return find_end(std::begin(container1), std::end(container1),
                    std::begin(container2), std::end(container2), p);
}

// Initializer list overload
template<typename T, typename Container,
         typename U = enable_if_t<is_container<Container>::value>>
inline auto find_end(Container& container1, std::initializer_list<T>&& container2) ->
decltype(std::begin(container1))
{
    return find_end(std::begin(container1), std::end(container1),
                    std::begin(container2), std::end(container2));
}

template<typename T, typename Container, typename BPred,
         typename U = enable_if_t<is_container<Container>::value>>
inline auto find_end(Container& container1, std::initializer_list<T>&& container2, BPred p) ->
decltype(std::begin(container1))
{
    return find_end(std::begin(container1), std::end(container1),
                    std::begin(container2), std::end(container2), p);
}

template<typename Container1, typename Container2,
         typename U = enable_if_t<is_container<Container1>::value &&
                                  is_container<Container2>::value>>
inline auto find_first_of(Container1& container1, Container2&& container2) ->
decltype(std::begin(container1))
{
    return find_first_of(std::begin(container1), std::end(container1),
                         std::begin(container2), std::end(container2));
}

template<typename Container1, typename Container2, typename BPred,
         typename U = enable_if_t<is_container<Container1>::value &&
                                  is_container<Container2>::value>>
inline auto find_first_of(Container1& container1, Container2&& container2, BPred p) ->
decltype(std::begin(container1))
{
    return find_first_of(std::begin(container1), std::end(container1),
                         std::begin(container2), std::end(container2), p);
}

// Initializer list overload
template<typename T, typename Container,
         typename U = enable_if_t<is_container<Container>::value>>
inline auto find_first_of(Container& container1, std::initializer_list<T>&& container2) ->
decltype(std::begin(container1))
{
    return find_first_of(std::begin(container1), std::end(container1),
                         std::begin(container2), std::end(container2));
}

template<typename T, typename Container, typename BPred,
         typename U = enable_if_t<is_container<Container>::value>>
inline auto find_first_of(Container& container1, std::initializer_list<T>&& container2, BPred p) ->
decltype(std::begin(container1))
{
    return find_first_of(std::begin(container1), std::end(container1),
                         std::begin(container2), std::end(container2), p);
}

template<typename Container,
         typename U = enable_if_t<is_container<Container>::value>>
inline auto adjacent_find(Container& container) -> decltype(std::begin(container))
{
    return adjacent_find(std::begin(container), std::end(container));
}

template<typename Container, typename BPred,
         typename U = enable_if_t<is_container<Container>::value>>
inline auto adjacent_find(Container& container, BPred p) -> decltype(std::begin(container))
{
    return adjacent_find(std::begin(container), std::end(container), p);
}

template<typename Container1, typename Container2,
         typename U = enable_if_t<is_container<Container1>::value &&
                                  is_container<Container2>::value>>
inline auto search(Container1& container1, Container2&& container2) ->
decltype(std::begin(container1))
{
    return search(std::begin(container1), std::end(container1),
                  std::begin(container2), std::end(container2));
}

template<typename Container1, typename Container2, typename BPred,
         typename U = enable_if_t<is_container<Container1>::value &&
                                  is_container<Container2>::value>>
inline auto search(Container1& container1, Container2&& container2, BPred p) ->
decltype(std::begin(container1))
{
    return search(std::begin(container1), std::end(container1),
                  std::begin(container2), std::end(container2), p);
}

// Initializer list overload
template<typename T, typename Container,
         typename U = enable_if_t<is_container<Container>::value>>
inline auto search(Container& container1, std::initializer_list<T>&& container2) ->
decltype(std::begin(container1))
{
    return search(std::begin(container1), std::end(container1),
                  std::begin(container2), std::end(container2));
}

template<typename T, typename Container, typename BPred,
         typename U = enable_if_t<is_container<Container>::value>>
inline auto search(Container& container1, std::initializer_list<T>&& container2, BPred p) ->
decltype(std::begin(container1))
{
    return search(std::begin(container1), std::end(container1),
                  std::begin(container2), std::end(container2), p);
}

template<typename Container, typename Size, typename T,
         typename U = enable_if_t<is_container<Container>::value>>
inline auto search_n(Container& container, Size count, const T& value) ->
decltype(std::begin(container))
{
    return search_n(std::begin(container), std::end(container), count, value);
}

template<typename Container, typename Size, typename T, typename BPred,
         typename U = enable_if_t<is_container<Container>::value>>
inline auto search_n(Container& container, Size count, const T& value, BPred p) ->
decltype(std::begin(container))
{
    return search_n(std::begin(container), std::end(container), count, value, p);
}
//////////////////////////////////////////////////////////////////////////////////////////
// Modifying sequence operations
// Container to container, automatically resize
template<typename Container1, typename Container2,
         typename U = enable_if_t<is_container<Container1>::value &&
                                  is_container<Container2>::value>>
inline void copy(Container1&& container1, Container2& container2)
{
    container2.resize(container_size(container1));
    copy(std::begin(container1), std::end(container1), std::begin(container2));
}

// Container to output iterator
template<typename Container, typename OutputIt,
         typename U = enable_if_t<is_container<Container>::value &&
                                  is_output_iterator<OutputIt>::value>>
inline auto copy(Container&& container, OutputIt d_first) -> OutputIt
{
    return copy(std::begin(container), std::end(container), d_first);
}

// Initializer list to output iterator
template<typename T, typename OutputIt,
         typename U = enable_if_t<is_output_iterator<OutputIt>::value>>
inline auto copy(std::initializer_list<T>&& container, OutputIt d_first) -> OutputIt
{
    return copy(std::begin(container), std::end(container), d_first);
}

// Container to container, automatically resize
template<typename Container1, typename Container2, typename UPred,
         typename U = enable_if_t<is_container<Container1>::value &&
                                  is_container<Container2>::value>>
inline void copy_if(Container1&& container1, Container2& container2, UPred p)
{
    container2.resize(container_size(container1));
    auto iter = copy_if(std::begin(container1), std::end(container1),
                        std::begin(container2), p);
    container2.resize(static_cast<decltype(container2.size())>(std::distance(std::begin(container2), iter)));
}

// Container to output iterator
template<typename Container, typename OutputIt, typename UPred,
         typename U = enable_if_t<is_container<Container>::value &&
                                  is_output_iterator<OutputIt>::value>>
inline auto copy_if(Container&& container, OutputIt d_first, UPred p) -> OutputIt
{
    return copy_if(std::begin(container), std::end(container), d_first, p);
}

// Container to output iterator
template<typename Container, typename OutputIt,
         typename U = enable_if_t<is_container<Container>::value &&
                                  is_output_iterator<OutputIt>::value>>
inline auto copy_backward(Container&& container, OutputIt d_first) -> OutputIt
{
    return copy_backward(std::begin(container), std::end(container), d_first);
}

// Initializer list to output iterator
template<typename T, typename OutputIt,
         typename U = enable_if_t<is_output_iterator<OutputIt>::value>>
inline auto copy_backward(std::initializer_list<T>&& container, OutputIt d_first) -> OutputIt
{
    return copy_backward(std::begin(container), std::end(container), d_first);
}

// Container to container, automatically resize
template<typename Container1, typename Container2,
         typename U = enable_if_t<is_container<Container1>::value &&
                                  is_container<Container2>::value>>
inline void move(Container1&& container1, Container2& container2)
{
    container2.resize(container_size(container1));
    move(std::begin(container1), std::end(container1), std::begin(container2));
}

// Container to output iterator
template<typename Container, typename OutputIt,
         typename U = enable_if_t<is_container<Container>::value &&
                                  is_output_iterator<OutputIt>::value>>
inline auto move(Container&& container, OutputIt d_first) -> OutputIt
{
    return move(std::begin(container), std::end(container), d_first);
}

// Container to output iterator
template<typename Container, typename OutputIt,
         typename U = enable_if_t<is_container<Container>::value &&
                                  is_output_iterator<OutputIt>::value>>
inline auto move_backward(Container&& container, OutputIt d_first) -> OutputIt
{
    return move_backward(std::begin(container), std::end(container), d_first);
}

template<typename Container, typename T,
         typename U = enable_if_t<is_container<Container>::value>>
inline void fill(Container& container, const T& value)
{
    fill(std::begin(container), std::end(container), value);
}

// Container to container, automatically resize
template<typename Container1, typename Container2, typename UPred,
         typename U = enable_if_t<is_container<Container1>::value &&
                                  is_container<Container2>::value>>
inline void transform(Container1&& container1, Container2& container2, UPred p)
{
    container2.resize(container_size(container1));
    transform(std::begin(container1), std::end(container1), std::begin(container2), p);
}

// Container to output iterator
template<typename Container, typename OutputIt, typename UPred,
         typename U = enable_if_t<is_container<Container>::value &&
                                  is_output_iterator<OutputIt>::value>>
inline auto transform(Container&& container, OutputIt d_first, UPred p) -> OutputIt
{
    return transform(std::begin(container), std::end(container), d_first, p);
}

// Two containers to output iterator
template<typename Container1, typename Container2, typename OutputIt, typename UPred,
         typename U = enable_if_t<is_container<Container1>::value &&
                                  is_container<Container2>::value &&
                                  is_output_iterator<OutputIt>::value>>
inline auto transform(Container1&& container1, Container2&& container2,
                      OutputIt d_first, UPred p) -> OutputIt
{
    return transform(std::begin(container1), std::end(container1), std::begin(container2),
                     d_first, p);
}

// Initializer list to container, automatically resize
template<typename T, typename Container, typename UPred,
         typename U = enable_if_t<is_container<Container>::value>>
inline void transform(std::initializer_list<T>&& container1, Container& container2, UPred p)
{
    container2.resize(container_size(container1));
    transform(std::begin(container1), std::end(container1), std::begin(container2), p);
}

// Initializer list to output iterator
template<typename T, typename OutputIt, typename UPred,
         typename U = enable_if_t<is_output_iterator<OutputIt>::value>>
inline auto transform(std::initializer_list<T>&& container, OutputIt d_first,
                      UPred p) -> OutputIt
{
    return transform(std::begin(container), std::end(container), d_first, p);
}

template<typename Container, typename Generator,
         typename U = enable_if_t<is_container<Container>::value>>
inline void generate(Container& container, Generator&& g)
{
    generate(std::begin(container), std::end(container), std::forward<Generator>(g));
}

// Automatically resize after removal
template<typename Container, typename T,
         typename U = enable_if_t<is_container<Container>::value>>
inline void remove(Container& container, const T& value)
{
    auto iter = remove(std::begin(container), std::end(container), value);
    container.resize(std::distance(std::begin(container), iter));
}

// Automatically resize after removal
template<typename Container, typename UPred,
         typename U = enable_if_t<is_container<Container>::value>>
inline void remove_if(Container& container, UPred p)
{
    auto iter = remove_if(std::begin(container), std::end(container), p);
    container.resize(std::distance(std::begin(container), iter));
}

// Container to container, automatically resize
template<typename Container1, typename Container2, typename T,
         typename U = enable_if_t<is_container<Container1>::value &&
                                  is_container<Container2>::value>>
inline void remove_copy(Container1&& container1, Container2& container2, const T& value)
{
    container2.resize(container_size(container1));
    auto iter = remove_copy(std::begin(container1), std::end(container1),
                            std::begin(container2), value);
    container2.resize(std::distance(std::begin(container2), iter));
}

template<typename Container1, typename Container2, typename UPred,
         typename U = enable_if_t<is_container<Container1>::value &&
                                  is_container<Container2>::value>>
inline void remove_copy_if(Container1&& container1, Container2& container2, UPred p)
{
    container2.resize(container_size(container1));
    auto iter = remove_copy_if(std::begin(container1), std::end(container1),
                               std::begin(container2), p);
    container2.resize(static_cast<decltype(container2.size())>(std::distance(std::begin(container2), iter)));
}

template<typename Container, typename T,
         typename U = enable_if_t<is_container<Container>::value>>
inline void replace(Container& container, const T& old_value, const T& new_value)
{
    replace(std::begin(container), std::end(container), old_value, new_value);
}

template<typename Container, typename UPred, typename T,
         typename U = enable_if_t<is_container<Container>::value>>
inline void replace_if(Container& container, UPred p, const T& new_value)
{
    replace_if(std::begin(container), std::end(container), p, new_value);
}

// Container to container, automatically resize
template<typename Container1, typename Container2, typename T,
         typename U = enable_if_t<is_container<Container1>::value &&
                                  is_container<Container2>::value>>
inline void replace_copy(Container1&& container1, Container2& container2,
                         const T& old_value, const T& new_value)
{
    container2.resize(container_size(container1));
    auto iter = replace_copy(std::begin(container1), std::end(container1),
                             std::begin(container2), old_value, new_value);
    container2.resize(std::distance(std::begin(container2), iter));
}

// Container to output iterator
template<typename Container, typename OutputIt, typename T,
         typename U = enable_if_t<is_container<Container>::value &&
                                  is_output_iterator<OutputIt>::value>>
inline auto replace_copy(Container&& container, OutputIt d_first,
                         const T& old_value, const T& new_value) -> OutputIt
{
    return replace_copy(std::begin(container), std::end(container),
                        d_first, old_value, new_value);
}

// Container to container, automatically resize
template<typename Container1, typename Container2, typename UPred, typename T,
         typename U = enable_if_t<is_container<Container1>::value &&
                                  is_container<Container2>::value>>
inline void replace_copy_if(Container1&& container1, Container2& container2,
                            UPred p, const T& new_value)
{
    container2.resize(container_size(container1));
    auto iter = replace_copy_if(std::begin(container1), std::end(container1),
                                std::begin(container2), p, new_value);
    container2.resize(std::distance(std::begin(container2), iter));
}

// Container to output iterator
template<typename Container, typename OutputIt, typename UPred, typename T,
         typename U = enable_if_t<is_container<Container>::value &&
                                  is_output_iterator<OutputIt>::value>>
inline auto replace_copy_if(Container&& container, OutputIt d_first,
                            UPred p, const T& new_value) -> OutputIt
{
    return replace_copy_if(std::begin(container), std::end(container),
                           d_first, p, new_value);
}

template<typename Container,
         typename U = enable_if_t<is_container<Container>::value>>
inline void reverse(Container& container)
{
    reverse(std::begin(container), std::end(container));
}

// Container to container, automatically resize
template<typename Container1, typename Container2,
         typename U = enable_if_t<is_container<Container1>::value &&
                                  is_container<Container2>::value>>
inline void reverse_copy(Container1&& container1, Container2& container2)
{
    container2.resize(container_size(container1));
    reverse_copy(std::begin(container1), std::end(container1), std::begin(container2));
}

// Container to output iterator
template<typename Container, typename OutputIt,
         typename U = enable_if_t<is_container<Container>::value &&
                                  is_output_iterator<OutputIt>::value>>
inline auto reverse_copy(Container&& container, OutputIt d_first) -> OutputIt
{
    return reverse_copy(std::begin(container), std::end(container), d_first);
}

template<typename Container, typename Pos,
         typename U = enable_if_t<is_container<Container>::value>>
inline auto rotate(Container& container, Pos pos) -> decltype(std::begin(container))
{
    auto mid = std::begin(container);
    std::advance(mid, pos);
    return rotate(std::begin(container), mid, std::end(container));
}

// Container to container, automatically resize
template<typename Container1, typename Container2, typename Pos,
         typename U = enable_if_t<is_container<Container1>::value &&
                                  is_container<Container2>::value>>
inline void rotate_copy(Container1&& container1, Pos pos, Container2& container2)
{
    container2.resize(container_size(container1));
    auto mid = std::begin(container1);
    std::advance(mid, pos);
    rotate_copy(std::begin(container1), mid, std::end(container1), std::begin(container2));
}

// Container to output iterator
template<typename Container, typename OutputIt, typename Pos,
         typename U = enable_if_t<is_container<Container>::value &&
                                  is_output_iterator<OutputIt>::value>>
inline auto rotate_copy(Container&& container, Pos pos, OutputIt d_first) -> OutputIt
{
    auto mid = std::begin(container);
    std::advance(mid, pos);
    return rotate_copy(std::begin(container), mid, std::end(container), d_first);
}

template<typename Container, typename URNG,
         typename U = enable_if_t<is_container<Container>::value>>
inline void shuffle(Container& container, URNG&& g)
{
    shuffle(std::begin(container), std::end(container), std::forward<URNG>(g));
}

// Automatically resize
template<typename Container,
         typename U = enable_if_t<is_container<Container>::value>>
inline void unique(Container& container)
{
    auto iter = unique(std::begin(container), std::end(container));
    container.resize(std::distance(std::begin(container), iter));
}

// Automatically resize
template<typename Container, typename BPred,
         typename U = enable_if_t<is_container<Container>::value>>
inline void unique(Container& container, BPred p)
{
    auto iter = unique(std::begin(container), std::end(container), p);
    container.resize(std::distance(std::begin(container), iter));
}

// Container to container, automatically resize
template<typename Container1, typename Container2,
         typename U = enable_if_t<is_container<Container1>::value &&
                                  is_container<Container2>::value>>
inline void unique_copy(Container1&& container1, Container2& container2)
{
    container2.resize(container_size(container1));
    auto iter = unique_copy(std::begin(container1), std::end(container1),
                            std::begin(container2));
    container2.resize(std::distance(std::begin(container2), iter));
}

// Container to container, automatically resize
template<typename Container1, typename Container2, typename BPred,
         typename U = enable_if_t<is_container<Container1>::value &&
                                  is_container<Container2>::value>>
inline void unique_copy(Container1&& container1, Container2& container2, BPred p)
{
    container2.resize(container_size(container1));
    auto iter = unique_copy(std::begin(container1), std::end(container1),
                            std::begin(container2), p);
    container2.resize(std::distance(std::begin(container2), iter));
}
//////////////////////////////////////////////////////////////////////////////////////////
// Partitioning operations

//////////////////////////////////////////////////////////////////////////////////////////
// Sorting operations
template<typename Container,
         typename U = enable_if_t<is_container<Container>::value>>
inline bool is_sorted(Container& container)
{
    return is_sorted(std::begin(container), std::end(container));
}

template<typename Container, typename Comp,
         typename U = enable_if_t<is_container<Container>::value>>
inline bool is_sorted(Container& container, Comp comp)
{
    return is_sorted(std::begin(container), std::end(container), comp);
}

template<typename Container,
         typename U = enable_if_t<is_container<Container>::value>>
inline auto is_sorted_until(Container& container) -> decltype(std::begin(container))
{
    return is_sorted_until(std::begin(container), std::end(container));
}

template<typename Container, typename Comp,
         typename U = enable_if_t<is_container<Container>::value>>
inline auto is_sorted_until(Container& container, Comp comp) -> decltype(std::begin(container))
{
    return is_sorted_until(std::begin(container), std::end(container), comp);
}

template<typename Container,
         typename U = enable_if_t<is_container<Container>::value>>
inline void sort(Container& container)
{
    sort(std::begin(container), std::end(container));
}

template<typename Container, typename Comp,
         typename U = enable_if_t<is_container<Container>::value>>
inline void sort(Container& container, Comp comp)
{
    sort(std::begin(container), std::end(container), comp);
}

template<typename Container, typename Size,
         typename U = enable_if_t<is_container<Container>::value>>
inline void partial_sort(Container& container, Size size)
{
    auto mid = std::begin(container);
    std::advance(mid, size);
    partial_sort(std::begin(container), mid, std::end(container));
}

template<typename Container, typename Size, typename Comp,
         typename U = enable_if_t<is_container<Container>::value>>
inline void partial_sort(Container& container, Size size, Comp comp)
{
    auto mid = std::begin(container);
    std::advance(mid, size);
    partial_sort(std::begin(container), mid, std::end(container), comp);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Binary search operations (on sorted ranges)

//////////////////////////////////////////////////////////////////////////////////////////
// Set operations (on sorted ranges)

//////////////////////////////////////////////////////////////////////////////////////////
// Heap operations

//////////////////////////////////////////////////////////////////////////////////////////
// Minimum/maximum operations
template<typename Container,
         typename U = enable_if_t<is_container<Container>::value>>
inline auto max_element(Container& container) -> decltype(std::begin(container))
{
    return max_element(std::begin(container), std::end(container));
}

template<typename Container, typename Comp,
         typename U = enable_if_t<is_container<Container>::value>>
inline auto max_element(Container& container, Comp comp) -> decltype(std::begin(container))
{
    return max_element(std::begin(container), std::end(container), comp);
}

template<typename Container,
         typename U = enable_if_t<is_container<Container>::value>>
inline auto min_element(Container& container) -> decltype(std::begin(container))
{
    return min_element(std::begin(container), std::end(container));
}

template<typename Container, typename Comp,
         typename U = enable_if_t<is_container<Container>::value>>
inline auto min_element(Container& container, Comp comp) -> decltype(std::begin(container))
{
    return min_element(std::begin(container), std::end(container), comp);
}

template<typename Container,
         typename U = enable_if_t<is_container<Container>::value>>
inline auto minmax_element(Container& container) -> std::pair<decltype(std::begin(container)),
                                                         decltype(std::begin(container))>
{
    return minmax_element(std::begin(container), std::end(container));
}

template<typename Container, typename Comp,
         typename U = enable_if_t<is_container<Container>::value>>
inline auto minmax_element(Container& container, Comp comp) ->
std::pair<decltype(std::begin(container)), decltype(std::begin(container))>
{
    return minmax_element(std::begin(container), std::end(container), comp);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Numeric operations
template<typename Container, typename T,
         typename U = enable_if_t<is_container<Container>::value>>
inline void iota(Container& container, T value)
{
    iota(std::begin(container), std::end(container), value);
}

template<typename Container,
         typename T = container_value_t<Container>,
         typename U = enable_if_t<is_container<Container>::value>>
inline T accumulate(Container&& container)
{
    T init = T();
    return accumulate(std::begin(container), std::end(container), init);
}

template<typename Container, typename BOperator,
         typename T = container_value_t<Container>,
         typename U = enable_if_t<is_container<Container>::value>>
inline T accumulate(Container&& container, BOperator op)
{
    T init = T();
    return accumulate(std::begin(container), std::end(container), init, op);
}

template<typename Container, typename T, typename BOperator,
         typename U = enable_if_t<is_container<Container>::value>>
inline T accumulate(Container&& container, T init, BOperator op)
{
    return accumulate(std::begin(container), std::end(container), init, op);
}

template<typename Container1, typename Container2,
         typename T = decltype(
         std::declval<container_value_t<Container1>>() *
         std::declval<container_value_t<Container2>>()),
         typename U = enable_if_t<is_container<Container1>::value &&
                                  is_container<Container2>::value>>
inline T inner_product(Container1&& container1, Container2&& container2)
{
    T init = T();
    return inner_product(std::begin(container1), std::end(container1),
                         std::begin(container2), init);
}

template<typename Container1, typename Container2,
         typename BOperator1, typename BOperator2,
         typename T = decltype(std::declval<container_value_t<Container1>>() *
                               std::declval<container_value_t<Container2>>()),
         typename U = enable_if_t<is_container<Container1>::value &&
                                  is_container<Container2>::value>>
inline T inner_product(Container1&& container1, Container2&& container2,
                       BOperator1 sum_op, BOperator2 mul_op)
{
    T init = T();
    return inner_product(std::begin(container1), std::end(container1),
                         std::begin(container2), init, sum_op, mul_op);
}

template<typename Container1, typename Container2,
         typename T, typename BOperator1, typename BOperator2,
         typename U = enable_if_t<is_container<Container1>::value &&
                                  is_container<Container2>::value>>
inline T inner_product(Container1&& container1, Container2&& container2,
                       T init, BOperator1 sum_op, BOperator2 mul_op)
{
    return inner_product(std::begin(container1), std::end(container1),
                         std::begin(container2), init, sum_op, mul_op);
}
CLS_END

#endif // CLS_ALGORITHM_HPP
