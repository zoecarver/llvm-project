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
// XFAIL: msvc && clang

// ranges::begin

#include <ranges>

#include <cassert>
#include <concepts>
#include <iterator>

#include "../begin.h"

#include "test_iterators.h"
#include "test_macros.h"

using begin_t = decltype(std::ranges::begin);

static_assert(std::semiregular<std::remove_cv_t<begin_t> >);
static_assert(std::invocable<begin_t&, int (&)[10]>);
static_assert(!std::invocable<begin_t&, int(&&)[10]>);
static_assert(!std::invocable<begin_t&, int (&)[]>);
static_assert(!std::invocable<begin_t&, int(&&)[]>);
ASSERT_NOEXCEPT(std::ranges::begin(std::declval<int (&)[10]>()));

// clang-format off
template <class T>
concept invocable_member_with_desired_result =
  std::invocable<begin_t&, T> &&
  requires(T&& t) {
    { std::ranges::begin(std::forward<T>(t)) } -> std::same_as<std::decay_t<decltype(t.begin())> >;
  };

template <class T>
concept invocable_adl_with_desired_result =
  std::invocable<begin_t&, T> &&
  requires(T&& t) {
    { std::ranges::begin(std::forward<T>(t)) } -> std::same_as<std::decay_t<decltype(begin(t))> >;
  };
// clang-format on

template <template <enable_constness, bool> class T, bool is_borrowed>
[[nodiscard]] consteval bool check_invocable_lvalue_member() {
  constexpr bool result = invocable_member_with_desired_result<T<enable_constness::mutable_only, is_borrowed>&>;
  static_assert(!invocable_member_with_desired_result<T<enable_constness::mutable_only, is_borrowed> const&>);
  static_assert(invocable_member_with_desired_result<T<enable_constness::const_only, is_borrowed>&> == result);
  static_assert(invocable_member_with_desired_result<T<enable_constness::const_only, is_borrowed> const&> == result);
  static_assert(invocable_member_with_desired_result<T<enable_constness::both, is_borrowed>&> == result);
  static_assert(invocable_member_with_desired_result<T<enable_constness::both, is_borrowed> const&> == result);
  return result;
}

template <template <enable_constness, bool> class T, bool is_borrowed>
[[nodiscard]] consteval bool check_invocable_rvalue_member() {
  constexpr bool result = invocable_member_with_desired_result<T<enable_constness::mutable_only, is_borrowed>&&>;
  static_assert(!invocable_member_with_desired_result<T<enable_constness::mutable_only, is_borrowed> const&&>);
  static_assert(invocable_member_with_desired_result<T<enable_constness::const_only, is_borrowed>&&> == result);
  static_assert(invocable_member_with_desired_result<T<enable_constness::const_only, is_borrowed> const&&> == result);
  static_assert(invocable_member_with_desired_result<T<enable_constness::both, is_borrowed>&&> == result);
  static_assert(invocable_member_with_desired_result<T<enable_constness::both, is_borrowed> const&&> == result);
  return result;
}

template <template <enable_constness, bool> class T, bool is_borrowed>
[[nodiscard]] consteval bool check_invocable_lvalue_adl() {
  constexpr bool result = invocable_adl_with_desired_result<T<enable_constness::mutable_only, is_borrowed>&>;
  static_assert(!invocable_adl_with_desired_result<T<enable_constness::mutable_only, is_borrowed> const&>);
  static_assert(!invocable_adl_with_desired_result<T<enable_constness::const_only, is_borrowed>&>);
  static_assert(invocable_adl_with_desired_result<T<enable_constness::const_only, is_borrowed> const&> == result);
  static_assert(invocable_adl_with_desired_result<T<enable_constness::both, is_borrowed>&> == result);
  static_assert(invocable_adl_with_desired_result<T<enable_constness::both, is_borrowed> const&> == result);
  return result;
}

