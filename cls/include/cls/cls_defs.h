﻿/////////////////////////////////////////////////////////////////////////////////
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

#ifndef CLS_DEFS_H
#define CLS_DEFS_H

// Guideline Support Library
#include <gsl/gsl>

#define CLS_BEGIN      namespace cls {
#define CLS_END        }

#if defined CLSAPI_EXPORTS
#  define CLS_EXPORTS __declspec(dllexport)
#else
#  define CLS_EXPORTS
#endif

// Macros for debugging use
#undef ASSERT
#undef VERIFY
#undef DBGVAR
#undef DBGMSG
#undef TRACE

#ifdef _MSC_VER
#  define NOMINMAX
#  define _WINSOCKAPI_
#  include <windows.h>
#  define ASSERT _ASSERTE
#else
#  include <cassert>
#  define ASSERT assert
#endif

#ifndef NDEBUG
#  define VERIFY ASSERT
#  define DBGVAR(os, var) \
     (os) << "Debug: " << __FILE__ << "(" << __LINE__ << "): " \
          << #var << " = [" << (var) << "]" << std::endl
#  define DBGMSG(os, msg) \
     (os) << "Debug: " << __FILE__ << "(" << __LINE__ << "): " \
          << msg << std::endl
#  define CLS_TRACE(format, ...) \
     char buffer[256]; \
     std::sprintf(buffer, (format), ##__VA_ARGS__); \
     std::stringstream ss; \
     ss << "Debug: " << __FILE__ << "(" << __LINE__ << "): " << buffer << std::endl
#  ifdef _MSC_VER
#    define TRACE(format, ...) { \
       CLS_TRACE((format), ##__VA_ARGS__); \
       OutputDebugString(ss.str().c_str()); \
     }
#  else
#    define TRACE(format, ...) { \
       CLS_TRACE((format), ##__VA_ARGS__); \
       std::cout << ss.str(); \
     }
#  endif
#else
#  define VERIFY(expression) (expression)
#  define DBGVAR(os, var) ((void)0)
#  define DBGMSG(os, msg) ((void)0)
#  define TRACE(...) ((void)0)
#endif

// Exception macros
#ifndef CLS_HAS_EXCEPT
#  define CLS_HAS_EXCEPT 1
#endif

#undef TRY_BEGIN
#undef CATCH
#undef CATCH_ALL
#undef CATCH_END
#undef THROW
#undef RETHROW
#if CLS_HAS_EXCEPT
#  define EXCEPT_BEGIN try {
#  define EXCEPT_END   } catch (const std::exception& e) { std::cerr << e.what() << std::endl;    \
                       } catch (...) { std::cerr << "Unknown exception!" << std::endl; }
#  define CATCH(x)     } catch (x) {

#  define THROW(x)     throw x
#  define RETHROW      throw
#else
#  define EXCEPT_BEGIN {{
#  define EXCEPT_END   };}
#  define CATCH(x)     } [](x) {

#  define THROW(x)     ((void)0)
#  define RETHROW
#endif

// Convenient type alias
using ushort = unsigned short;
using uchar  = unsigned char;
using uint   = unsigned int;
using ulong  = unsigned long;
using ullong = unsigned long long;
using llong  = long long;

// Size and alignment
CLS_BEGIN
using size_type = std::ptrdiff_t;
template<typename T>
constexpr size_type size_of = static_cast<size_type>(sizeof(T));
template <typename T>
constexpr size_type align_of = static_cast<size_type>(alignof(T));
CLS_END

// Detect GCC and Clang Version
#define GCC_VERSION   (__GNUC__ * 100 +\
                       __GNUC_MINOR__)
#define CLANG_VERSION (__clang_major__ * 100 +\
                       __clang_minor__)
#define CPP11_SUPPORT (GCC_VERSION >= 408 || CLANG_VERSION >= 303 || _MSC_VER >= 1900)
#define CPP14_SUPPORT (GCC_VERSION >= 500 || CLANG_VERSION >= 304 || _MSC_VER >= 1900)

#endif // CLS_DEFS_H
