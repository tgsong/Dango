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

#ifndef CLS_BYTE_ARRAY_HPP
#define CLS_BYTE_ARRAY_HPP

#include <cstring>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include "cls_defs.h"

CLS_BEGIN
class ByteArray : public vector<char> {
public:
    using Base = vector<char>;

    ByteArray() = default;
    ByteArray(const Base& data) : Base(data) {}
    ByteArray(Base&& data) : Base(move(data)) {}
#if CPP11_SUPPORT
    using Base::Base;
#else
    ByteArray(size_t n, char byte = char())
        : Base(n, byte) {}

    template<typename InputIterator>
    ByteArray(InputIterator first, InputIterator last)
        : Base(first, last) {}
#endif

    explicit ByteArray(const char* data, int size = -1)
        : Base(data, data + (size < 0 ? strlen(data) : size)) {}

    explicit ByteArray(const string& data)
        : Base(data.begin(), data.end()) {}

    // Allow implicit conversion
    operator string() const  { return to_string(); }

    string to_string() const { return string(begin(), end()); }

    ByteArray& append(const ByteArray& data) {
        insert(end(), data.begin(), data.end());
        return *this;
    }

    ByteArray& append(const string& data) {
        insert(end(), data.begin(), data.end());
        return *this;
    }

    ByteArray& append(const char* data, int size) {
        insert(end(), data, data + (size < 0 ? strlen(data) : size));
        return *this;
    }

    ByteArray& append(char byte) {
        push_back(byte);
        return *this;
    };

    ByteArray& operator+=(const ByteArray& data) { return append(data); };
    ByteArray& operator+=(const string& data)    { return append(data); };
    ByteArray& operator+=(const char* data)      { return append(data, -1); };
    ByteArray& operator+=(char byte)             { return append(byte); };

    void fill(char byte) { assign(size(), byte); };

    ByteArray sub(int pos, int len = -1) const {
        auto begin_iter = begin() + pos;
        return ByteArray(begin_iter, len < 0 ? end() : begin_iter + len);
    };
};

inline ByteArray operator+(const ByteArray& left, const ByteArray& right)
{
    ByteArray result(left);
    return result += right;
}

inline bool operator==(const ByteArray& left, const ByteArray& right)
{
    return left.size() == right.size() &&
           equal(left.begin(), left.end(), right.begin());
}

inline bool operator!=(const ByteArray& left, const ByteArray& right)
{
    return !(left == right);
}

inline ostream& operator<<(ostream& os, const ByteArray& byte_arr)
{
    stringstream ss;
    ss.flags(ios::right | ios::hex);
    ss.fill('0');
    for_each(byte_arr.begin(), byte_arr.end(), [&ss](uchar c) {
        ss << setw(2) << (int)c << " ";
    });
    os << ss.str();
    return os;
}
CLS_END



#endif // CLS_BYTE_ARRAY_HPP