template <template <enable_constness, bool> class T, bool is_borrowed>
[[nodiscard]] consteval bool check_invocable_rvalue_adl() {
  constexpr bool result = invocable_adl_with_desired_result<T<enable_constness::mutable_only, is_borrowed>&&>;
  static_assert(!invocable_adl_with_desired_result<T<enable_constness::mutable_only, is_borrowed> const&&>);
  static_assert(!invocable_adl_with_desired_result<T<enable_constness::const_only, is_borrowed>&&>);
  static_assert(invocable_adl_with_desired_result<T<enable_constness::const_only, is_borrowed> const&&> == result);
  static_assert(invocable_adl_with_desired_result<T<enable_constness::both, is_borrowed>&&> == result);
  static_assert(invocable_adl_with_desired_result<T<enable_constness::both, is_borrowed> const&&> == result);
  return result;
}

static_assert(check_invocable_lvalue_member<member_only, !borrowed>());
static_assert(!check_invocable_rvalue_member<member_only, !borrowed>());
static_assert(check_invocable_lvalue_member<member_only, borrowed>());
static_assert(check_invocable_rvalue_member<member_only, borrowed>());

static_assert(check_invocable_lvalue_member<lvalue_member_only, !borrowed>());
static_assert(!check_invocable_rvalue_member<lvalue_member_only, !borrowed>());
static_assert(check_invocable_lvalue_member<lvalue_member_only, borrowed>());
static_assert(check_invocable_rvalue_member<lvalue_member_only, borrowed>());

static_assert(!check_invocable_lvalue_member<rvalue_member_only, !borrowed>());
static_assert(!check_invocable_rvalue_member<rvalue_member_only, !borrowed>());
static_assert(!check_invocable_lvalue_member<rvalue_member_only, borrowed>());
static_assert(!check_invocable_rvalue_member<rvalue_member_only, borrowed>());

static_assert(check_invocable_lvalue_member<member_preferred, !borrowed>());
static_assert(!check_invocable_rvalue_member<member_preferred, !borrowed>());
static_assert(check_invocable_lvalue_member<member_preferred, borrowed>());
static_assert(check_invocable_rvalue_member<member_preferred, borrowed>());

static_assert(check_invocable_lvalue_member<lvalue_member_preferred, !borrowed>());
static_assert(!check_invocable_rvalue_member<lvalue_member_preferred, !borrowed>());
static_assert(check_invocable_lvalue_member<lvalue_member_preferred, borrowed>());
static_assert(check_invocable_rvalue_member<lvalue_member_preferred, borrowed>());

static_assert(!check_invocable_lvalue_member<rvalue_member_preferred, !borrowed>());
static_assert(!check_invocable_rvalue_member<rvalue_member_preferred, !borrowed>());
static_assert(!check_invocable_lvalue_member<rvalue_member_preferred, borrowed>());
static_assert(!check_invocable_rvalue_member<rvalue_member_preferred, borrowed>());

static_assert(check_invocable_lvalue_adl<lvalue_adl_only, !borrowed>());
static_assert(!check_invocable_rvalue_adl<lvalue_adl_only, !borrowed>());
static_assert(check_invocable_lvalue_adl<lvalue_adl_only, borrowed>());
static_assert(check_invocable_rvalue_adl<lvalue_adl_only, borrowed>());

static_assert(!check_invocable_lvalue_adl<rvalue_adl_only, !borrowed>());
static_assert(!check_invocable_rvalue_adl<rvalue_adl_only, !borrowed>());
static_assert(!check_invocable_lvalue_adl<rvalue_adl_only, borrowed>());
static_assert(!check_invocable_rvalue_adl<rvalue_adl_only, borrowed>());

static_assert(check_invocable_lvalue_adl<adl_only, !borrowed>());
static_assert(!check_invocable_rvalue_adl<adl_only, !borrowed>());
static_assert(check_invocable_lvalue_adl<adl_only, borrowed>());
static_assert(check_invocable_rvalue_adl<adl_only, borrowed>());

static_assert(check_invocable_lvalue_adl<lvalue_adl_with_private_members, !borrowed>());
static_assert(!check_invocable_rvalue_adl<lvalue_adl_with_private_members, !borrowed>());
static_assert(check_invocable_lvalue_adl<lvalue_adl_with_private_members, borrowed>());
static_assert(check_invocable_rvalue_adl<lvalue_adl_with_private_members, borrowed>());

static_assert(check_invocable_lvalue_adl<lvalue_adl_with_private_lvalue_members, !borrowed>());
static_assert(!check_invocable_rvalue_adl<lvalue_adl_with_private_lvalue_members, !borrowed>());
static_assert(check_invocable_lvalue_adl<lvalue_adl_with_private_lvalue_members, borrowed>());
static_assert(check_invocable_rvalue_adl<lvalue_adl_with_private_lvalue_members, borrowed>());

