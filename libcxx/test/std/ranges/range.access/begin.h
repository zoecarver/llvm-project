//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LIBCXX_STD_TEST_RANGE_ACCESS_BEGIN_H
#define LIBCXX_STD_TEST_RANGE_ACCESS_BEGIN_H

#include <ranges>

// clang-format off

enum class enable_constness { mutable_only, const_only, both };
enum called { neither, member, adl, both };
constexpr bool borrowed = true;

template<enable_constness Const, bool is_borrowed>
struct member_only {
  constexpr int* begin()
  requires (Const != enable_constness::const_only) {
    callee |= called::member;
    return &value;
  }

  constexpr int const* begin() const
  requires (Const != enable_constness::mutable_only) {
    callee |= called::member;
    return &value;
  }

  int value = 0;
  mutable int callee = neither;
};

template<enable_constness Const>
inline constexpr bool std::ranges::enable_borrowed_range<member_only<Const, true> > = true;

template<enable_constness Const, bool>
struct lvalue_member_only {
  constexpr int* begin() &
  requires (Const != enable_constness::const_only) {
    callee |= called::member;
    return &value;
  }

  constexpr int const* begin() const&
  requires (Const != enable_constness::mutable_only) {
    callee |= called::member;
    return &value;
  }

  int value = 0;
  mutable int callee = neither;
};

template<enable_constness Const>
inline constexpr bool
std::ranges::enable_borrowed_range<lvalue_member_only<Const, borrowed> > = true;

template<enable_constness Const, bool>
struct rvalue_member_only {
  constexpr int* begin() &&
  requires (Const != enable_constness::const_only) {
    callee |= called::member;
    return &value;
  }

  constexpr int const* begin() const&&
  requires (Const != enable_constness::mutable_only) {
    callee |= called::member;
    return &value;
  }

  int value = 0;
  mutable int callee = neither;
};

template<enable_constness Const>
inline constexpr bool std::ranges::enable_borrowed_range<rvalue_member_only<Const, borrowed> > = true;

template<enable_constness Const, bool is_borrowed>
struct member_preferred : member_only<Const, is_borrowed> {
  constexpr friend int* begin(member_preferred& x)
  requires (Const != enable_constness::const_only) {
    x.callee |= called::adl;
    return &x.value;
  }

  constexpr friend int const* begin(member_preferred const& x)
  requires (Const != enable_constness::mutable_only) {
    x.callee |= called::adl;
    return &x.value;
  }

  int value = 0;
};

template<enable_constness Const>
inline constexpr bool std::ranges::enable_borrowed_range<member_preferred<Const, borrowed> > = true;

template<enable_constness Const, bool is_borrowed>
struct lvalue_member_preferred : lvalue_member_only<Const, is_borrowed> {
  constexpr friend int* begin(lvalue_member_preferred& x)
  requires (Const != enable_constness::const_only) {
    x.callee |= called::adl;
    return &x.value;
  }

  constexpr friend int const* begin(lvalue_member_preferred const& x)
  requires (Const != enable_constness::mutable_only) {
    x.callee |= called::adl;
    return &x.value;
  }

  int value = 0;
};

template<enable_constness Const>
inline constexpr bool std::ranges::enable_borrowed_range<lvalue_member_preferred<Const, borrowed> > = true;

template<enable_constness Const, bool is_borrowed>
struct rvalue_member_preferred : rvalue_member_only<Const, is_borrowed> {
  constexpr friend int* begin(rvalue_member_preferred&& x)
  requires (Const != enable_constness::const_only) {
    x.callee |= called::adl;
    return &x.value;
  }

  constexpr friend int const* begin(rvalue_member_preferred const&& x)
  requires (Const != enable_constness::mutable_only) {
    x.callee |= called::adl;
    return &x.value;
  }

  int value = 0;
};

template<enable_constness Const>
inline constexpr bool std::ranges::enable_borrowed_range<rvalue_member_preferred<Const, borrowed> > = true;

template<enable_constness Const, bool>
struct lvalue_adl_only {
  constexpr friend int* begin(lvalue_adl_only& x)
  requires (Const != enable_constness::const_only) {
    x.callee |= called::adl;
    return &x.value;
  }

  constexpr friend int const* begin(lvalue_adl_only const& x)
  requires (Const != enable_constness::mutable_only) {
    x.callee |= called::adl;
    return &x.value;
  }

  int value = 0;
  mutable int callee = neither;
};

template<enable_constness Const>
inline constexpr bool std::ranges::enable_borrowed_range<lvalue_adl_only<Const, borrowed> > = true;

template<enable_constness Const, bool>
struct rvalue_adl_only {
  constexpr friend int* begin(rvalue_adl_only&& x)
  requires (Const != enable_constness::const_only) {
    x.callee |= called::adl;
    return &x.value;
  }

