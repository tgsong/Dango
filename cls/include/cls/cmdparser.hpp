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

#ifndef CLS_CMDPARSER_HPP
#define CLS_CMDPARSER_HPP

#include <vector>
#include <map>
#include <sstream>
#include <stdexcept>
#include "cls_defs.h"

CLS_BEGIN
class ParseError : public std::logic_error {
public:
    explicit ParseError(const std::string& err_msg)
        : std::logic_error(err_msg) {
    }
};

enum { no_argument = 0, required_argument, optional_argument };

struct LongOption {
    std::string name;
    int    has_arg;
    char   val;
};

class CmdLineParser {
private:
    struct Option {
        char name;
        int has_arg;
    };

public:
    CmdLineParser(int argc, const char* const argv[], const std::string& options)
        : arg_vec(argv + 1, argv + argc), pos(0), curr_idx(0, false), next_idx(0) {
        if (argc != 1) initParser(options);
    }

    CmdLineParser(int argc, const char* const argv[],
                  const std::string& options,
                  const std::vector<LongOption>& long_options)
        : CmdLineParser(argc, argv, options) {
        // Store user provided long options
        for (const auto& lopt : long_options) {
            long_opt_map.insert({lopt.name, lopt});
        }
    }

    // Return next option character, if no more option characters available, return -1
    char get() {
        if (pos == opt_idx_vec.size()) return -1;
        curr_idx = opt_idx_vec[pos++];

        std::string option = arg_vec[curr_idx.first];
        if (!curr_idx.second) {     // Short option
            if (opt_map.find(option[1]) == opt_map.end()) return '?';
            opt_char = option[1];
        } else {                    // Long option
            option = option.substr(2, option.find('=') - 2);
            if (long_opt_map.find(option) == long_opt_map.end()) return '?';
            opt_char = long_opt_map.at(option).val;
        }
        bool next_is_arg = parseArg();
        if (opt_arg == ":") return ':';

        // Update the index of next option to be processed
        if (pos != opt_idx_vec.size()) {
            next_idx = opt_idx_vec[pos].first;
        } else {
            next_idx = opt_idx_vec.back().first + (next_is_arg ? 2 : 1);
        }

        return opt_char;
    }

    // Get current option argument
    template<typename T>
    T getArg() const {
        std::istringstream ss(opt_arg);
        T parsed_arg;
        ss >> parsed_arg;
        return parsed_arg;
    }

    // Get the index of next option to be processed in argv
    int getIndex() const {
        return static_cast<int>(next_idx + 1);    // Program name counts 1
    }

    // Get argument of custom option
    template<typename T>
    T parse(const std::string& name) {
        // Save current state
        auto old_curr_idx = curr_idx;
        auto old_opt_arg  = opt_arg;

        if (!findArg(name)) {
#if CLS_HAS_EXCEPT
            throw ParseError("Argument not found or not required!");
#else
            cerr << "Argument not found or not required!" << endl;
            exit(1);
#endif
        }

        std::istringstream ss(opt_arg);
        T parsed_arg;
        ss >> parsed_arg;

        curr_idx = old_curr_idx;
        opt_arg  = old_opt_arg;

        return parsed_arg;
    }

private:
    void initParser(const std::string& options) {
        // Get all possible options
        char last_opt;
        for (const auto& opt : options) {
            if (opt != ':') {
                opt_map.insert({opt, Option {opt, 0}});
                last_opt = opt;
            } else {
                opt_map.at(last_opt).has_arg++;
            }
        }
        // Extract all options and long options in arg_vec, store index
        for (size_t i = 0; i < arg_vec.size(); ++i) {
            const std::string& arg = arg_vec[i];
            if (arg[0] == '-') opt_idx_vec.emplace_back(i, arg[1] == '-');
        }
    }

    // Return true if parsed argument is the next argument in argv, else return false
    bool parseArg() {
        std::string option   = arg_vec[curr_idx.first];
        std::string next_arg = curr_idx.first + 1 == arg_vec.size() ? "" : arg_vec[curr_idx.first + 1];
        if (curr_idx.second) {    // Long option
            option.erase(0, 2);
            opt_arg = parseArg(option, next_arg, curr_idx.second, long_opt_map.at(
                               option.substr(0, option.find('='))).has_arg);
        } else {                  // Short option
            option.erase(0, 1);
            opt_arg = parseArg(option, next_arg, curr_idx.second,
                               opt_map.at(option[0]).has_arg);
        }
        return !opt_arg.empty() && opt_arg == next_arg;
    }

    std::string parseArg(const std::string& option, const std::string& next, bool is_long_opt, int has_argument) const {
        // Option does not require an argument
        if (!has_argument) return std::string("\0", 1);

        size_t eq_sign_pos = option.find('=');
        if (eq_sign_pos != std::string::npos) return option.substr(eq_sign_pos + 1);

        if (!is_long_opt) {
            std::string arg = option.substr(1);
            if (!arg.empty()) return arg;
        }

        if (!next.empty() && next[0] != '-') return next;

        // Option require an argument but not found
        if (has_argument == 1) return ":";

        return "";
    }

    bool findArg(const std::string& name) {
        // Traverse all possible options
        for (const auto& idx : opt_idx_vec) {
            auto option = arg_vec[idx.first];
            // Check if match any short/long option
            if (name == option.substr(1, 1) ||
                name == option.substr(2, option.find('=') - 2)) {
                curr_idx = idx;
                parseArg();
                if (opt_arg[0] == '\0' || opt_arg == ":") return false;
            }
        }
        return true;
    }

private:
    // All commandline arguments
    std::vector<std::string> arg_vec;
    // Option index in arg_vec, second element is true if it's a long option
    std::vector<std::pair<size_t, bool>> opt_idx_vec;
    size_t pos;
    std::pair<size_t, bool> curr_idx;

    // Index of next option to be processed
    size_t next_idx;
    // Current option
    char opt_char;
    // Argument of current option
    std::string opt_arg;

    // User defined available options
    std::map<char, Option>       opt_map;
    std::map<std::string, LongOption> long_opt_map;
};

template<>
inline std::string CmdLineParser::getArg<std::string>() const {
    return opt_arg;
}

template<>
inline std::string CmdLineParser::parse<std::string>(const std::string& name) {
    // Save current state
    auto old_curr_idx = curr_idx;
    auto old_opt_arg  = opt_arg;

    if (!findArg(name)) {
#if CLS_HAS_EXCEPT
        throw ParseError("Argument not found or not required!");
#else
        cerr << "Argument not found or not required!" << endl;
        exit(1);
#endif
    }
    std::string parsed_arg = opt_arg;

    curr_idx = old_curr_idx;
    opt_arg  = old_opt_arg;

    return parsed_arg;
}

CLS_END

#endif // CLS_CMDPARSER_HPP