static_assert(check_invocable_lvalue_adl<lvalue_adl_with_private_rvalue_members, !borrowed>());
static_assert(!check_invocable_rvalue_adl<lvalue_adl_with_private_rvalue_members, !borrowed>());
static_assert(check_invocable_lvalue_adl<lvalue_adl_with_private_rvalue_members, borrowed>());
static_assert(check_invocable_rvalue_adl<lvalue_adl_with_private_rvalue_members, borrowed>());

static_assert(!check_invocable_lvalue_adl<rvalue_adl_with_private_members, !borrowed>());
static_assert(!check_invocable_rvalue_adl<rvalue_adl_with_private_members, !borrowed>());
static_assert(!check_invocable_lvalue_adl<rvalue_adl_with_private_members, borrowed>());
static_assert(!check_invocable_rvalue_adl<rvalue_adl_with_private_members, borrowed>());

static_assert(!check_invocable_lvalue_adl<rvalue_adl_with_private_lvalue_members, !borrowed>());
static_assert(!check_invocable_rvalue_adl<rvalue_adl_with_private_lvalue_members, !borrowed>());
static_assert(!check_invocable_lvalue_adl<rvalue_adl_with_private_lvalue_members, borrowed>());
static_assert(!check_invocable_rvalue_adl<rvalue_adl_with_private_lvalue_members, borrowed>());
static_assert(!check_invocable_rvalue_adl<rvalue_adl_with_private_members, borrowed>());

static_assert(!check_invocable_lvalue_adl<rvalue_adl_with_private_rvalue_members, !borrowed>());
static_assert(!check_invocable_rvalue_adl<rvalue_adl_with_private_rvalue_members, !borrowed>());
static_assert(!check_invocable_lvalue_adl<rvalue_adl_with_private_rvalue_members, borrowed>());
static_assert(!check_invocable_rvalue_adl<rvalue_adl_with_private_rvalue_members, borrowed>());

static_assert(check_invocable_lvalue_adl<lvalue_adl_with_bad_members, !borrowed>());
static_assert(!check_invocable_rvalue_adl<lvalue_adl_with_bad_members, !borrowed>());
static_assert(check_invocable_lvalue_adl<lvalue_adl_with_bad_members, borrowed>());
static_assert(check_invocable_rvalue_adl<lvalue_adl_with_bad_members, borrowed>());

static_assert(!check_invocable_lvalue_adl<rvalue_adl_with_bad_members, !borrowed>());
static_assert(!check_invocable_rvalue_adl<rvalue_adl_with_bad_members, !borrowed>());
static_assert(!check_invocable_lvalue_adl<rvalue_adl_with_bad_members, borrowed>());
static_assert(!check_invocable_rvalue_adl<rvalue_adl_with_bad_members, borrowed>());

static_assert(check_invocable_lvalue_adl<lvalue_adl_with_data_member_begin, !borrowed>());
static_assert(!check_invocable_rvalue_adl<lvalue_adl_with_data_member_begin, !borrowed>());
static_assert(check_invocable_lvalue_adl<lvalue_adl_with_data_member_begin, borrowed>());
static_assert(check_invocable_rvalue_adl<lvalue_adl_with_data_member_begin, borrowed>());

static_assert(!check_invocable_lvalue_adl<rvalue_adl_with_data_member_begin, !borrowed>());
static_assert(!check_invocable_rvalue_adl<rvalue_adl_with_data_member_begin, !borrowed>());
static_assert(!check_invocable_lvalue_adl<rvalue_adl_with_data_member_begin, borrowed>());
static_assert(!check_invocable_rvalue_adl<rvalue_adl_with_data_member_begin, borrowed>());

static_assert(!std::invocable<begin_t&, empty&>);
static_assert(!std::invocable<begin_t&, empty const&>);
static_assert(!std::invocable<begin_t&, empty&&>);
static_assert(!std::invocable<begin_t&, empty const&&>);

static_assert(!std::invocable<begin_t&, bad_member_return&>);
static_assert(!std::invocable<begin_t&, bad_member_return const&>);
static_assert(!std::invocable<begin_t&, bad_member_return&&>);
static_assert(!std::invocable<begin_t&, bad_member_return const&&>);

