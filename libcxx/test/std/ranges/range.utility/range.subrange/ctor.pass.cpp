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
#include <tuple>
#include "test_macros.h"
#include "test_iterators.h"

namespace ranges = std::ranges;

// convertible-to-non-slicing cases:
//   1. Not convertible (fail)
//   2. Only one is a pointer (succeed)
//   3. Both are not pointers (succeed)
//   4. Pointer elements are different types (fail)
//   5. Pointer elements are same type (succeed)

int globalBuff[8];

struct Empty {};

using ForwardIter = forward_iterator<int*>;
using FowardSubrange = ranges::subrange<ForwardIter, ForwardIter, std::ranges::subrange_kind::unsized>;

struct ConvertibleForwardIter {
    typedef std::forward_iterator_tag       iterator_category;
    typedef int                             value_type;
    typedef std::ptrdiff_t                  difference_type;
    typedef int*                            pointer;
    typedef int&                            reference;
    typedef ConvertibleForwardIter          self;

    int *base = nullptr;

    constexpr ConvertibleForwardIter() = default;
    constexpr explicit ConvertibleForwardIter(int *ptr) : base(ptr) { }

    friend bool operator==(const self&, const self&);

    reference operator*() const;
    pointer operator->() const;
    self& operator++();
    self operator++(int);
    self& operator--();
    self operator--(int);

    constexpr operator pointer() const { return base; }

    // Explicitly deleted so this doesn't model sized_sentinel_for.
    friend constexpr difference_type operator-(int *, self const&) = delete;
    friend constexpr difference_type operator-(self const&, int*) = delete;
};
using ConvertibleFowardSubrange = ranges::subrange<ConvertibleForwardIter, int*,
                                                   std::ranges::subrange_kind::unsized>;
static_assert(std::is_convertible_v<ConvertibleForwardIter, int*>);

using std::ptrdiff_t;
template<bool EnableConvertible>
struct SizedSentinelForwardIterBase {
    typedef std::forward_iterator_tag       iterator_category;
    typedef int                             value_type;
    typedef ptrdiff_t                       difference_type;
    typedef int*                            pointer;
    typedef int&                            reference;
    typedef std::make_unsigned_t<ptrdiff_t> udifference_type;
    typedef SizedSentinelForwardIterBase    self;

    int *value = nullptr;

    constexpr SizedSentinelForwardIterBase() = default;
    constexpr explicit SizedSentinelForwardIterBase(int *ptr) : value(ptr) {}

    constexpr int *base() const { return value; }

    friend bool operator==(const self&, const self&);

    reference operator*() const;
    pointer operator->() const;
    self& operator++();
    self operator++(int);
    self& operator--();
    self operator--(int);

    template<bool E = EnableConvertible>
      requires E
    constexpr operator pointer() const { return value; }
};
using SizedSentinelForwardIter = SizedSentinelForwardIterBase<false>;
using SizedSentinelFowardSubrange = ranges::subrange<SizedSentinelForwardIter,
                                                     SizedSentinelForwardIter,
                                                     std::ranges::subrange_kind::sized>;
using ConvertibleSizedSentinelForwardIter = SizedSentinelForwardIterBase<true>;
using ConvertibleSizedSentinelForwardSubrange = ranges::subrange<ConvertibleSizedSentinelForwardIter, int*,
                                                                 std::ranges::subrange_kind::sized>;

using SizedIntPtrSubrange = ranges::subrange<int*, int*, std::ranges::subrange_kind::sized>;

// !StoreSize ctor.
static_assert( std::is_constructible_v<FowardSubrange, ForwardIter, ForwardIter>); // Default case.
static_assert(!std::is_constructible_v<FowardSubrange, Empty, ForwardIter>); // 1.
static_assert( std::is_constructible_v<ConvertibleFowardSubrange, ConvertibleForwardIter, int*>); // 2.
static_assert( std::is_constructible_v<FowardSubrange, ForwardIter, ForwardIter>); // 3. (Same as default case.)
// 4. and 5. must be sized.

// Sized ctor.
static_assert( std::is_constructible_v<SizedSentinelFowardSubrange, SizedSentinelForwardIter, SizedSentinelForwardIter, SizedSentinelForwardIter::udifference_type>); // Default case.
static_assert(!std::is_constructible_v<SizedSentinelFowardSubrange, Empty, SizedSentinelForwardIter, SizedSentinelForwardIter::udifference_type>); // 1.
static_assert( std::is_constructible_v<ConvertibleSizedSentinelForwardSubrange, ConvertibleSizedSentinelForwardIter, int*, ConvertibleSizedSentinelForwardIter::udifference_type>); // 2.
static_assert( std::is_constructible_v<SizedSentinelFowardSubrange, SizedSentinelForwardIter, SizedSentinelForwardIter, SizedSentinelForwardIter::udifference_type>); // 3. (Same as default case.)
static_assert(!std::is_constructible_v<SizedIntPtrSubrange, long*, int*, size_t>); // 4.
static_assert( std::is_constructible_v<SizedIntPtrSubrange, int*, int*, size_t>); // 5.
// TODO: is there any integral type that is not convertible to an unsized type?
// static_assert(!std::is_constructible_v<SizedSentinelFowardSubrange, SizedSentinelForwardIter, SizedSentinelForwardIter, SizedSentinelForwardIter::difference_type>); // Not unsigned.