  constexpr friend int const* begin(rvalue_adl_only const&& x)
  requires (Const != enable_constness::mutable_only) {
    x.callee |= called::adl;
    return &x.value;
  }

  int value = 0;
  mutable int callee = neither;
};

template<enable_constness Const>
inline constexpr bool std::ranges::enable_borrowed_range<rvalue_adl_only<Const, borrowed> > = true;

template<enable_constness Const, bool>
struct adl_only {
  constexpr friend int* begin(adl_only& x)
  requires (Const != enable_constness::const_only) {
    x.callee |= called::adl;
    return &x.value;
  }

  constexpr friend int const* begin(adl_only const& x)
  requires (Const != enable_constness::mutable_only) {
    x.callee |= called::adl;
    return &x.value;
  }

  constexpr friend int* begin(adl_only&& x)
  requires (Const != enable_constness::const_only) {
    x.callee |= called::adl;
    return &x.value;
  }

  constexpr friend int const* begin(adl_only const&& x)
  requires (Const != enable_constness::mutable_only) {
    x.callee |= called::adl;
    return &x.value;
  }

  int value = 0;
  mutable int callee = neither;
};

template<enable_constness Const>
inline constexpr bool std::ranges::enable_borrowed_range<adl_only<Const, borrowed> > = true;

template<enable_constness Const, bool is_borrowed>
struct lvalue_adl_with_private_members : private member_only<Const, is_borrowed> {
  constexpr friend int* begin(lvalue_adl_with_private_members& x)
  requires (Const != enable_constness::const_only) {
    x.callee |= called::adl;
    return &x.value;
  }

  constexpr friend int const* begin(lvalue_adl_with_private_members const& x)
  requires (Const != enable_constness::mutable_only) {
    x.callee |= called::adl;
    return &x.value;
  }

  int value = 0;
  mutable int callee = neither;
};

template<enable_constness Const>
inline constexpr bool std::ranges::enable_borrowed_range<lvalue_adl_with_private_members<Const, borrowed> > = true;

template<enable_constness Const, bool is_borrowed>
struct lvalue_adl_with_private_lvalue_members : private lvalue_member_only<Const, is_borrowed> {
  constexpr friend int* begin(lvalue_adl_with_private_lvalue_members& x)
  requires (Const != enable_constness::const_only) {
    x.callee |= called::adl;
    return &x.value;
  }

  constexpr friend int const* begin(lvalue_adl_with_private_lvalue_members const& x)
  requires (Const != enable_constness::mutable_only) {
    x.callee |= called::adl;
    return &x.value;
  }

  int value = 0;
  mutable int callee = neither;
};

template<enable_constness Const>
inline constexpr bool std::ranges::enable_borrowed_range<lvalue_adl_with_private_lvalue_members<Const, borrowed> > = true;

template<enable_constness Const, bool is_borrowed>
struct lvalue_adl_with_private_rvalue_members : private rvalue_member_only<Const, is_borrowed> {
  constexpr friend int* begin(lvalue_adl_with_private_rvalue_members& x)
  requires (Const != enable_constness::const_only) {
    x.callee |= called::adl;
    return &x.value;
  }

  constexpr friend int const* begin(lvalue_adl_with_private_rvalue_members const& x)
  requires (Const != enable_constness::mutable_only) {
    x.callee |= called::adl;
    return &x.value;
  }

  int value = 0;
  mutable int callee = neither;
};

template<enable_constness Const>
inline constexpr bool std::ranges::enable_borrowed_range<lvalue_adl_with_private_rvalue_members<Const, borrowed> > = true;

template<enable_constness Const, bool is_borrowed>
struct rvalue_adl_with_private_members : private member_only<Const, is_borrowed> {
  constexpr friend int* begin(rvalue_adl_with_private_members&& x)
  requires (Const != enable_constness::const_only) {
    x.callee |= called::adl;
    return &x.value;
  }

  constexpr friend int const* begin(rvalue_adl_with_private_members const&& x)
  requires (Const != enable_constness::mutable_only) {
    x.callee |= called::adl;
    return &x.value;
  }

  int value = 0;
  mutable int callee = neither;
};

template<enable_constness Const>
inline constexpr bool std::ranges::enable_borrowed_range<rvalue_adl_with_private_members<Const, borrowed> > = true;

template<enable_constness Const, bool is_borrowed>
struct rvalue_adl_with_private_lvalue_members : private lvalue_member_only<Const, is_borrowed> {
  constexpr friend int* begin(rvalue_adl_with_private_lvalue_members&& x)
  requires (Const != enable_constness::const_only) {
    x.callee |= called::adl;
    return &x.value;
  }

  constexpr friend int const* begin(rvalue_adl_with_private_lvalue_members const&& x)
  requires (Const != enable_constness::mutable_only) {
    x.callee |= called::adl;
    return &x.value;
  }

  int value = 0;
  mutable int callee = neither;
};