static_assert(std::invocable<begin_t&, member_returns_decayable&>);
static_assert(std::invocable<begin_t&, member_returns_decayable const&>);

static_assert(std::invocable<begin_t&, adl_returns_decayable&>);
static_assert(std::invocable<begin_t&, adl_returns_decayable const&>);

ASSERT_NOEXCEPT(std::ranges::begin(std::declval<nothrow_member_begin<int*>&>()));
ASSERT_NOT_NOEXCEPT(std::ranges::begin(std::declval<nothrow_member_begin<ThrowingIterator<int> >&>()));
ASSERT_NOEXCEPT(std::ranges::begin(std::declval<nothrow_adl_begin<int*>&>()));
ASSERT_NOT_NOEXCEPT(std::ranges::begin(std::declval<nothrow_adl_begin<ThrowingIterator<int> >&>()));

template <typename T, std::same_as<T>... Ts>
constexpr bool check_lvalue_array(T t, Ts... ts) {
  T x[] = {t, ts...};
  return std::ranges::begin(x) == &x[0];
}

template <class T>
[[nodiscard]] constexpr bool check_member_impl() {
  std::remove_cvref_t<T> t;
  return std::ranges::begin(std::forward<T>(t)) == std::forward<T>(t).begin() && t.callee == called::member;
}

template <template <enable_constness, bool> class T, bool is_borrowed>
constexpr void check_member_with_lvalue_param() {
  static_assert(check_member_impl<T<enable_constness::mutable_only, is_borrowed>&>());
  assert((check_member_impl<T<enable_constness::mutable_only, is_borrowed>&>()));
  // static_assert(check_lvalue_member_impl<T<enable_constness::mutable_only, is_borrowed> const&>());
  // assert((check_lvalue_member_impl<T<enable_constness::mutable_only, is_borrowed> const&>()));
  static_assert(check_member_impl<T<enable_constness::const_only, is_borrowed>&>());
  assert((check_member_impl<T<enable_constness::const_only, is_borrowed>&>()));
  static_assert(check_member_impl<T<enable_constness::const_only, is_borrowed> const&>());
  assert((check_member_impl<T<enable_constness::const_only, is_borrowed> const&>()));
  static_assert(check_member_impl<T<enable_constness::both, is_borrowed>&>());
  assert((check_member_impl<T<enable_constness::both, is_borrowed>&>()));
  static_assert(check_member_impl<T<enable_constness::both, is_borrowed> const&>());
  assert((check_member_impl<T<enable_constness::both, is_borrowed> const&>()));
}

template <template <enable_constness, bool> class T, bool is_borrowed>
constexpr void check_member_with_rvalue_param() {
  static_assert(check_member_impl<T<enable_constness::mutable_only, is_borrowed>&&>());
  assert((check_member_impl<T<enable_constness::mutable_only, is_borrowed>&&>()));
  // static_assert(check_rvalue_member_impl<T<enable_constness::mutable_only, is_borrowed> const&&>());
  // assert((check_rvalue_member_impl<T<enable_constness::mutable_only, is_borrowed> const&&>()));
  static_assert(check_member_impl<T<enable_constness::const_only, is_borrowed>&&>());
  assert((check_member_impl<T<enable_constness::const_only, is_borrowed>&&>()));
  static_assert(check_member_impl<T<enable_constness::const_only, is_borrowed> const&&>());
  assert((check_member_impl<T<enable_constness::const_only, is_borrowed> const&&>()));
  static_assert(check_member_impl<T<enable_constness::both, is_borrowed>&&>());
  assert((check_member_impl<T<enable_constness::both, is_borrowed>&&>()));
  static_assert(check_member_impl<T<enable_constness::both, is_borrowed> const&&>());
  assert((check_member_impl<T<enable_constness::both, is_borrowed> const&&>()));
}

template <class T>
[[nodiscard]] constexpr bool check_adl_impl() {
  std::remove_cvref_t<T> t;
  using std::begin;
  return std::ranges::begin(std::forward<T>(t)) == begin(std::forward<T>(t)) && t.callee == called::adl;
}

