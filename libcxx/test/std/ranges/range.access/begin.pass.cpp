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

// std::ranges::begin

#include <ranges>

#include <cassert>
#include "test_macros.h"
#include "test_iterators.h"

using RangeBeginT = decltype(std::ranges::begin);

static int globalBuff[8];

struct Incomplete;

static_assert(!std::is_invocable_v<RangeBeginT, Incomplete[]>);
static_assert(!std::is_invocable_v<RangeBeginT, Incomplete[2]>);
static_assert(!std::is_invocable_v<RangeBeginT, Incomplete[2][2]>);
static_assert(!std::is_invocable_v<RangeBeginT, int [1]>);
static_assert(!std::is_invocable_v<RangeBeginT, int (&&)[1]>);
static_assert( std::is_invocable_v<RangeBeginT, int (&)[1]>);
static_assert(!std::is_invocable_v<RangeBeginT, int []>);
static_assert(!std::is_invocable_v<RangeBeginT, int (&&)[]>);
// TODO: This should be valid.
static_assert(!std::is_invocable_v<RangeBeginT, int (&)[]>);

struct BeginMember {
  int x;
  constexpr const int *begin() const { return &x; }
};

// Ensure that we can't call with rvalues with borrowing disabled.
static_assert( std::is_invocable_v<RangeBeginT, BeginMember &>);
static_assert(!std::is_invocable_v<RangeBeginT, BeginMember &&>);

constexpr bool testArray() {
  int a[2];
  assert(std::ranges::begin(a) == a);

  int b[2][2];
  assert(std::ranges::begin(b) == b);

  BeginMember c[2];
  assert(std::ranges::begin(c) == c);

  return true;
}

struct BeginMemberFunction {
  int x;
  constexpr const int *begin() const { return &x; }
  friend constexpr int *begin(BeginMemberFunction const& bf);
};

struct BeginMemberReturnsInt {
  int begin() const;
};

static_assert(!std::is_invocable_v<RangeBeginT, BeginMemberReturnsInt const&>);

struct BeginMemberReturnsVoidPtr {
  const void *begin() const;
};

static_assert(!std::is_invocable_v<RangeBeginT, BeginMemberReturnsVoidPtr const&>);

struct Empty { };
struct EmptyBeginMember {
  Empty begin() const;
};
struct EmptyPtrBeginMember {
  Empty x;
  constexpr const Empty *begin() const { return &x; }
};

static_assert(!std::is_invocable_v<RangeBeginT, EmptyBeginMember const&>);

struct PtrConvertible {
  operator int*() const;
};
struct PtrConvertibleBeginMember {
  PtrConvertible begin() const;
};

static_assert(!std::is_invocable_v<RangeBeginT, PtrConvertibleBeginMember const&>);

struct NonConstBeginMember {
  int x;
  constexpr int *begin() { return &x; }
};

static_assert(!std::is_invocable_v<RangeBeginT, NonConstBeginMember const&>);

struct EnabledBorrowingBeginMember {
  constexpr int *begin() const { return &globalBuff[0]; }
};

template<>
inline constexpr bool std::ranges::enable_borrowed_range<EnabledBorrowingBeginMember> = true;

constexpr bool testBeginMember() {
  BeginMember a;
  assert(std::ranges::begin(a) == &a.x);

  NonConstBeginMember b;
  assert(std::ranges::begin(b) == &b.x);

  EnabledBorrowingBeginMember c;
  assert(std::ranges::begin(std::move(c)) == &globalBuff[0]);

  BeginMemberFunction d;
  assert(std::ranges::begin(d) == &d.x);

  EmptyPtrBeginMember e;
  assert(std::ranges::begin(e) == &e.x);

  return true;
}

struct BeginFunction {
  int x;
  friend constexpr const int *begin(BeginFunction const& bf) { return &bf.x; }
};

static_assert( std::is_invocable_v<RangeBeginT, BeginFunction const&>);
static_assert(!std::is_invocable_v<RangeBeginT, BeginFunction &&>);

struct BeginFunctionWithDataMember {
  int x;
  int begin;
  friend constexpr const int *begin(BeginFunctionWithDataMember const& bf) { return &bf.x; }
};

struct BeginFunctionWithPrivateBeginMember : private BeginMember {
  int y;
  friend constexpr const int *begin(BeginFunctionWithPrivateBeginMember const& bf) { return &bf.y; }
};

struct BeginFunctionReturnsEmptyPtr {
  Empty x;
  friend constexpr const Empty *begin(BeginFunctionReturnsEmptyPtr const& bf) { return &bf.x; }
};

struct BeginFunctionByValue {
  friend constexpr int *begin(BeginFunctionByValue) { return &globalBuff[1]; }
};

struct BeginFunctionEnabledBorrowing {
  friend constexpr int *begin(BeginFunctionEnabledBorrowing) { return &globalBuff[2]; }
};

template<>
inline constexpr bool std::ranges::enable_borrowed_range<BeginFunctionEnabledBorrowing> = true;

struct BeginFunctionReturnsInt {
  friend constexpr int begin(BeginFunctionReturnsInt const&);
};

static_assert(!std::is_invocable_v<RangeBeginT, BeginFunctionReturnsInt const&>);

struct BeginFunctionReturnsVoidPtr {
  friend constexpr void *begin(BeginFunctionReturnsVoidPtr const&);
};

static_assert(!std::is_invocable_v<RangeBeginT, BeginFunctionReturnsVoidPtr const&>);

struct BeginFunctionReturnsEmpty {
  friend constexpr Empty begin(BeginFunctionReturnsEmpty const&);
};

static_assert(!std::is_invocable_v<RangeBeginT, BeginFunctionReturnsEmpty const&>);

struct BeginFunctionReturnsPtrConvertible {
  friend constexpr PtrConvertible begin(BeginFunctionReturnsPtrConvertible const&);
};

static_assert(!std::is_invocable_v<RangeBeginT, BeginFunctionReturnsPtrConvertible const&>);

constexpr bool testBeginFunction() {
  const BeginFunction a{};
  assert(std::ranges::begin(a) == &a.x);

  BeginFunctionByValue b;
  assert(std::ranges::begin(b) == &globalBuff[1]);

  BeginFunctionEnabledBorrowing c;
  assert(std::ranges::begin(std::move(c)) == &globalBuff[2]);

  const BeginFunctionReturnsEmptyPtr d{};
  assert(std::ranges::begin(d) == &d.x);

  const BeginFunctionWithDataMember e{};
  assert(std::ranges::begin(e) == &e.x);

  const BeginFunctionWithPrivateBeginMember f{};
  assert(std::ranges::begin(f) == &f.y);

  return true;
}

int main(int, char**) {
  testArray();
  static_assert(testArray());

  testBeginMember();
  static_assert(testBeginMember());

  testBeginFunction();
  static_assert(testBeginFunction());

  return 0;
}

