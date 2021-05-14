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

// __semiregular_box& operator=(__semiregular_box const&)

#include <__ranges/semiregular_box.h>

#include <cassert>
#include <concepts>
#include <type_traits>

#include "test_macros.h"

template <bool nothrow>
struct no_move_assign {
  constexpr no_move_assign(int x) : value(x) {}
  no_move_assign(no_move_assign&&) = default;

  constexpr no_move_assign(no_move_assign const& other) noexcept(nothrow) : value(other.value) {}

  no_move_assign& operator=(no_move_assign&&) = delete;
  no_move_assign& operator=(no_move_assign const&) = default;

  int value;
  constexpr bool operator==(int const x) const { return value == x; }
};

template <bool nothrow>
struct no_copy_assign {
  constexpr no_copy_assign(int x) : value(x) {}
  no_copy_assign(no_copy_assign&&) = default;

  constexpr no_copy_assign(no_copy_assign const& other) noexcept(nothrow) : value(other.value) {}

  no_copy_assign& operator=(no_copy_assign&&) = default;
  no_copy_assign& operator=(no_copy_assign const&) = delete;

  int value;
  constexpr bool operator==(int const x) const { return value == x; }
};

struct copyable {
  constexpr copyable(int x) : value(x) {}

  int value;
  constexpr bool operator==(int const x) const { return value == x; }
};

template <class T, bool nothrow = true>
constexpr bool check_copy_assign() {
  using box_t = std::ranges::__semiregular_box<T>;
  static_assert(std::is_copy_assignable_v<box_t>);
  static_assert(std::is_nothrow_copy_assignable_v<box_t> == nothrow);

  auto x = box_t(std::in_place, 5);
  auto y = box_t(std::in_place, 10);
  x = y;
  assert(x.has_value());
  assert(y.has_value());
  assert(x.value() == 10);

  y.reset();
  x = y;
  assert(!x.has_value());
  return true;
}

int main() {
  static_assert(check_copy_assign<int>());
  check_copy_assign<int>();

  static_assert(check_copy_assign<copyable>());
  check_copy_assign<copyable>();

  static_assert(check_copy_assign<no_move_assign<true> >());
  check_copy_assign<no_move_assign<true> >();

  static_assert(check_copy_assign<no_move_assign<false>, false>());
  check_copy_assign<no_move_assign<false>, false>();

  static_assert(check_copy_assign<no_copy_assign<true> >());
  check_copy_assign<no_copy_assign<true> >();

  static_assert(check_copy_assign<no_copy_assign<false>, false>());
  check_copy_assign<no_copy_assign<false>, false>();
}
