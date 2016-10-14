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

#ifndef CLS_TIMER_HPP
#define CLS_TIMER_HPP

#include <array>
#include <chrono>
#include <mutex>
#include <cstdio>
#include <cmath>
#include "string.hpp"


CLS_BEGIN
namespace detail {
using high_res_clock = std::chrono::high_resolution_clock;

const std::array<const char*, 4> suffixes = { "ns", "us", "ms", "s" };
const int clock_unit = 1000;

inline void printDuration(const high_res_clock::duration& t, int precision = 0)
{
    auto count = std::chrono::duration_cast<std::chrono::nanoseconds>(t).count();
    size_t exp = 0;
    if (count)
        exp = std::min(static_cast<size_t>(std::log(count) / std::log(clock_unit)), suffixes.size() - 1);

    auto count_num = count / std::pow(clock_unit, exp);
    auto count_unit = suffixes[exp];

    printf("%.*f%s", precision, count_num, count_unit);
}

inline std::string formatDuration(const std::chrono::milliseconds& t, int precision = 0)
{
    auto count = std::chrono::duration_cast<std::chrono::nanoseconds>(t).count();
    size_t exp = 0;
    if (count)
        exp = std::min(static_cast<size_t>(std::log(count) / std::log(clock_unit)), suffixes.size() - 1);

    auto count_num = count / std::pow(clock_unit, exp);
    auto count_unit = suffixes[exp];

    return format("%.*f%s", precision, count_num, count_unit);
}
}

class ScopeTimer {
    using sys_clock = std::chrono::system_clock;
    using seconds   = std::chrono::duration<double>;

public:
    explicit ScopeTimer(int precision = 3)
        :start(sys_clock::now()), prec(precision) {}

    ~ScopeTimer()
    {
        seconds delta = sys_clock::now() - start;
        printf("Time elapsed: %.*fs\n", prec, delta.count());
    }

private:
    sys_clock::time_point start;
    int prec;
};

class CPUTimer {
    using sys_clock = std::chrono::system_clock;
    using seconds   = std::chrono::duration<double>;

public:
    explicit CPUTimer(bool is_started = true)
        : start_point(), since_begin(seconds::zero()),
          is_stopped(false), need_print(true)
    {
        if (is_started) start();
    }

    double elapsed(const std::string& title = "Since begin", int precision = 3)
    {
        if (is_stopped) {
            printf("Paused\n");
            return -1;
        }

        auto now     = sys_clock::now();
        since_begin += now - start_point;
        start_point  = now;

        if(need_print) printf("%s: %.*fs\n", title.c_str(),
                              precision, since_begin.count());

        return since_begin.count();
    }

    double delta(const std::string& title = "Since last check", int precision = 3)
    {
        if (is_stopped) {
            printf("Paused\n");
            return -1;
        }

        auto now      = sys_clock::now();
        seconds delta = now - start_point;
        since_begin  += delta;
        start_point   = now;

        if (need_print) printf("%s: %.*fs\n", title.c_str(),
                               precision, delta.count());

        return delta.count();
    }

    void start()
    {
        is_stopped  = false;
        start_point = sys_clock::now();
    }

    void pause()
    {
        if (!is_stopped) {
            is_stopped    = true;
            seconds delta = sys_clock::now() - start_point;
            since_begin  += delta;
        }
    }

    void resume()
    {
        start();
    }

    void reset(bool is_started = true)
    {
        since_begin = seconds::zero();
        is_stopped  = false;
        if (is_started) start();
    }

    void printOff() { need_print = false; }
    void printOn()  { need_print = true; }

private:
    sys_clock::time_point start_point;
    seconds since_begin;
    bool    is_stopped;
    bool    need_print;
};
CLS_END

#endif // CLS_TIMER_HPP
