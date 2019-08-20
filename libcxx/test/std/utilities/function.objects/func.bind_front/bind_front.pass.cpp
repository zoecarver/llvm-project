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
#include "callable_types.h"

int add (int a, int b) { return a + b; }

int long_test (int a, int b, int c, int d, int e, int f)
{ return a + b + c + d + e + f; }

struct Foo { int a; int b; };

struct FooCall { Foo operator()(int a, int b) { return Foo { a, b }; } };

struct S { bool operator()(int a) { return a == 1; } };

void basic_tests()
{
    int n = 2;
    int m = 1;

    auto a = std::bind_front(add, m, n);
    assert(a() == 3);

    auto b = std::bind_front(long_test, m, n, m, m, m, m);
    assert(b() == 7);

    auto c = std::bind_front(long_test, n, m);
    assert(c(1, 1, 1, 1) == 7);

    auto d = std::bind_front(S{}, m);
    assert(d());

    auto f = std::bind_front(add, n);
    assert(f(3) == 5);
}

void constructor_tests()
{
    {
        MoveOnlyCallable value(true);
        using RetT = decltype(std::bind_front(std::move(value), 1));

        static_assert( std::is_move_constructible<RetT>::value);
        static_assert(!std::is_copy_constructible<RetT>::value);
        static_assert(!std::is_move_assignable<RetT>::value);
        static_assert(!std::is_copy_assignable<RetT>::value);

        auto ret = std::bind_front(std::move(value), 1);
        assert(ret());
        assert(ret(1, 2, 3));

        auto ret1 = std::move(ret);
        assert(!ret());
        assert(ret1());
        assert(ret1(1, 2, 3));
    }
    {
        CopyCallable value(true);
        using RetT = decltype(std::bind_front(value, 1));

        static_assert( std::is_move_constructible<RetT>::value);
        static_assert( std::is_copy_constructible<RetT>::value);
        static_assert(!std::is_move_assignable<RetT>::value);
        static_assert(!std::is_copy_assignable<RetT>::value);

        auto ret = std::bind_front(value, 1);
        assert(ret());
        assert(ret(1, 2, 3));

        auto ret1 = std::move(ret);
        assert(ret1());
        assert(ret1(1, 2, 3));

        auto ret2 = std::bind_front(std::move(value), 1);
        assert(!ret());
        assert( ret2());
        assert( ret2(1, 2, 3));
    }
    {
        CopyAssignableWrapper value(true);
        using RetT = decltype(std::bind_front(value, 1));

        static_assert(std::is_move_constructible<RetT>::value);
        static_assert(std::is_copy_constructible<RetT>::value);
        static_assert(std::is_move_assignable<RetT>::value);
        static_assert(std::is_copy_assignable<RetT>::value);
    }
    {
        MoveAssignableWrapper value(true);
        using RetT = decltype(std::bind_front(std::move(value), 1));

        static_assert( std::is_move_constructible<RetT>::value);
        static_assert(!std::is_copy_constructible<RetT>::value);
        static_assert( std::is_move_assignable<RetT>::value);
        static_assert(!std::is_copy_assignable<RetT>::value);
    }
}

template<class Res, class F, class... Args>
void test_return(F&& value, Args&&... args)
{
    auto ret = std::bind_front(std::forward<F>(value), std::forward<Args>(args)...);
    static_assert(std::is_same<decltype(ret()), Res>::value);
}

void test_return_types()
{
    test_return<Foo>(FooCall{}, 1, 2);
    test_return<bool>(S{}, 1);
    test_return<int>(add, 2, 2);
}

void test_arg_count()
{
    using T = decltype(std::bind_front(add, 1));
    static_assert(!std::is_invocable<T>::value);
    static_assert( std::is_invocable<T, int>::value);
}

int main(int, char**)
{
    basic_tests();
    constructor_tests();
    test_return_types();
    test_arg_count();

    return 0;
}