struct ForwardBorrowedRange {
  constexpr ForwardIter begin() const { return ForwardIter(globalBuff); }
  constexpr ForwardIter end() const { return ForwardIter(globalBuff + 8); }
};

template<>
inline constexpr bool ranges::enable_borrowed_range<ForwardBorrowedRange> = true;

struct ForwardRange {
  ForwardIter begin() const;
  ForwardIter end() const;
};

struct ConvertibleForwardBorrowedRange {
  constexpr ConvertibleForwardIter begin() const { return ConvertibleForwardIter(globalBuff); }
  constexpr int *end() const { return globalBuff + 8; }
};

template<>
inline constexpr bool ranges::enable_borrowed_range<ConvertibleForwardBorrowedRange> = true;

struct ForwardBorrowedRangeDifferentSentienl {
  struct sentinel {
    int *value;
    friend bool operator==(sentinel s, ForwardIter i) { return s.value == i.base(); }
  };

  constexpr ForwardIter begin() const { return ForwardIter(globalBuff); }
  constexpr sentinel end() const { return sentinel{globalBuff + 8}; }
};

template<>
inline constexpr bool ranges::enable_borrowed_range<ForwardBorrowedRangeDifferentSentienl> = true;

using DifferentSentienlSubrange = ranges::subrange<ForwardIter,
                                                   ForwardBorrowedRangeDifferentSentienl::sentinel,
                                                   std::ranges::subrange_kind::unsized>;

// Range ctor.
static_assert( std::is_constructible_v<FowardSubrange, ForwardBorrowedRange>); // Default case.
static_assert(!std::is_constructible_v<FowardSubrange, ForwardRange>); // Not borrowed.
// Iter convertible to sentinel (pointer) type.
static_assert( std::is_constructible_v<ConvertibleFowardSubrange, ConvertibleForwardBorrowedRange>);
// Where neither iter or sentinel are pointers, but they are different.
static_assert( std::is_constructible_v<DifferentSentienlSubrange, ForwardBorrowedRangeDifferentSentienl>);

// Pair like conversion.
static_assert( std::is_convertible_v<FowardSubrange, std::pair<ForwardIter, ForwardIter>>);
static_assert( std::is_convertible_v<FowardSubrange, std::tuple<ForwardIter, ForwardIter>>);
static_assert(!std::is_convertible_v<FowardSubrange, std::tuple<ForwardIter, ForwardIter>&>);
static_assert(!std::is_convertible_v<FowardSubrange, std::tuple<ForwardIter, ForwardIter, ForwardIter>>);
static_assert( std::is_convertible_v<ConvertibleFowardSubrange, std::tuple<ConvertibleForwardIter, int*>>);
static_assert(!std::is_convertible_v<SizedIntPtrSubrange, std::tuple<long*, int*>>);
static_assert( std::is_convertible_v<SizedIntPtrSubrange, std::tuple<int*, int*>>);

constexpr bool test() {
  FowardSubrange a(ForwardIter(globalBuff), ForwardIter(globalBuff + 8));
  assert(a.begin().base() == globalBuff);
  assert(a.end().base() == globalBuff + 8);

  ConvertibleFowardSubrange b(ConvertibleForwardIter(globalBuff), globalBuff + 8);
  assert(b.begin() == globalBuff);
  assert(b.end() == globalBuff + 8);

  SizedSentinelFowardSubrange d(SizedSentinelForwardIter(globalBuff), SizedSentinelForwardIter(globalBuff + 8), 8);
  assert(d.begin().base() == globalBuff);
  assert(d.end().base() == globalBuff + 8);
  assert(d.size() == 8);

  ConvertibleSizedSentinelForwardSubrange e(ConvertibleSizedSentinelForwardIter(globalBuff), ConvertibleSizedSentinelForwardIter(globalBuff + 8), 8);
  assert(e.begin() == globalBuff);
  assert(e.end() == globalBuff + 8);
  assert(e.size() == 8);

  SizedIntPtrSubrange f(globalBuff, globalBuff + 8, 8);
  assert(f.begin() == globalBuff);
  assert(f.end() == globalBuff + 8);
  assert(f.size() == 8);

  FowardSubrange g{ForwardBorrowedRange()};
  assert(g.begin().base() == globalBuff);
  assert(g.end().base() == globalBuff + 8);

  ConvertibleFowardSubrange h{ConvertibleForwardBorrowedRange()};
  assert(h.begin() == globalBuff);
  assert(h.end() == globalBuff + 8);

  DifferentSentienlSubrange i{ForwardBorrowedRangeDifferentSentienl()};
  assert(i.begin().base() == globalBuff);
  assert(i.end().value == globalBuff + 8);

  FowardSubrange j(ForwardIter(globalBuff), ForwardIter(globalBuff + 8));
  std::pair<ForwardIter, ForwardIter> jPair = j;
  assert(jPair.first.base() == globalBuff);
  assert(jPair.second.base() == globalBuff + 8);
  std::tuple<ForwardIter, ForwardIter> jTuple = j;
  assert(get<0>(jTuple).base() == globalBuff);
  assert(get<1>(jTuple).base() == globalBuff + 8);

  return true;
}

int main(int, char**) {
  test();
  static_assert(test());

  return 0;
}
