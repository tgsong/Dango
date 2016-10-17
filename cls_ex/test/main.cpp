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

#include <iostream>
#include <future>
#include <deque>
#include <boost/container/small_vector.hpp>
#include <cls_ex/deque_x.h>

void performance_test()
{
    clock_t start;
    clock_t end;


    // My custom Deque

    std::string str = "hello";
    cls::Deque<std::string, 1 << 10> my_deque;

    start = clock();
    for (size_t i = 0; i < 1e6; i++)
    {
        my_deque.push_back(str);
    }
    my_deque.clear();
    for (size_t i = 0; i < 1e6; i++)
    {
        my_deque.push_front(str);
    }

    end = clock();
    printf("My deque       %f\n", (double(end - start) / 1000));

    // Standard

    std::deque<std::string> std_container;

    start = clock();
    for (size_t i = 0; i < 1e6; i++)
    {
        std_container.push_back(str);
    }
    std_container.clear();
    for (size_t i = 0; i < 1e6; i++)
    {
        std_container.push_front(str);
    }

    end = clock();
    printf("Standard %f\n", (double(end - start) / 1000));
}

int main(/*int argc, char* argv[]*/)
{
    performance_test();

    return 0;
}
