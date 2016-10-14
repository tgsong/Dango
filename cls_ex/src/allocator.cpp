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

#include <cls_ex/allocator.h>

CLS_BEGIN
// Default active allocator
std::unique_ptr<Allocator> ActiveAllocator::m_allocator = std::make_unique<DefaultAllocator>();

Allocator::~Allocator() = default;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DefaultAllocator
void* DefaultAllocator::allocate(size_type n)
{
    return boost::alignment::aligned_alloc(static_cast<size_t>(MIN_ALIGNMENT), static_cast<size_t>(n));
}

void* DefaultAllocator::allocate(size_type n, size_type alignment)
{
    return boost::alignment::aligned_alloc(
        static_cast<size_t>(std::max(MIN_ALIGNMENT, alignment)), static_cast<size_t>(n));
}

void DefaultAllocator::deallocate(void* p, size_type)
{
    boost::alignment::aligned_free(p);
}
CLS_END
