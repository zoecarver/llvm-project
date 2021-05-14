//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03, c++11, c++14, c++17
// UNSUPPORTED: libcpp-no-concepts
// UNSUPPORTED: gcc-10

// __semiregular_box::__semiregular_box()

#include <__ranges/semiregular_box.h>

#include <cassert>
#include <type_traits>
#include <vector>

#include "test_macros.h"

struct aggregate {
  double value = 16.0;
  constexpr friend bool operator==(aggregate const x, aggregate const y) { return x.value == y.value; }
};

struct not_default_initializable {
  not_default_initializable() = delete;
  not_default_initializable(int) {}
};

using std::ranges::__semiregular_box;

template <std::default_initializable T>
constexpr bool check_default_initialization() {
  static_assert(!std::is_trivially_default_constructible_v<__semiregular_box<T> >);
  static_assert(std::is_nothrow_default_constructible_v<__semiregular_box<T> >);
  __semiregular_box<T> const boxed;
  T const value;
  assert(boxed.has_value());
  assert(*boxed == value);
  return true;
}

template <class T>
constexpr bool check_default_initialization() {
  static_assert(!std::is_trivially_default_constructible_v<__semiregular_box<T> >);
  static_assert(std::is_nothrow_default_constructible_v<__semiregular_box<T> >);
  __semiregular_box<T> const boxed;
  assert(!boxed.has_value());
  return true;
}

int main(int, char**) {
  static_assert(std::default_initializable<__semiregular_box<int> >);
  static_assert(!std::is_trivially_default_constructible_v<__semiregular_box<int> >);
  static_assert(std::is_nothrow_default_constructible_v<__semiregular_box<int> >);

  static_assert(check_default_initialization<aggregate>());
  check_default_initialization<aggregate>();

  check_default_initialization<std::vector<int> >();

  static_assert(check_default_initialization<not_default_initializable>());
  check_default_initialization<not_default_initializable>();

  return 0;
}
