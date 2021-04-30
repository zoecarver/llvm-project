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

namespace ranges = std::ranges;

template<class T>
concept ValidViewInterfaceType = requires { typename ranges::view_interface<T>; };

struct Empty { };

static_assert(!ValidViewInterfaceType<void>);
static_assert(!ValidViewInterfaceType<void*>);
static_assert(!ValidViewInterfaceType<Empty*>);
static_assert(!ValidViewInterfaceType<Empty const>);
static_assert(!ValidViewInterfaceType<Empty &>);
static_assert( ValidViewInterfaceType<Empty>);

static_assert(std::derived_from<ranges::view_interface<Empty>, ranges::view_base>);

using InputIter = cpp20_input_iterator<const int*>;

struct InputRange : ranges::view_interface<InputRange> {
  int buff[8] = {0, 1, 2, 3, 4, 5, 6, 7};
  constexpr InputIter begin() const { return InputIter(buff); }
  constexpr InputIter end() const { return InputIter(buff + 8); }
};

struct NotSizedSentinel {
  using I = int*;
  using value_type = std::iter_value_t<I>;
  using difference_type = std::iter_difference_t<I>;
  using iterator_concept = std::forward_iterator_tag;

  NotSizedSentinel() = default;
  explicit constexpr NotSizedSentinel(I);

  constexpr int &operator*() const { return *value; };
  NotSizedSentinel& operator++();
  NotSizedSentinel operator++(int);
  bool operator==(NotSizedSentinel const&) const;

  int *value;
};
static_assert(std::forward_iterator<NotSizedSentinel>);

using ForwardIter = forward_iterator<int*>;

// So that we conform to sized_sentinel_for.
constexpr std::ptrdiff_t operator-(const ForwardIter& x, const ForwardIter& y) {
    return x.base() - y.base();
}

struct ForwardRange : ranges::view_interface<ForwardRange> {
  int buff[8] = {0, 1, 2, 3, 4, 5, 6, 7};
  constexpr ForwardIter begin() const { return ForwardIter(const_cast<int*>(buff)); }
  constexpr ForwardIter end() const { return ForwardIter(const_cast<int*>(buff) + 8); }
};

struct MoveOnlyForwardRange : ranges::view_interface<MoveOnlyForwardRange> {
  int buff[8] = {0, 1, 2, 3, 4, 5, 6, 7};
  MoveOnlyForwardRange(MoveOnlyForwardRange const&) = delete;
  MoveOnlyForwardRange(MoveOnlyForwardRange &&) = default;
  MoveOnlyForwardRange() = default;
  constexpr ForwardIter begin() const { return ForwardIter(const_cast<int*>(buff)); }
  constexpr ForwardIter end() const { return ForwardIter(const_cast<int*>(buff) + 8); }
};

struct EmptyIsTrue : ranges::view_interface<EmptyIsTrue> {
  int buff[8] = {0, 1, 2, 3, 4, 5, 6, 7};
  constexpr ForwardIter begin() const { return ForwardIter(const_cast<int*>(buff)); }
  constexpr ForwardIter end() const { return ForwardIter(const_cast<int*>(buff) + 8); }
  constexpr bool empty() const { return true; }
};

struct SizeIsTen : ranges::view_interface<SizeIsTen> {
  int buff[8] = {0, 1, 2, 3, 4, 5, 6, 7};
  constexpr ForwardIter begin() const { return ForwardIter(const_cast<int*>(buff)); }
  constexpr ForwardIter end() const { return ForwardIter(const_cast<int*>(buff) + 8); }
  constexpr size_t size() const { return 10; }
};

using RAIter = random_access_iterator<int*>;

struct RARange : ranges::view_interface<RARange> {
  int buff[8] = {0, 1, 2, 3, 4, 5, 6, 7};
  constexpr RAIter begin() const { return RAIter(const_cast<int*>(buff)); }
  constexpr RAIter end() const { return RAIter(const_cast<int*>(buff) + 8); }
};

using ContIter = contiguous_iterator<const int*>;

struct ContRange : ranges::view_interface<ContRange> {
  int buff[8] = {0, 1, 2, 3, 4, 5, 6, 7};
  constexpr ContIter begin() const { return ContIter(buff); }
  constexpr ContIter end() const { return ContIter(buff + 8); }
};

struct DataIsNull : ranges::view_interface<DataIsNull> {
  int buff[8] = {0, 1, 2, 3, 4, 5, 6, 7};
  constexpr ContIter begin() const { return ContIter(buff); }
  constexpr ContIter end() const { return ContIter(buff + 8); }
  constexpr const int *data() const { return nullptr; }
};

template<class T>
concept EmptyInvocable = requires (T const& obj) { obj.empty(); };

template<class T>
concept BoolOpInvocable = requires (T const& obj) { bool(obj); };