template<enable_constness Const>
inline constexpr bool std::ranges::enable_borrowed_range<rvalue_adl_with_private_lvalue_members<Const, borrowed> > = true;

template<enable_constness Const, bool is_borrowed>
struct rvalue_adl_with_private_rvalue_members : private rvalue_member_only<Const, is_borrowed> {
  constexpr friend int* begin(rvalue_adl_with_private_rvalue_members&& x)
  requires (Const != enable_constness::const_only) {
    x.callee |= called::adl;
    return &x.value;
  }

  constexpr friend int const* begin(rvalue_adl_with_private_rvalue_members const&& x)
  requires (Const != enable_constness::mutable_only) {
    x.callee |= called::adl;
    return &x.value;
  }

  int value = 0;
  mutable int callee = neither;
};

template<enable_constness Const>
inline constexpr bool std::ranges::enable_borrowed_range<rvalue_adl_with_private_rvalue_members<Const, borrowed> > = true;

template<enable_constness Const, bool>
struct lvalue_adl_with_bad_members {
  constexpr int begin() {
    callee |= called::member;
    return value;
  }

  constexpr int begin() const {
    callee |= called::member;
    return value;
  }

  constexpr friend int* begin(lvalue_adl_with_bad_members& x)
  requires (Const != enable_constness::const_only) {
    x.callee |= called::adl;
    return &x.value;
  }

  constexpr friend int const* begin(lvalue_adl_with_bad_members const& x)
  requires (Const != enable_constness::mutable_only) {
    x.callee |= called::adl;
    return &x.value;
  }

  int value = 0;
  mutable int callee = called::neither;
};

template<enable_constness Const>
inline constexpr bool std::ranges::enable_borrowed_range<lvalue_adl_with_bad_members<Const, borrowed> > = true;

template<enable_constness Const, bool>
struct rvalue_adl_with_bad_members {
  constexpr int begin() {
    callee |= called::member;
    return value;
  }

  constexpr int begin() const {
    callee |= called::member;
    return value;
  }

  constexpr friend int* begin(rvalue_adl_with_bad_members&& x)
  requires (Const != enable_constness::const_only) {
    x.callee |= called::adl;
    return &x.value;
  }

  constexpr friend int const* begin(rvalue_adl_with_bad_members const&& x)
  requires (Const != enable_constness::mutable_only) {
    x.callee |= called::adl;
    return &x.value;
  }

  int value = 0;
  mutable int callee = called::neither;
};

template<enable_constness Const>
inline constexpr bool std::ranges::enable_borrowed_range<rvalue_adl_with_bad_members<Const, borrowed> > = true;

template<enable_constness Const, bool>
struct lvalue_adl_with_data_member_begin {
  constexpr friend int* begin(lvalue_adl_with_data_member_begin& x)
  requires (Const != enable_constness::const_only) {
    x.callee |= called::adl;
    return &x.value;
  }

  constexpr friend int const* begin(lvalue_adl_with_data_member_begin const& x)
  requires (Const != enable_constness::mutable_only) {
    x.callee |= called::adl;
    return &x.value;
  }

  int value = 0;
  int begin = 0;
  mutable int callee = called::neither;
};

template<enable_constness Const>
inline constexpr bool std::ranges::enable_borrowed_range<lvalue_adl_with_data_member_begin<Const, borrowed> > = true;

template<enable_constness Const, bool>
struct rvalue_adl_with_data_member_begin {
  constexpr friend int* begin(rvalue_adl_with_data_member_begin&& x)
  requires (Const != enable_constness::const_only) {
    x.callee |= called::adl;
    return &x.value;
  }

  constexpr friend int const* begin(rvalue_adl_with_data_member_begin const&& x)
  requires (Const != enable_constness::mutable_only) {
    x.callee |= called::adl;
    return &x.value;
  }

  int value = 0;
  int begin = 0;
  mutable int callee = called::neither;
};

template<enable_constness Const>
inline constexpr bool std::ranges::enable_borrowed_range<rvalue_adl_with_data_member_begin<Const, borrowed> > = true;

struct empty {};
template<> inline constexpr bool std::ranges::enable_borrowed_range<empty> = true;

struct bad_member_return {
  int begin();
  int begin() const;
};

template<> inline constexpr bool std::ranges::enable_borrowed_range<bad_member_return> = true;

struct member_returns_decayable {
  int*& begin();
  int*& begin() const;
};

struct adl_returns_decayable {
  friend int*& begin(adl_returns_decayable&);
  friend int*& begin(adl_returns_decayable const&);
};

template<class T>
struct nothrow_member_begin {
  T begin() noexcept;
  T begin() const noexcept;
};

template<class T>
struct nothrow_adl_begin {
  friend T begin(nothrow_adl_begin&) noexcept;
  friend T begin(nothrow_adl_begin const&) noexcept;
};

// clang-format on

#endif // LIBCXX_STD_TEST_RANGE_ACCESS_BEGIN_H
