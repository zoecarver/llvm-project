//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14, c++17

// functional

// template <class F, class... Args> unspecified bind_front(F&&, Args&&...);

#include <functional>

constexpr int pass(const int n) { return n; }

int simple(int n) { return n; }

template<class T>
T do_nothing(T t) { return t; }

int main(int, char**)
{
    int n = 1;
    const int c = 1;

    auto p = std::bind_front(pass, c);
    static_assert(p() == 1); // expected-error {{static_assert expression is not an integral constant expression}}

    auto d = std::bind_front(do_nothing, n); // expected-error {{no matching function for call to 'bind_front'}}

    return 0;
}