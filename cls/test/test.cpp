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

#include <random>

#include <catch.hpp>

#include <cls/utilities.h>

using namespace std;
using namespace cls;

namespace {
vector<int> vec1 {9, 3, 5, 7, 9, 13, 9, 17, 17, 9};
int arr1[] = {9, 3, 5, 7, 3, 11, 9, 1, 13, 9};
}

TEST_CASE("Algorithm tests", "[algorithm]") {
    SECTION("print container contents") {
        cout << vec1 << endl;
    }

    CHECK(all_of(vec1, [](int ele) { return ele % 2 == 1; }));
    CHECK(none_of(arr1, [](int ele) { return ele % 2 == 0; }));

    CHECK(count(vec1, 9) == 4);
    CHECK(count_if(arr1, [](int ele) { return ele % 3 == 0; }) == 5);

    auto non_eq = mismatch(vec1, arr1, less_equal<int>());
    DBGVAR(cout, *non_eq.first);
    DBGVAR(cout, *non_eq.second);

    auto vec2 = vec1;
    CHECK(equal(vec1, vec2));
    CHECK(equal(vec1, arr1, greater_equal<int>()));

    CHECK(find(arr1, 11) != end(arr1));
    DBGVAR(cout, *find_if(arr1, [](int ele) { return ele > 9; }));
    CHECK(distance(begin(vec1), find_first_of(vec1, {11, 15, 17}, greater<int>())) == 5);

    CHECK(distance(begin(vec1), adjacent_find(vec1)) == 7);
    CHECK(distance(begin(arr1), adjacent_find(arr1,
        [](int ele1, int ele2) { return (ele2 - ele1) == 8; })) == 4);

    CHECK(distance(begin(arr1), search(arr1, {11, 9, 1})) == 5);
    CHECK(distance(begin(vec1), search_n(vec1, 6, 7, greater<int>())) == 4);

    copy({1, 2, 3}, vec2.begin());
    copy(arr1, vec2);
    CHECK(equal(arr1, vec2));
    copy_if(vec1, vec2, [](int ele) { return ele >= 13; });
    CHECK(equal({13, 17, 17}, vec2));
    vec2 = {5, 7, 3, 11, 9, 1, 13};
    copy_backward({13, 15, 15}, vec2.end());
    CHECK(3 == distance(search(vec2, {13, 15, 15}), end(vec2)));

    move_backward(vec2, end(arr1));

    fill(arr1, -1);
    CHECK(all_of(arr1, [](int ele) { return ele == -1; }));

    transform(vec2, begin(arr1), negate<int>());
    transform({1, 3, 5}, vec1, negate<int>());
    CHECK(equal(vec2, arr1, [](int ele1, int ele2) {
        return abs(ele1) == abs(ele2);
    }));

    const uint SEED = random_device()();
    auto  rd_engine = bind(uniform_int_distribution<> {1, 20}, default_random_engine {SEED});
    generate(arr1, rd_engine);
    remove_copy_if(arr1, vec1, [](int ele) { return ele % 2 != 0; });
    rotate_copy(vec2, 3, begin(arr1));

    DBGVAR(cout, accumulate(vec1) + 0.1f);
    DBGVAR(cout, accumulate(arr1, 1, [](int ele, int init) {
        return ele*init;
    }));

    vector<float> vec3(vec1.size());
    transform(arr1, vec3, [](int ele) { return ele + 0.3; });
    DBGVAR(cout, inner_product(vec1, vec3));

    auto inprod = inner_product(vec1, vec3,
        [](float ele1, float ele2) { return ele1 + ele2; },
        [](int   ele1, float ele2) { return ele1 / ele2; });
    DBGVAR(cout, inprod);

    vec1.resize(10);
    generate(vec1, rd_engine);
    partial_sort(vec1, 3);
    CHECK(4 <= distance(vec1.begin(), is_sorted_until(vec1)));
    sort(vec1, greater<int>());

    auto minmax_val = minmax_element(arr1);
    DBGVAR(cout, *minmax_val.first);
    DBGVAR(cout, *minmax_val.second);
}

TEST_CASE("Print tests", "[print]") {
    CHECK(L"hello" == stows(string("hello")));
    CHECK("hello" == wstos(wstring(L"hello")));
}