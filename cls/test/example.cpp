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

#include <forward_list>
#include <list>

#include <catch.hpp>

#include <cls/utilities.h>

using namespace cls;

CLS_BEGIN
struct Shape {
public:
    using Ptr = std::unique_ptr<Shape>;
    virtual ~Shape() = default;

    virtual void draw() { std::cout << "drawing a shape" << std::endl; }
};

struct Rect : Shape {
    Rect(int width, int height)
        : w(width), h(height) {}
    Rect(int width)
        : w(width), h(width) {}
    ~Rect() {
        std::cout << "Rect dctor called, width: " << w << " height: " << h << std::endl;
    }

    void draw() override {
        std::cout << "drawing a Rect, width: " << w << " height: " << h << std::endl;
    }

    int w, h;
};

struct Square final : Rect {
    Square(int width)
        : Rect(width), w(width) {};
    ~Square() {
        std::cout << "Square dctor called, width: " << w << std::endl;
    }

    void draw() override {
        std::cout << "drawing a square, width: " << w << std::endl;
    }

    int w;
};

struct Star final : Shape {
    ~Star() {
        std::cout << "Star dctor called" << std::endl;
    }

    void draw() override {
        std::cout << "drawing a star\n";
    }
};

struct CmdArgs {
    std::string name       = "";
    std::string address    = "";
    int    post_code  = 0;
    bool   need_print = false;
    std::string load_file  = "";
};

