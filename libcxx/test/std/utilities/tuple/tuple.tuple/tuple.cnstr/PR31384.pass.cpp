// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03

// See https://llvm.org/PR31384

#include <tuple>
#include <cassert>

#include "test_macros.h"

int count = 0;

struct Explicit {
  Explicit() = default;
  explicit Explicit(int) {}
};

struct Implicit {
  Implicit() = default;
  Implicit(int) {}
};

template<class T>
struct Derived : std::tuple<T> {
  using std::tuple<T>::tuple;
  template<class U>
  operator std::tuple<U>() && { ++count; return {}; }
};


template<class T>
struct ExplicitDerived : std::tuple<T> {
  using std::tuple<T>::tuple;
  template<class U>
  explicit operator std::tuple<U>() && { ++count; return {}; }
};

int main(int, char**) {
  {
    std::tuple<Explicit> foo = Derived<int>{42}; ((void)foo);
    assert(count == 1);
    std::tuple<Explicit> bar(Derived<int>{42}); ((void)bar);
    assert(count == 2);
  }
  count = 0;
  {
    std::tuple<Implicit> foo = Derived<int>{42}; ((void)foo);
    assert(count == 1);
    std::tuple<Implicit> bar(Derived<int>{42}); ((void)bar);
    assert(count == 2);
  }
  count = 0;
  {
    static_assert(!std::is_convertible<
        ExplicitDerived<int>, std::tuple<Explicit>>::value, "");
    std::tuple<Explicit> bar(ExplicitDerived<int>{42}); ((void)bar);
    assert(count == 1);
  }
  count = 0;
  {
    std::tuple<Implicit> foo = ExplicitDerived<int>{42}; ((void)foo);
    static_assert(std::is_convertible<
        ExplicitDerived<int>, std::tuple<Implicit>>::value, "");
    assert(count == 0);
    std::tuple<Implicit> bar(ExplicitDerived<int>{42}); ((void)bar);
    assert(count == 1);
  }
  count = 0;


  return 0;
}
