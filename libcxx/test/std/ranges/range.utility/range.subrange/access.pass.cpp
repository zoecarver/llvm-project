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

// class std::ranges::subrange;

#include <ranges>

#include <cassert>
#include "test_macros.h"
#include "test_iterators.h"

namespace ranges = std::ranges;

// Note: begin and end tested in range.subrange.ctor.pass.cpp.

int globalBuff[8];

struct MoveOnlyForwardIter {
    typedef std::forward_iterator_tag       iterator_category;
    typedef int                             value_type;
    typedef std::ptrdiff_t                  difference_type;
    typedef int*                            pointer;
    typedef int&                            reference;
    typedef MoveOnlyForwardIter             self;

    int *base = nullptr;

    MoveOnlyForwardIter() = default;
    MoveOnlyForwardIter(MoveOnlyForwardIter &&) = default;
    MoveOnlyForwardIter &operator=(MoveOnlyForwardIter&&) = default;
    MoveOnlyForwardIter(MoveOnlyForwardIter const&) = delete;
    constexpr MoveOnlyForwardIter(int *ptr) : base(ptr) { }

    friend bool operator==(const self&, const self&);
    constexpr friend bool operator==(const self& lhs, int* rhs) { return lhs.base == rhs; }

    reference operator*() const;
    pointer operator->() const;
    self& operator++();
    self operator++(int);
    self& operator--();
    self operator--(int);

    constexpr operator pointer() const { return base; }
};

using ForwardIter = forward_iterator<int*>;

using std::ptrdiff_t;
struct SizedSentinelForwardIter {
    typedef std::forward_iterator_tag       iterator_category;
    typedef int                             value_type;
    typedef ptrdiff_t                       difference_type;
    typedef int*                            pointer;
    typedef int&                            reference;
    typedef std::make_unsigned_t<ptrdiff_t> udifference_type;
    typedef SizedSentinelForwardIter        self;

    int *base;

    SizedSentinelForwardIter() = default;
    constexpr SizedSentinelForwardIter(int *ptr) : base(ptr) { }

    constexpr friend bool operator==(const self& lhs, const self& rhs) { return lhs.base == rhs.base; }

    reference operator*() const;
    pointer operator->() const;
    self& operator++();
    self operator++(int);
    self& operator--();
    self operator--(int);

    constexpr friend difference_type operator-(SizedSentinelForwardIter const&a,
                                               SizedSentinelForwardIter const&b) {
      return a.base - b.base;
    }
};

constexpr bool testPrimatives() {
  ranges::subrange<MoveOnlyForwardIter, int*> a(MoveOnlyForwardIter(globalBuff), globalBuff + 8, 8);
  assert(a.begin().base == globalBuff);
  assert(!a.empty());
  assert(a.size() == 8);

  ranges::subrange<ForwardIter> b(ForwardIter(nullptr), ForwardIter(nullptr));
  assert(b.empty());

  ranges::subrange<ForwardIter> c{ForwardIter(globalBuff), ForwardIter(globalBuff)};
  assert(c.empty());

  ranges::subrange<ForwardIter> d(ForwardIter(globalBuff), ForwardIter(globalBuff + 1));
  assert(!d.empty());

  ranges::subrange<SizedSentinelForwardIter> e(SizedSentinelForwardIter(globalBuff),
                                               SizedSentinelForwardIter(globalBuff + 8), 8);
  assert(!e.empty());
  assert(e.size() == 8);

  return true;
}

using InputIter = cpp17_input_iterator<int*>;

using BidirIter = bidirectional_iterator<int*>;

constexpr bool testAdvance() {
  ranges::subrange<int*> a(globalBuff, globalBuff + 8, 8);
  auto a1 = a.next();
  assert(a1.begin() == globalBuff + 1);
  assert(a1.size() == 7);
  auto a5 = a.next(5);
  assert(a5.begin() == globalBuff + 5);
  assert(a5.size() == 3);
  auto a4 = a5.prev();
  assert(a4.begin() == globalBuff + 4);
  assert(a4.size() == 4);

  ranges::subrange<InputIter> b(InputIter(globalBuff), InputIter(globalBuff + 8));
  auto b1 = std::move(b).next();
  assert(b1.begin().base() == globalBuff + 1);

  ranges::subrange<BidirIter> c(BidirIter(globalBuff + 4), BidirIter(globalBuff + 8));
  auto c1 = c.prev();
  assert(c1.begin().base() == globalBuff + 3);
  auto c2 = c.prev(4);
  assert(c2.begin().base() == globalBuff);

  ranges::subrange<BidirIter> d(BidirIter(globalBuff + 4), BidirIter(globalBuff + 8));
  auto d1 = d.advance(4);
  assert(d1.begin().base() == globalBuff + 8);
  assert(d1.empty());
  auto d2 = d1.advance(-4);
  assert(d2.begin().base() == globalBuff + 4);

  return true;
}

template<size_t I, class S>
concept GetInvocable = requires {
  std::get<I>(std::declval<S>());
};

static_assert( GetInvocable<0, ranges::subrange<int*>>);
static_assert( GetInvocable<1, ranges::subrange<int*>>);
static_assert(!GetInvocable<2, ranges::subrange<int*>>);
static_assert(!GetInvocable<3, ranges::subrange<int*>>);

constexpr bool testGet() {
  ranges::subrange<int*> a(globalBuff, globalBuff + 8, 8);
  assert(std::get<0>(a) == a.begin());
  assert(std::get<1>(a) == a.end());

  assert(a.begin() == std::get<0>(std::move(a)));
  ranges::subrange<int*> b(globalBuff, globalBuff + 8, 8);
  assert(b.end() == std::get<1>(std::move(b)));

  return true;
}

int main(int, char**) {
  testPrimatives();
  static_assert(testPrimatives());

  testAdvance();
  static_assert(testAdvance());

  testGet();
  static_assert(testGet());

  return 0;
}