namespace example {
void cmdLineParser()
{
    // Command line parser
    // Test arguments: --name=Clany -a "441 S Dunn St Apt 5" -c47401 -p --file "..\CMakeLists.txt"
    int t_argc = 9;
    const char* t_argv[] = {
        "Path to executable",
        "--name=Clany",
        "-a",
        "441 S Dunn St Apt 5",
        "-c47401",
        "-p",
        "shit!",
        "--file",
        "cmake_install.cmake"
    };

    std::vector<LongOption> long_options = {
        {"name"     , required_argument, 'n'},
        {"address"  , optional_argument, 'a'},
        {"post_code", required_argument, 'c'},
        {"print"    , no_argument      , 'p'},
        {"file"     , required_argument, 'f'},
    };
    CmdLineParser cmd_parser(t_argc, t_argv, "n:a::c:pf:", long_options);

    CmdArgs cmd_args;
    char ch;
    while ((ch = cmd_parser.get()) != -1) {
        switch (ch) {
        case 'n':
            cmd_args.name = cmd_parser.getArg<std::string>();
            break;
        case 'a':
            cmd_args.address = cmd_parser.getArg<std::string>();
            break;
        case 'c':
            cmd_args.post_code = cmd_parser.getArg<int>();
            break;
        case 'p':
            cmd_args.need_print = true;
            break;
        case 'f':
            cmd_args.load_file = cmd_parser.getArg<std::string>();
            break;
        case ':':
            std::cerr << "ERROR: Invalid option, missing argument!" << std::endl;
            std::exit(1);
        case '?':
            std::cerr << "ERROR: Unknown argument!" << std::endl;
            std::exit(1);
        default:
            std::cerr << "ERROR: Parsing fail!" << std::endl;
            std::exit(1);
        }
    }

    if (cmd_args.need_print) {
        std::cout.setf(std::ios::left);
        std::cout << std::setw(9) << "name"      << ": " << cmd_args.name      << std::endl;
        std::cout << std::setw(9) << "address"   << ": " << cmd_args.address   << std::endl;
        std::cout << std::setw(9) << "post code" << ": " << cmd_args.post_code << std::endl;
    }

    // File operation
    if (!cmd_args.load_file.empty()) {
        TRACE("Loaded file name: %s", cmd_args.load_file.c_str());

        auto text = readFile(cmd_args.load_file);       // Read text file, save to std::string
        auto data = readBinaryFile(cmd_args.load_file); // Read binary file, save to ByteArray

        int line_num = countLine(cmd_args.load_file);
        text = getLineStr(cmd_args.load_file, line_num / 6);
        TRACE("%s", text.c_str());
    }
}

void factoryPattern()
{
    // Factory pattern
    Shape::Ptr shape;

    using ShapeFactory = Factory<Shape>;
//    using ShapeFactory = Factory<Shape, std::string, shared_ptr>;

    ShapeFactory::addType<Rect, int>("Rect");
    ShapeFactory::addType<Rect, int, int>("Rect");
    ShapeFactory::addType<Square, int>("Square");
    ShapeFactory::addType<Star>("Star");

    shape = ShapeFactory::create("Rect", 7);
    shape->draw();
    shape = ShapeFactory::create("Rect", 3, 4);
    shape->draw();
    shape = ShapeFactory::create("Square", 9);
    shape->draw();
    shape = ShapeFactory::create("Star");
    shape->draw();

    ShapeFactory::removeType("Rect");
    shape = ShapeFactory::create("Rect", 5);
    if (!shape) std::cout << "could not find type Square" << std::endl;
    shape = ShapeFactory::create("Rect", 5, 10);
    if (!shape) std::cout << "could not find type Square" << std::endl;
}

void iteratorTraits()
{
    static_assert(!is_iterator<Shape>::value,
        "Shape is not an iterator");
    static_assert(is_iterator<Shape*>::value,
        "Shape* is an iterator");
    static_assert(!is_const_iterator<const int>::value,
        "const int is not a const iterator");
    static_assert(is_const_iterator<const Shape*>::value,
        "const Shape* is a const iterator");
    static_assert(!is_const_iterator<std::vector<int>::iterator>::value,
        "std::vector<int>::iterator is not a const iterator");
    static_assert(is_const_iterator<std::vector<int>::const_iterator>::value,
        "std::vector<int>::const_iterator is a const iterator");
    static_assert(is_input_iterator<std::vector<int>::iterator&>::value,
        "std::vector<int>::iterator is an input iterator");
    static_assert(!is_input_iterator<std::ostream_iterator<int>>::value,
        "ostream_iterator<int> is not an input iterator");
    static_assert(is_output_iterator<std::ostream_iterator<int>>::value,
        "ostream_iterator<int> is an output iterator");
    static_assert(!is_output_iterator<std::istream_iterator<int>>::value,
        "istream_iterator<int> is not an output iterator");
    static_assert(!is_output_iterator<std::vector<int>::const_iterator>::value,
        "std::vector<int>::const_iterator is not an output iterator");
    static_assert(is_forward_iterator<std::forward_list<int>::iterator>::value,
        "forward_list<int>::iterator is an forward iterator");
    static_assert(!is_bidirectional_iterator<std::forward_list<int>::iterator>::value,
        "forward_list<int>::iterator is not an bidirectional iterator");
    static_assert(is_bidirectional_iterator<std::list<int>::iterator>::value,
        "list<int>::iterator is an bidirectional iterator");
    static_assert(!is_random_access_iterator<std::forward_list<int>::iterator>::value,
        "forward_list<int>::iterator is not an random access iterator");
    static_assert(is_random_access_iterator<std::vector<int>::const_iterator>::value,
        "std::vector<int>::const_iterator is an random access iterator");
    static_assert(is_random_access_iterator<Shape*>::value,
        "Shape* is an random access iterator");
}

void print()
{
    using cls::print;

    std::string s0 = "Hello C++14!\n";
    std::string& s1 = s0;
    print("%s", std::string("Hello C++11!\n"));
    print("s0: %s", s0);
    print("s1: %s", s1);
    print("%d %d\n");
    print(L"%s %s\n");
}
}
CLS_END

TEST_CASE("Examples", "[example]") {
    example::cmdLineParser();
    example::factoryPattern();
    example::iteratorTraits();
    example::print();
}