constexpr bool testEmpty() {
  static_assert(!EmptyInvocable<InputRange>);
  static_assert( EmptyInvocable<ForwardRange>);

  static_assert(!BoolOpInvocable<InputRange>);
  static_assert( BoolOpInvocable<ForwardRange>);

  ForwardRange forwardRange;
  assert(!forwardRange.empty());
  assert(!static_cast<ForwardRange const&>(forwardRange).empty());

  assert(forwardRange);
  assert(static_cast<ForwardRange const&>(forwardRange));

  assert(!ranges::empty(forwardRange));
  assert(!ranges::empty(static_cast<ForwardRange const&>(forwardRange)));

  EmptyIsTrue emptyTrue;
  assert(emptyTrue.empty());
  assert(static_cast<EmptyIsTrue const&>(emptyTrue).empty());
  assert(!emptyTrue.ranges::view_interface<EmptyIsTrue>::empty());

  assert(!emptyTrue);
  assert(!static_cast<EmptyIsTrue const&>(emptyTrue));
  assert(!emptyTrue.ranges::view_interface<EmptyIsTrue>::operator bool());

  assert(ranges::empty(emptyTrue));
  assert(ranges::empty(static_cast<EmptyIsTrue const&>(emptyTrue)));

  // Try calling empty on an rvalue.
  MoveOnlyForwardRange moveOnly;
  assert(!std::move(moveOnly).empty());

  return true;
}

template<class T>
concept DataInvocable = requires (T const& obj) { obj.data(); };

constexpr bool testData() {
  static_assert(!DataInvocable<ForwardRange>);
  static_assert( DataInvocable<ContRange>);

  ContRange contiguous;
  assert(contiguous.data() == contiguous.buff);
  assert(static_cast<ContRange const&>(contiguous).data() == contiguous.buff);

  assert(ranges::data(contiguous) == contiguous.buff);
  assert(ranges::data(static_cast<ContRange const&>(contiguous)) == contiguous.buff);

  DataIsNull dataNull;
  assert(dataNull.data() == nullptr);
  assert(static_cast<DataIsNull const&>(dataNull).data() == nullptr);
  assert(dataNull.ranges::view_interface<DataIsNull>::data() == dataNull.buff);

// TODO: why is the child member picked?
//   assert(ranges::data(dataNull) == nullptr);
//   assert(ranges::data(static_cast<DataIsNull const&>(dataNull)) == nullptr);

  return true;
}

template<class T>
concept SizeInvocable = requires (T const& obj) { obj.size(); };

constexpr bool testSize() {
  static_assert(!SizeInvocable<InputRange>);
  static_assert(!SizeInvocable<NotSizedSentinel>);
  static_assert( SizeInvocable<ForwardRange>);

  ForwardRange forwardRange;
  assert(forwardRange.size() == 8);
  assert(static_cast<ForwardRange const&>(forwardRange).size() == 8);

  assert(ranges::size(forwardRange) == 8);
  assert(ranges::size(static_cast<ForwardRange const&>(forwardRange)) == 8);

  SizeIsTen sizeTen;
  assert(sizeTen.size() == 10);
  assert(static_cast<SizeIsTen const&>(sizeTen).size() == 10);
  assert(sizeTen.ranges::view_interface<SizeIsTen>::size() == 8);

  assert(ranges::size(sizeTen) == 10);
  assert(ranges::size(static_cast<SizeIsTen const&>(sizeTen)) == 10);

  return true;
}

template<class T>
concept SubscriptInvocable = requires (T const& obj, size_t n) { obj[n]; };

constexpr bool testSubscript() {
  static_assert(!SubscriptInvocable<ForwardRange>);
  static_assert( SubscriptInvocable<RARange>);

  RARange randomAccess;
  assert(randomAccess[2] == 2);
  assert(static_cast<RARange const&>(randomAccess)[2] == 2);
  randomAccess[2] = 3;
  assert(randomAccess[2] == 3);

  return true;
}

template<class T>
concept FrontInvocable = requires (T const& obj) { obj.front(); };

template<class T>
concept BackInvocable = requires (T const& obj) { obj.back(); };

constexpr bool testFrontBack() {
  static_assert(!FrontInvocable<InputRange>);
  static_assert( FrontInvocable<ForwardRange>);
  static_assert(!BackInvocable<ForwardRange>);
  static_assert( BackInvocable<RARange>);

  ForwardRange forwardRange;
  assert(forwardRange.front() == 0);
  assert(static_cast<ForwardRange const&>(forwardRange).front() == 0);
  forwardRange.front() = 2;
  assert(forwardRange.front() == 2);

  RARange randomAccess;
  assert(randomAccess.front() == 0);
  assert(static_cast<RARange const&>(randomAccess).front() == 0);
  randomAccess.front() = 2;
  assert(randomAccess.front() == 2);

  assert(randomAccess.back() == 7);
  assert(static_cast<RARange const&>(randomAccess).back() == 7);
  randomAccess.back() = 2;
  assert(randomAccess.back() == 2);

  return true;
}

int main(int, char**) {
  testEmpty();
  static_assert(testEmpty());

  testData();
  static_assert(testData());

  testSize();
  static_assert(testSize());

  testSubscript();
  static_assert(testSubscript());

  testFrontBack();
  static_assert(testFrontBack());

  return 0;
}
