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

// __semiregular_box& operator=(__semiregular_box&&)

#include <__ranges/semiregular_box.h>

#include <cassert>
#include <concepts>
#include <type_traits>
#include <utility>

#include "test_macros.h"

template <bool nothrow>
struct no_move_assign {
  constexpr no_move_assign(int x) : value(x) {}

  constexpr no_move_assign(no_move_assign&& other) noexcept(nothrow) : value(std::exchange(other.value, 0)) {}

  no_move_assign(no_move_assign const&) = default;
  no_move_assign& operator=(no_move_assign&&) = delete;
  no_move_assign& operator=(no_move_assign const&) = default;

  int value;
  constexpr bool operator==(int const x) const { return value == x; }
};

struct movable {
  constexpr movable(int x) : value(x) {}

  int value;
  constexpr bool operator==(int const x) const { return value == x; }
};

template <class T, bool nothrow_expected = true>
constexpr bool check_move_assign() {
  using box_t = std::ranges::__semiregular_box<T>;
  static_assert(std::is_move_assignable_v<box_t>);
  static_assert(std::is_nothrow_move_assignable_v<box_t> == nothrow_expected);

  auto x = box_t(std::in_place, 5);
  auto y = box_t(std::in_place, 10);
  x = std::move(y);
  assert(x.has_value());
  assert(y.has_value());
  assert(x.value() == 10);
  assert(std::is_trivially_move_assignable_v<box_t> || y.value() == 0);

  y.reset();
  x = std::move(y);
  assert(!x.has_value());
  return true;
}

int main() {
  static_assert(check_move_assign<int>());
  check_move_assign<int>();

  static_assert(check_move_assign<movable>());
  check_move_assign<movable>();

  static_assert(check_move_assign<no_move_assign<true> >());
  check_move_assign<no_move_assign<true> >();

  static_assert(check_move_assign<no_move_assign<false>, false>());
  check_move_assign<no_move_assign<false>, false>();
}
