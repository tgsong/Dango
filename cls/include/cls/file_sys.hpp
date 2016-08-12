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

#ifndef CLS_FILE_HANDLE_HPP
#define CLS_FILE_HANDLE_HPP

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <iterator>
#include <stdexcept>
#include <limits>

#if defined _WIN32 && defined _MSC_VER && _MSC_VER >= 1800
#  include <filesystem>
#else
#  include <dirent.h>
#endif

#include "cls_defs.h"

_CLS_BEGIN
typedef istreambuf_iterator<char> ifsbuf_iter;

class FileExcept :public runtime_error
{
public:
    FileExcept(const string& err_msg) :runtime_error(err_msg) {};
};


inline string readFile(const string& file_name)
{
    ifstream ifs(file_name);
    if (!ifs.is_open()) {
#if CLS_HAS_EXCEPT
        throw FileExcept("Could not open file " + file_name);
#else
        cerr << "Fail to open the file" << endl;
        return string("");
#endif
    }

    return string(ifsbuf_iter(ifs), ifsbuf_iter());
}

inline vector<char> readBinaryFile(const string& file_name)
{
    ifstream ifs(file_name, ios::binary);
    if (!ifs) {
#if CLS_HAS_EXCEPT
        throw FileExcept("Could not open file " + file_name);
#else
        cerr << "Fail to open the file" << endl;
        return vector<char>();
#endif
    }

    return vector<char>(ifsbuf_iter(ifs), ifsbuf_iter());
}


inline ifstream& gotoLine(ifstream& file, int num)
{
    file.seekg(ios::beg);
    for (int i = 0; i < num-1; ++i) {
        file.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    return file;
}


inline string getLineStr(ifstream& ifs, int num)
{
    gotoLine(ifs, num);

    string curr_line;
    getline(ifs, curr_line);

    return curr_line;
}


inline string getLineStr(const string& file_name, int num)
{
    ifstream ifs(file_name);
    if (!ifs) {
#if CLS_HAS_EXCEPT
        throw FileExcept("Could not open file " + file_name);
#else
        cerr << "Fail to open the file" << endl;
        return string("");
#endif
    }

    return getLineStr(ifs, num);
}


inline int countLine(const string& file_name)
{
    ifstream ifs (file_name);

    int count = 0;
    string tmp;
    while (getline(ifs, tmp)) {
        ++count;
    }

    return count;
}
_CLS_END

#endif // CLS_FILE_HANDLE_HPP