/////////////////////////////////////////////////////////////////////////////////
// The MIT License(MIT)
//
// Copyright (c) 2015 Tiangang Song
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

#ifndef CLS_STRING_HPP
#define CLS_STRING_HPP

#include <cstdio>
#include <iostream>
#include <string>
#include <codecvt>
#include <locale>
#include "cls_defs.h"

CLS_BEGIN
namespace detail {
template <typename T>
T argument(T value) noexcept
{
    return value;
}

template <typename T>
inline const T* argument(const std::basic_string<T>& value) noexcept
{
    return value.c_str();
}

template <typename... Args>
inline int formatStr(
    char* buffer,
    const size_t b_size,
    const char* const format,
    const Args&... args
) noexcept
{
    int result = snprintf(buffer, b_size, format, argument(args)...);
    ASSERT(-1 != result);
    return result;
}

template <typename... Args>
inline int formatStr(
    wchar_t* buffer,
    const size_t b_size,
    const wchar_t* const format,
    const Args&... args
) noexcept
{
    int result = swprintf(buffer, b_size, format, argument(args)...);
    ASSERT(-1 != result);
    return result;
}
} // End namespace detail

////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename... Args>
inline void print(const char* const format, const Args&... args) noexcept
{
    printf(format, detail::argument(args)...);
}

inline void print(const char* const value) noexcept
{
    printf("%s", value);
}

inline void print(const wchar_t* const value) noexcept
{
    printf("%ls", value);
}

template <typename T>
inline void print(const std::basic_string<T>& value) noexcept
{
    print(value.c_str());
}

template <typename T, typename... Args>
auto format(const T* const format, const Args&... args) -> std::basic_string<T>
{
    static const size_t BUFFER_SIZE = 1024;

    std::basic_string<T> buffer(BUFFER_SIZE, '\0');

    size_t size = detail::formatStr(&buffer[0], buffer.size() + 1, format, args...);
    if (size > buffer.size())
    {
        buffer.resize(size);
        detail::formatStr(&buffer[0], buffer.size() + 1, format, args...);
    }
    else if (size < buffer.size())
    {
        buffer.resize(size);
    }

    return buffer;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
inline std::string wstos(const wchar_t* wstr)
{
    auto result = format("%ls", wstr);
    return result;
}

inline std::string wstos(const std::wstring& wstr)
{
    return wstos(wstr.c_str());
}

inline std::wstring stows(const char* str)
{
    auto result = format(L"%hs", str);
    return result;
}

inline std::wstring stows(const std::string& str)
{
    return stows(str.c_str());
}

inline std::string to_utf8(const std::wstring& wstr)
{
  static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  return converter.to_bytes(wstr);
}

inline std::wstring to_utf16(const std::string& str)
{
  static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  return converter.from_bytes(str);
}
CLS_END

#endif // CLS_STRING_HPP
