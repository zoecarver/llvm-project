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

// std::ranges::ref_view

#include <ranges>

#include <cassert>
#include "test_macros.h"
#include "test_iterators.h"

namespace ranges = std::ranges;

int globalBuff[8];

template<class T>
concept ValidRefView = requires { typename ranges::ref_view<T>; };

struct Range {
  int start = 0;
  constexpr friend int* begin(Range const& range) { return globalBuff + range.start; }
  constexpr friend int* end(Range const&) { return globalBuff + 8; }
  constexpr friend int* begin(Range& range) { return globalBuff + range.start; }
  constexpr friend int* end(Range&) { return globalBuff + 8; }
};

struct BeginOnly {
  friend int* begin(BeginOnly const&);
  friend int* begin(BeginOnly &);
};

static_assert( ValidRefView<Range>);
static_assert(!ValidRefView<BeginOnly>);
static_assert(!ValidRefView<int (&)[4]>);

static_assert(std::derived_from<ranges::ref_view<Range>, ranges::view_interface<ranges::ref_view<Range>>>);
static_assert(std::semiregular<ranges::ref_view<Range>>);

struct RangeConvertible {
  operator Range& ();
};

struct RValueRangeConvertible {
  operator Range&& ();
};

static_assert( std::is_constructible_v<ranges::ref_view<Range>, Range&>);
static_assert( std::is_constructible_v<ranges::ref_view<Range>, RangeConvertible>);
static_assert(!std::is_constructible_v<ranges::ref_view<Range>, RValueRangeConvertible>);

struct ForwardRange {
  constexpr forward_iterator<int*> begin() const { return forward_iterator<int*>(globalBuff); }
  constexpr forward_iterator<int*> end() const { return forward_iterator<int*>(globalBuff + 8); }
};

struct Cpp17InputRange {
  constexpr cpp17_input_iterator<int*> begin() const {
    return cpp17_input_iterator<int*>(globalBuff);
  }
  constexpr int* end() const { return globalBuff + 8; }
};

bool operator==(cpp17_input_iterator<int*> lhs, int* rhs) { return lhs.base() == rhs; }
bool operator==(int* lhs, cpp17_input_iterator<int*> rhs) { return rhs.base() == lhs; }

struct Cpp20InputRange {
  constexpr cpp20_input_iterator<int*> begin() const {
    return cpp20_input_iterator<int*>(globalBuff);
  }
  constexpr int* end() const { return globalBuff + 8; }
};

bool operator==(const cpp20_input_iterator<int*> &lhs, int* rhs) { return lhs.base() == rhs; }
bool operator==(int* lhs, const cpp20_input_iterator<int*> &rhs) { return rhs.base() == lhs; }

template<>
inline constexpr bool ranges::enable_borrowed_range<Cpp20InputRange> = true;

template<class R>
concept EmptyInvocable = requires (ranges::ref_view<R> view) { view.empty(); };

template<class R>
concept SizeInvocable = requires (ranges::ref_view<R> view) { view.size(); };

template<class R>
concept DataInvocable = requires (ranges::ref_view<R> view) { view.data(); };

// Testing ctad.
static_assert(std::same_as<decltype(ranges::ref_view(std::declval<Range&>())),
              ranges::ref_view<Range>>);

constexpr bool test() {
  {
    // ref_view::base
    Range range;
    ranges::ref_view<Range> view{range};
    assert(view.begin() == globalBuff);
    view.base() = Range{2};
    assert(view.begin() == globalBuff + 2);
  }

  {
    // ref_view::begin
    Range range1;
    ranges::ref_view<Range> view1{range1};
    assert(view1.begin() == globalBuff);

    ForwardRange range2;
    ranges::ref_view<ForwardRange> view2{range2};
    assert(view2.begin().base() == globalBuff);

    Cpp17InputRange range3;
    ranges::ref_view<Cpp17InputRange> view3{range3};
    assert(view3.begin().base() == globalBuff);

    Cpp20InputRange range4;
    ranges::ref_view<Cpp20InputRange> view4{range4};
    assert(view4.begin().base() == globalBuff);
  }

  {
    // ref_view::end
    Range range1;
    ranges::ref_view<Range> view1{range1};
    assert(view1.end() == globalBuff + 8);

    ForwardRange range2;
    ranges::ref_view<ForwardRange> view2{range2};
    assert(view2.end().base() == globalBuff + 8);

    Cpp17InputRange range3;
    ranges::ref_view<Cpp17InputRange> view3{range3};
    assert(view3.end() == globalBuff + 8);

    Cpp20InputRange range4;
    ranges::ref_view<Cpp20InputRange> view4{range4};
    assert(view4.end() == globalBuff + 8);
  }

  {
    // ref_view::empty
    Range range{8};
    ranges::ref_view<Range> view1{range};
    assert(view1.empty());

    ForwardRange range2;
    ranges::ref_view<ForwardRange> view2{range2};
    assert(!view2.empty());

    static_assert(!EmptyInvocable<Cpp17InputRange>);
    static_assert(!EmptyInvocable<Cpp20InputRange>);
  }

  {
    // ref_view::size
    Range range1{8};
    ranges::ref_view<Range> view1{range1};
    assert(view1.size() == 0);

    Range range2{2};
    ranges::ref_view<Range> view2{range2};
    assert(view2.size() == 6);

    static_assert(!SizeInvocable<ForwardRange>);
  }

  {
    // ref_view::data
    Range range1;
    ranges::ref_view<Range> view1{range1};
    assert(view1.data() == globalBuff);

    Range range2{2};
    ranges::ref_view<Range> view2{range2};
    assert(view2.data() == globalBuff + 2);

    static_assert(!DataInvocable<ForwardRange>);
  }

  return true;
}

int main(int, char**) {
  test();
  static_assert(test());

  return 0;
}
