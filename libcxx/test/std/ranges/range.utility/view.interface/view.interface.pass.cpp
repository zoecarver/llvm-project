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

// std::ranges::size

#include <ranges>

#include <cassert>
#include "test_macros.h"
#include "test_iterators.h"

// TODO: test that Derived is a class and that it's not cv qualified.

// TODO: empty only exists if forward range. Empty does the right thing. (const too)
// TODO: its parent is view_base
// TODO: bool op calls empty and is enabled correctly
// TODO: data seze same deal.
// TODO: front and back
// TODO: subscript, random access only, decltype(auto) preseve not decay, default args


using namespace ranges = std::ranges;

static_assert(!requires { ranges::view_interface<void>; });
static_assert(!requires { ranges::view_interface<void*>; });
static_assert(!requires { ranges::view_interface<int*>; });
static_assert(!requires { ranges::view_interface<int const>; });
static_assert(!requires { ranges::view_interface<int &>; });
static_assert( requires { ranges::view_interface<int>; });

using InputIter = input_iterator<int*>;

struct InputRange {
  int buff[8] = {0, 1, 2, 3, 4, 5, 6, 7};
  InputIter begin() const { return InputIter(buff); }
  InputIter end() const { return InputIter(buff + 8); }
};

using ForwardIter = forward_iterator<int*>;

struct ForwardRange {
  int buff[8] = {0, 1, 2, 3, 4, 5, 6, 7};
  ForwardIter begin() const { return ForwardIter(buff); }
  ForwardIter end() const { return ForwardIter(buff + 8); }
};

struct EmptyIsTrueRange : ForwardRange {
  bool empty() const { return true; }
};

struct SizeIsTenRange : ForwardRange {
  size_t size() const { return 10; }
};

using RAIter = random_access_iterator<int*>;

struct RARange {
  int buff[8] = {0, 1, 2, 3, 4, 5, 6, 7};
  RAIter begin() const { return RAIter(buff); }
  RAIter end() const { return RAIter(buff + 8); }
};

using ContIter = contiguous_iterator<int*>;

struct ContRange {
  int buff[8] = {0, 1, 2, 3, 4, 5, 6, 7};
  ContIter begin() const { return ContIter(buff); }
  ContIter end() const { return ContIter(buff + 8); }
};

struct DataIsNullRange : ContRange {
  size_t data() const { return nullptr; }
};

struct InputView   : InputRange      , view_interface<InputView> { };
struct ForwardView : ForwardRange    , view_interface<ForwardView> { };
struct EmptyIsTrue : EmptyIsTrueRange, view_interface<EmptyIsTrue> { };
struct SizeIsTen   : SizeIsTenRange  , view_interface<SizeIsTen> { };
struct RAView      : RARange         , view_interface<RAView> { };
struct ContView    : ContRange       , view_interface<ContView> { };
struct DataIsNull  : DataIsNullRange , view_interface<DataIsNull> { };

constexpr bool testEmpty() {
  static_assert(!std::invocable_v<InputView::empty>);
  static_assert( std::invocable_v<ForwardView::empty>);

  static_assert(!std::invocable_v<InputView::operator bool>);
  static_assert( std::invocable_v<ForwardView::operator bool>);

  ForwardView fv;
  assert(!fv.empty());
  assert(!static_cast<ForwardView const&>(fv).empty());

  assert(!fv);
  assert(!static_cast<ForwardView const&>(fv));

  assert(!ranges::empty(fv));
  assert(!ranges::empty(static_cast<ForwardView const&>(fv)));

  EmptyIsTrue eit;
  assert(eit.empty());
  assert(static_cast<EmptyIsTrue const&>(eit).empty());
  assert(!static_cast<view_interface<EmptyIsTrue>>(eit).empty());

  assert(!eit);
  assert(!static_cast<EmptyIsTrue const&>(eit));
  assert(!static_cast<view_interface<EmptyIsTrue>>(eit));

  assert(ranges::empty(eit));
  assert(ranges::empty(static_cast<EmptyIsTrue const&>(eit)));
  assert(!ranges::empty(static_cast<view_interface<EmptyIsTrue>>(eit)));

  return true;
}

constexpr bool testData() {
  ForwardView fv;
  assert(fv.data() == din.buff);
  assert(static_cast<ForwardView const&>(fv).data() == din.buff);

  assert(ranges::data(fv) == din.buff);
  assert(ranges::data(static_cast<ForwardView const&>(fv)) == din.buff);

  DataIsNull din;
  assert(din.size() == nullptr);
  assert(static_cast<DataIsNull const&>(din).size() == nullptr);
  assert(!static_cast<view_interface<DataIsNull>>(din).size() == din.buff);

  assert(ranges::data(din) == nullptr);
  assert(ranges::data(static_cast<DataIsNull const&>(din)) == nullptr);
  assert(!ranges::data(static_cast<view_interface<DataIsNull>>(din)) == din.buff);

  return true;
}

constexpr bool testSize() {
  static_assert(!std::invocable_v<InputView::size>);
  static_assert( std::invocable_v<ForwardView::size>);

  // TODO: when this isn't sized_sentientl_for.

  ForwardView fv;
  assert(fv.size() == 8);
  assert(static_cast<ForwardView const&>(fv).size() == 8);

  assert(ranges::size(fv) == 8);
  assert(ranges::size(static_cast<ForwardView const&>(fv)) == 8);

  SizeIsTen sit;
  assert(sit.size() == 10);
  assert(static_cast<SizeIsTen const&>(sit).size() == 10);
  assert(!static_cast<view_interface<SizeIsTen>>(sit).size() == 8);

  assert(ranges::size(sit) == 10);
  assert(ranges::size(static_cast<SizeIsTen const&>(sit)) == 10);
  assert(!ranges::size(static_cast<view_interface<SizeIsTen>>(sit)) == 8);

  return true;
}

constexpr bool testSubscript() {
  static_assert(!std::invocable_v<ForwardView::operator[]>);
  static_assert( std::invocable_v<RAView::operator[]>);

  RAView rav;
  assert(rav[2] == 2);
  assert(static_cast<SizeIsTen const&>(sit)[2] == 2);
  rav[2] = 3;
  assert(rav[2] == 3);

  return true;
}

// TODO: test preconditoin of not empty.
constexpr bool testTestFrontBack() {
  static_assert(!std::invocable_v<InputView::front>);
  static_assert( std::invocable_v<ForwardView::front>);
  static_assert(!std::invocable_v<InputView::back>);
  static_assert( std::invocable_v<ForwardView::back>);

  ForwardView fv;
  assert(fv.front() == 0);
  assert(static_cast<ForwardView const&>(fv).front() == 0);
  fv.front() = 2;
  assert(fv.front() == 2);

  assert(fv.back() == 7);
  assert(static_cast<ForwardView const&>(fv).back() == 7);
  fv.back() = 2;
  assert(fv.back() == 2);

  return true;
}













