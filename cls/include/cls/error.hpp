/////////////////////////////////////////////////////////////////////////////////
// The MIT License(MIT)
//
// Copyright (c) 2015 by Tiangang Song
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

#ifndef CLS_ERROR_HPP
#define CLS_ERROR_HPP

#include <cassert>
#include <stdexcept>
#include "string.hpp"

#if CPP11_SUPPORT
#  define CLS_FUNC __func__
#else
#  define CLS_FUNC ""
#endif

#define CLS_Error( msg ) cls::error( msg, CLS_FUNC, __FILE__, __LINE__ )

#define CLS_Assert( expr ) if(!!(expr)) ; else cls::error( "Assertion failed: "#expr, CLS_FUNC, __FILE__, __LINE__ )

#ifdef _DEBUG
#  define CLS_AssertD(expr) CLS_Assert(expr)
#else
#  define CLS_AssertD(expr)
#endif

_CLS_BEGIN
struct Exception : public std::exception
{
  Exception() = default;
  Exception(const string& i_err, const string& i_func, const string& i_file, int i_line)
    : err(i_err), func(i_func), file(i_file), line(i_line)
  {
    if (func.size() > 0)
      msg = format("%s:%d: %s in function %s\n", file.c_str(), line, err.c_str(), func.c_str());
    else
      msg = format("%s:%d: %s\n", file.c_str(), line, err.c_str());
  }

  const char* what() const noexcept override
  {
    return msg.c_str();
  }

private:
  string msg = "";

  string err  = "";
  string func = "";
  string file = "";
  int line    = 0;
};

inline void error(const Exception& exc)
{
  cerr << exc.what() << endl;

  throw exc;
}

inline void error(const string& err, const string& func, const string& file, int line)
{
  error(cls::Exception(err, func, file, line));
}
_CLS_END

#endif // CLS_ERROR_HPP