template <template <enable_constness, bool> class T, bool is_borrowed>
constexpr void check_adl_with_lvalue_param() {
  static_assert(check_adl_impl<T<enable_constness::mutable_only, is_borrowed>&>());
  assert((check_adl_impl<T<enable_constness::mutable_only, is_borrowed>&>()));
  static_assert(check_adl_impl<T<enable_constness::const_only, is_borrowed> const&>());
  assert((check_adl_impl<T<enable_constness::const_only, is_borrowed> const&>()));
  static_assert(check_adl_impl<T<enable_constness::both, is_borrowed>&>());
  assert((check_adl_impl<T<enable_constness::both, is_borrowed>&>()));
  static_assert(check_adl_impl<T<enable_constness::both, is_borrowed> const&>());
  assert((check_adl_impl<T<enable_constness::both, is_borrowed> const&>()));
}

template <template <enable_constness, bool> class T, bool is_borrowed>
constexpr void check_adl_with_rvalue_param() {
  static_assert(check_adl_impl<T<enable_constness::mutable_only, is_borrowed>&&>());
  assert((check_adl_impl<T<enable_constness::mutable_only, is_borrowed>&&>()));
  static_assert(check_adl_impl<T<enable_constness::const_only, is_borrowed> const&&>());
  assert((check_adl_impl<T<enable_constness::const_only, is_borrowed> const&&>()));
  static_assert(check_adl_impl<T<enable_constness::both, is_borrowed>&&>());
  assert((check_adl_impl<T<enable_constness::both, is_borrowed>&&>()));
  static_assert(check_adl_impl<T<enable_constness::both, is_borrowed> const&&>());
  assert((check_adl_impl<T<enable_constness::both, is_borrowed> const&&>()));
}

int main() {
  struct S {
    int x;
  };

  static_assert(check_lvalue_array(0, 1, 2, 3, 4, 5));
  assert((check_lvalue_array(0, 1, 2, 3, 4, 5)));

  static_assert(check_lvalue_array(0.0, 1.0));
  assert((check_lvalue_array(0.0, 1.0)));

  static_assert(check_lvalue_array(S{0}, S{1}, S{2}));
  assert((check_lvalue_array(S{0}, S{1}, S{2})));

  check_member_with_lvalue_param<member_only, !borrowed>();
  check_member_with_lvalue_param<member_only, borrowed>();
  check_member_with_rvalue_param<member_only, borrowed>();

  check_member_with_lvalue_param<lvalue_member_only, !borrowed>();
  check_member_with_lvalue_param<lvalue_member_only, borrowed>();

  check_member_with_lvalue_param<member_preferred, !borrowed>();
  check_member_with_lvalue_param<member_preferred, borrowed>();
  check_member_with_rvalue_param<member_preferred, borrowed>();

  check_member_with_lvalue_param<lvalue_member_preferred, !borrowed>();
  check_member_with_lvalue_param<lvalue_member_preferred, borrowed>();

  check_adl_with_lvalue_param<lvalue_adl_only, !borrowed>();
  check_adl_with_lvalue_param<lvalue_adl_only, borrowed>();

  check_adl_with_lvalue_param<adl_only, !borrowed>();
  check_adl_with_lvalue_param<adl_only, borrowed>();
  check_adl_with_rvalue_param<adl_only, borrowed>();

  check_adl_with_lvalue_param<lvalue_adl_with_private_members, !borrowed>();
  check_adl_with_lvalue_param<lvalue_adl_with_private_members, borrowed>();

  check_adl_with_lvalue_param<lvalue_adl_with_private_lvalue_members, !borrowed>();
  check_adl_with_lvalue_param<lvalue_adl_with_private_lvalue_members, borrowed>();

  check_adl_with_lvalue_param<lvalue_adl_with_private_rvalue_members, !borrowed>();
  check_adl_with_lvalue_param<lvalue_adl_with_private_rvalue_members, borrowed>();

  check_adl_with_lvalue_param<lvalue_adl_with_bad_members, !borrowed>();
  check_adl_with_lvalue_param<lvalue_adl_with_bad_members, borrowed>();

  check_adl_with_lvalue_param<lvalue_adl_with_data_member_begin, !borrowed>();
  check_adl_with_lvalue_param<lvalue_adl_with_data_member_begin, borrowed>();
}
