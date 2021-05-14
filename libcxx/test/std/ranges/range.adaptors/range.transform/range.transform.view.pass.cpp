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

// class std::ranges::transform_view;

#include <ranges>

#include <vector>
#include <list>
#include <numeric>
#include <string>
#include <iostream>
#include <cctype>

#include <cassert>
#include "test_macros.h"
#include "test_iterators.h"

namespace ranges = std::ranges;

int globalBuff[8] = {0,1,2,3,4,5,6,7};

template<class T, class F>
concept ValidDropView = requires { typename ranges::transform_view<T, F>; };

struct View : std::ranges::view_base {
  int start;
  int *ptr;
  constexpr View(int* ptr = globalBuff, int start = 0) : start(start), ptr(ptr) {}
  constexpr View(View&&) = default;
  constexpr View& operator=(View&&) = default;
  constexpr friend int* begin(View& view) { return view.ptr + view.start; }
  constexpr friend int* begin(View const& view) { return view.ptr + view.start; }
  constexpr friend int* end(View& view) { return view.ptr + 8; }
  constexpr friend int* end(View const& view) { return view.ptr + 8; }
};

struct CopyableView : std::ranges::view_base {
  int start;
  constexpr CopyableView(int start = 0) : start(start) {}
  constexpr CopyableView(CopyableView const&) = default;
  constexpr CopyableView& operator=(CopyableView const&) = default;
  constexpr friend int* begin(CopyableView& view) { return globalBuff + view.start; }
  constexpr friend int* begin(CopyableView const& view) { return globalBuff + view.start; }
  constexpr friend int* end(CopyableView&) { return globalBuff + 8; }
  constexpr friend int* end(CopyableView const&) { return globalBuff + 8; }
};

using ForwardIter = forward_iterator<int*>;
struct ForwardView : std::ranges::view_base {
  int *ptr;
  constexpr ForwardView(int* ptr = globalBuff) : ptr(ptr) {}
  constexpr ForwardView(ForwardView&&) = default;
  constexpr ForwardView& operator=(ForwardView&&) = default;
  constexpr friend ForwardIter begin(ForwardView& view) { return ForwardIter(view.ptr); }
  constexpr friend ForwardIter begin(ForwardView const& view) { return ForwardIter(view.ptr); }
  constexpr friend ForwardIter end(ForwardView& view) { return ForwardIter(view.ptr + 8); }
  constexpr friend ForwardIter end(ForwardView const& view) { return ForwardIter(view.ptr + 8); }
};

struct ForwardRange {
  ForwardIter begin() const;
  ForwardIter end() const;
  ForwardIter begin();
  ForwardIter end();
};

using RandomAccessIter = random_access_iterator<int*>;
struct RandomAccessView : std::ranges::view_base {
  RandomAccessIter begin() const;
  RandomAccessIter end() const;
  RandomAccessIter begin();
  RandomAccessIter end();
};

using BidirectionalIter = bidirectional_iterator<int*>;
struct BidirectionalView : std::ranges::view_base {
  BidirectionalIter begin() const;
  BidirectionalIter end() const;
  BidirectionalIter begin();
  BidirectionalIter end();
};

struct BorrowableRange {
  friend int* begin(BorrowableRange const& range);
  friend int* end(BorrowableRange const&);
  friend int* begin(BorrowableRange& range);
  friend int* end(BorrowableRange&);
};

template<>
inline constexpr bool ranges::enable_borrowed_range<BorrowableRange> = true;

struct InputView : std::ranges::view_base {
  int *ptr;
  constexpr InputView(int* ptr = globalBuff) : ptr(ptr) {}
  constexpr cpp20_input_iterator<int*> begin() const { return cpp20_input_iterator<int*>(ptr); }
  constexpr int* end() const { return ptr + 8; }
  constexpr cpp20_input_iterator<int*> begin() { return cpp20_input_iterator<int*>(ptr); }
  constexpr int* end() { return ptr + 8; }
};

constexpr bool operator==(const cpp20_input_iterator<int*> &lhs, int* rhs) { return lhs.base() == rhs; }
constexpr bool operator==(int* lhs, const cpp20_input_iterator<int*> &rhs) { return rhs.base() == lhs; }

struct SentinelView : std::ranges::view_base {
  int count;
  constexpr SentinelView(int count = 8) : count(count) {}
  constexpr RandomAccessIter begin() const { return RandomAccessIter(globalBuff); }
  constexpr int* end() const { return globalBuff + count; }
  constexpr RandomAccessIter begin() { return RandomAccessIter(globalBuff); }
  constexpr int* end() { return globalBuff + count; }
};

constexpr long operator- (const RandomAccessIter &lhs, int* rhs) { return lhs.base() - rhs; }
constexpr long operator- (int* lhs, const RandomAccessIter &rhs) { return lhs - rhs.base(); }
constexpr bool operator==(const RandomAccessIter &lhs, int* rhs) { return lhs.base() == rhs; }
constexpr bool operator==(int* lhs, const RandomAccessIter &rhs) { return rhs.base() == lhs; }

struct SizedSentinelNotConstView : std::ranges::view_base {
  ForwardIter begin() const;
  int *end() const;
  ForwardIter begin();
  int *end();
  size_t size();
};
bool operator==(const ForwardIter &lhs, int* rhs);
bool operator==(int* lhs, const ForwardIter &rhs);

struct Range {
  friend int* begin(Range const&);
  friend int* end(Range const&);
  friend int* begin(Range&);
  friend int* end(Range&);
};

using CountedIter = stride_counting_iterator<forward_iterator<int*>>;
struct CountedView : std::ranges::view_base {
  constexpr CountedIter begin() { return CountedIter(ForwardIter(globalBuff)); }
  constexpr CountedIter begin() const { return CountedIter(ForwardIter(globalBuff)); }
  constexpr CountedIter end() { return CountedIter(ForwardIter(globalBuff + 8)); }
  constexpr CountedIter end() const { return CountedIter(ForwardIter(globalBuff + 8)); }
};

struct PlusPlus {
  constexpr int operator()(int x) { return ++x; }
};

struct PlusPlusConst {
  constexpr int operator()(int x) const { return ++x; }
};

struct PlusPlusRef {
  constexpr int& operator()(int x) { return ++x; }
};

template<class T>
concept BeginInvocable = requires(T t) { t.begin(); };

template<class T>
concept EndInvocable = requires(T t) { t.end(); };

template<class T>
concept EndIsIter = requires(T t) { ++t.end(); };

template<class T>
concept SizeInvocable = requires(T t) { t.size(); };

constexpr void testViewMembers() {
  {
    // transform_view::base
    std::ranges::transform_view<View, PlusPlus> transformView1;
    auto base1 = std::move(transformView1).base();
    assert(ranges::begin(base1) == globalBuff);

    std::ranges::transform_view<CopyableView, PlusPlus> transformView2;
    auto base2 = transformView2.base();
    assert(ranges::begin(base2) == globalBuff);
    auto base3 = std::move(transformView2).base();
    assert(ranges::begin(base3) == globalBuff);

    const std::ranges::transform_view<CopyableView, PlusPlus> transformView3;
    const auto base4 = transformView3.base();
    assert(ranges::begin(base4) == globalBuff);
    const auto base5 = std::move(transformView3).base();
    assert(ranges::begin(base5) == globalBuff);
  }

  {
    // transform_view::begin
    int buff[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    std::ranges::transform_view transformView1(View{buff}, PlusPlus{});
    assert(transformView1.begin().base() == buff);
    assert(*transformView1.begin() == 1);

    std::ranges::transform_view transformView2(ForwardView{buff}, PlusPlus{});
    assert(transformView2.begin().base().base() == buff);
    assert(*transformView2.begin() == 1);

    std::ranges::transform_view transformView3(InputView{buff}, PlusPlus{});
    assert(transformView3.begin().base().base() == buff);
    assert(*transformView3.begin() == 1);

    const std::ranges::transform_view transformView4(View{buff}, PlusPlusConst{});
    assert(*transformView4.begin() == 1);

    static_assert(!BeginInvocable<const std::ranges::transform_view<View, PlusPlus>>);
  }

  {
    // transform_view::end
    std::ranges::transform_view transformView1(View{}, PlusPlus{});
    assert(transformView1.end().base() == globalBuff + 8);

    std::ranges::transform_view transformView2(ForwardView{}, PlusPlus{});
    assert(transformView2.end().base().base() == globalBuff + 8);

    std::ranges::transform_view transformView3(InputView{}, PlusPlus{});
    assert(transformView3.end().base() == globalBuff + 8);

    const std::ranges::transform_view transformView4(View{}, PlusPlusConst{});
    assert(transformView4.end().base() == globalBuff + 8);

    static_assert(!EndInvocable<const std::ranges::transform_view<View, PlusPlus>>);
    static_assert( EndInvocable<      std::ranges::transform_view<View, PlusPlus>>);
    static_assert( EndInvocable<const std::ranges::transform_view<View, PlusPlusConst>>);
    static_assert(!EndInvocable<const std::ranges::transform_view<InputView, PlusPlus>>);
    static_assert( EndInvocable<      std::ranges::transform_view<InputView, PlusPlus>>);
    static_assert( EndInvocable<const std::ranges::transform_view<InputView, PlusPlusConst>>);

    static_assert(!EndIsIter<const std::ranges::transform_view<InputView, PlusPlusConst>>);
    static_assert(!EndIsIter<      std::ranges::transform_view<InputView, PlusPlus>>);
    static_assert( EndIsIter<const std::ranges::transform_view<View, PlusPlusConst>>);
    static_assert( EndIsIter<      std::ranges::transform_view<View, PlusPlus>>);
  }

  {
    // transform_view::size
    std::ranges::transform_view transformView1(View{}, PlusPlus{});
    assert(transformView1.size() == 8);

    const std::ranges::transform_view transformView2(View{globalBuff, 4}, PlusPlus{});
    assert(transformView2.size() == 4);

    static_assert(!SizeInvocable<std::ranges::transform_view<ForwardView, PlusPlus>>);

    static_assert(SizeInvocable<std::ranges::transform_view<SizedSentinelNotConstView, PlusPlus>>);
    static_assert(!SizeInvocable<const std::ranges::transform_view<SizedSentinelNotConstView, PlusPlus>>);
  }
}

template<class V, class F>
concept HasIterConcept = requires { typename std::ranges::transform_view<V, F>::iterator_category; };

template<class V, class F>
concept BaseInvocable = requires(std::ranges::iterator_t<std::ranges::transform_view<V, F>> iter) {
  iter.base();
};

constexpr void testIterator() {
  // Member typedefs.
  {
    // Member typedefs for contiguous iterator.
    static_assert(std::same_as<std::iterator_traits<int*>::iterator_concept, std::contiguous_iterator_tag>);
    static_assert(std::same_as<std::iterator_traits<int*>::iterator_category, std::random_access_iterator_tag>);

    using TView = std::ranges::transform_view<View, PlusPlusRef>;
    using TIter = std::ranges::iterator_t<TView>;
    static_assert(std::same_as<typename TIter::iterator_concept, std::random_access_iterator_tag>);
    static_assert(std::same_as<typename TIter::iterator_category, std::random_access_iterator_tag>);
    static_assert(std::same_as<typename TIter::value_type, int>);
    static_assert(std::same_as<typename TIter::difference_type, std::ptrdiff_t>);
  }
  {
    // Member typedefs for random access iterator.
    using TView = std::ranges::transform_view<RandomAccessView, PlusPlusRef>;
    using TIter = std::ranges::iterator_t<TView>;
    static_assert(std::same_as<typename TIter::iterator_concept, std::random_access_iterator_tag>);
    static_assert(std::same_as<typename TIter::iterator_category, std::random_access_iterator_tag>);
    static_assert(std::same_as<typename TIter::value_type, int>);
    static_assert(std::same_as<typename TIter::difference_type, std::ptrdiff_t>);
  }
  {
    // Member typedefs for random access iterator/not-lvalue-ref.
    using TView = std::ranges::transform_view<RandomAccessView, PlusPlus>;
    using TIter = std::ranges::iterator_t<TView>;
    static_assert(std::same_as<typename TIter::iterator_concept, std::random_access_iterator_tag>);
    static_assert(std::same_as<typename TIter::iterator_category, std::input_iterator_tag>); // Note: this is now input_iterator_tag.
    static_assert(std::same_as<typename TIter::value_type, int>);
    static_assert(std::same_as<typename TIter::difference_type, std::ptrdiff_t>);
  }
  {
    // Member typedefs for bidirectional iterator.
    using TView = std::ranges::transform_view<BidirectionalView, PlusPlusRef>;
    using TIter = std::ranges::iterator_t<TView>;
    static_assert(std::same_as<typename TIter::iterator_concept, std::bidirectional_iterator_tag>);
    static_assert(std::same_as<typename TIter::iterator_category, std::bidirectional_iterator_tag>);
    static_assert(std::same_as<typename TIter::value_type, int>);
    static_assert(std::same_as<typename TIter::difference_type, std::ptrdiff_t>);
  }
  {
    // Member typedefs for forward iterator.
    using TView = std::ranges::transform_view<ForwardView, PlusPlusRef>;
    using TIter = std::ranges::iterator_t<TView>;
    static_assert(std::same_as<typename TIter::iterator_concept, std::forward_iterator_tag>);
    static_assert(std::same_as<typename TIter::iterator_category, std::forward_iterator_tag>);
    static_assert(std::same_as<typename TIter::value_type, int>);
    static_assert(std::same_as<typename TIter::difference_type, std::ptrdiff_t>);
  }
  {
    // Member typedefs for input iterator.
    using TView = std::ranges::transform_view<InputView, PlusPlusRef>;
    using TIter = std::ranges::iterator_t<TView>;
    static_assert(std::same_as<typename TIter::iterator_concept, std::input_iterator_tag>);
    static_assert(!HasIterConcept<InputView, PlusPlusRef>);
    static_assert(std::same_as<typename TIter::value_type, int>);
    static_assert(std::same_as<typename TIter::difference_type, std::ptrdiff_t>);
  }

  // Members
  {
    // __iterator::base
    std::ranges::transform_view<View, PlusPlus> transformView1;
    auto iter1 = std::move(transformView1).begin();
    assert(iter1.base() == globalBuff);
    assert(std::move(iter1).base() == globalBuff);

    std::ranges::transform_view<InputView, PlusPlus> transformView2;
    auto iter2 = transformView2.begin();
    assert(std::move(iter2).base() == globalBuff);
    static_assert(!BaseInvocable<InputView, PlusPlus>);
  }
  // Note: __iterator::operator* tested elsewhere.
  // Note: member operator requirements tested at the end of the file.
  {
    // __iterator::operator{++,--,+=,-=}
    std::ranges::transform_view<View, PlusPlus> transformView1;
    auto iter1 = std::move(transformView1).begin();
    assert((++iter1).base() == globalBuff + 1);

    assert((iter1++).base() == globalBuff + 1);
    assert(iter1.base() == globalBuff + 2);

    assert((--iter1).base() == globalBuff + 1);
    assert((iter1--).base() == globalBuff + 1);
    assert(iter1.base() == globalBuff);

    // Check that decltype(InputIter++) == void.
    ASSERT_SAME_TYPE(decltype(
      std::declval<std::ranges::iterator_t<std::ranges::transform_view<InputView, PlusPlus>>>()++),
      void);

    assert((iter1 += 4).base() == globalBuff + 4);
    assert((iter1 -= 3).base() == globalBuff + 1);
  }
  {
    // __iterator::operator[]
    int buff[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    std::ranges::transform_view transformView1(View{buff}, PlusPlus{});
    auto iter1 = std::move(transformView1).begin() + 1;
    assert(iter1[0] == 2);
    assert(iter1[4] == 6);
  }
  {
    // __iterator::operator{<,>,<=,>=}
    std::ranges::transform_view<View, PlusPlus> transformView1;
    auto iter1 = std::move(transformView1).begin();
    std::ranges::transform_view<View, PlusPlus> transformView2;
    auto iter2 = std::move(transformView2).begin();
    assert(iter1 == iter2);
    assert(iter1 + 1 != iter2);
    assert(iter1 + 1 == iter2 + 1);

    assert(iter1 < iter1 + 1);
    assert(iter1 + 1 > iter1);
    assert(iter1 <= iter1 + 1);
    assert(iter1 <= iter2);
    assert(iter1 + 1 >= iter2);
    assert(iter1     >= iter2);
    // TODO: operator<=> when three_way_comparable is implemented.
  }
  {
    // __iterator::operator{+,-}
    std::ranges::transform_view<View, PlusPlus> transformView1;
    auto iter1 = std::move(transformView1).begin();
    std::ranges::transform_view<View, PlusPlus> transformView2;
    auto iter2 = std::move(transformView2).begin();
    iter1 += 4;
    assert((iter1 + 1).base() == globalBuff + 5);
    assert((1 + iter1).base() == globalBuff + 5);
    assert((iter1 - 1).base() == globalBuff + 3);
    assert(iter1 - iter2 == 4);
  }
  // TODO: I'm not going to test iter_move until the TODOs in transform_view.h are addressed.
  {
    // __iterator::iter_swap
    int buff[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    std::ranges::transform_view transformView1(View{buff}, PlusPlus{});
    auto iter1 = std::move(transformView1).begin();
    std::ranges::transform_view transformView2(View{buff}, PlusPlus{});
    auto iter2 = std::move(transformView2).begin();
    iter1 += 4;
    std::ranges::iter_swap(iter1, iter2);
    assert(*iter1 == 1);
    assert(*(buff + 4) == 0);
    assert(*iter2 == 5);
    assert(*buff == 4);
    assert(*transformView1.begin() == 5);
  }
}

constexpr void testSentinel() {
  std::ranges::transform_view<SentinelView, PlusPlusConst> transformView1;
  // Going to const and back.
  auto sent1 = transformView1.end();
  std::ranges::sentinel_t<const std::ranges::transform_view<SentinelView, PlusPlusConst>> sent2{sent1};
  std::ranges::sentinel_t<const std::ranges::transform_view<SentinelView, PlusPlusConst>> sent3{sent2};
  (void)sent3;

  std::ranges::transform_view transformView2(SentinelView{4}, PlusPlusConst());
  auto sent4 = transformView2.end();

  static_assert(!EndIsIter<decltype(sent1)>);
  static_assert(!EndIsIter<decltype(sent2)>);
  assert(sent1.base() == globalBuff + 8);

  auto iter1 = transformView1.begin();
  assert(iter1 != sent1);
  assert(iter1 != sent2);
  assert(iter1 != sent4);
  assert(iter1 + 8 == sent1);
  assert(iter1 + 8 == sent2);
  assert(iter1 + 4 == sent4);

  assert(sent1 - iter1 == 8);
  assert(sent4 - iter1 == 4);
  assert(iter1 - sent1 == -8);
  assert(iter1 - sent4 == -4);
}

// CTAD
static_assert(std::same_as<decltype(std::ranges::transform_view(InputView(), PlusPlus())),
                           std::ranges::transform_view<InputView, PlusPlus>>);
static_assert(std::same_as<decltype(std::ranges::transform_view(std::declval<ForwardRange&>(), PlusPlus())),
                           std::ranges::transform_view<std::ranges::ref_view<ForwardRange>, PlusPlus>>);
static_assert(std::same_as<decltype(std::ranges::transform_view(BorrowableRange(), PlusPlus())),
                           std::ranges::transform_view<std::ranges::subrange<int*>, PlusPlus>>);

constexpr bool test() {
  testViewMembers();
  testIterator();
  testSentinel();

  return true;
}

template<std::ranges::range R>
auto toUpper(R range) {
  return std::ranges::transform_view(range, [](char c) { return std::toupper(c); });
}

template<std::ranges::range R, class Fn = std::plus<std::iter_value_t<R>>>
auto withRandom(R range, Fn func = Fn()) {
  return std::ranges::transform_view(range, std::bind_front(func, std::rand()));
}

template<class E1, class E2, size_t N, class Join = std::plus<E1>>
auto joinArrays(E1 (&a)[N], E2 (&b)[N], Join join = Join()) {
  return std::ranges::transform_view(a, [&a, &b, join](auto& x) {
    auto idx = (&x) - a;
    return join(x, b[idx]);
  });
}

template<>
constexpr bool ranges::enable_view<std::vector<int>> = true;

template<>
constexpr bool ranges::enable_view<std::string_view> = true;

int main(int, char**) {
  test();
  static_assert(test());

  {
    std::vector vec = {1,2,3,4};
    auto sortOfRandom = withRandom(vec);
    std::vector check = {16808, 16809, 16810, 16811};
    assert(std::equal(sortOfRandom.begin(), sortOfRandom.end(), check.begin(), check.end()));
  }

  {
    int a[4] = {1, 2, 3, 4};
    int b[4] = {4, 3, 2, 1};
    auto out = joinArrays(a, b);
    int check[4] = {5, 5, 5, 5};
    assert(std::equal(out.begin(), out.end(), check));
  }

  {
    std::string_view str = "Hello, World.";
    auto upp = toUpper(str);
    std::string_view check = "HELLO, WORLD.";
    assert(std::equal(upp.begin(), upp.end(), check.begin(), check.end()));
  }

  return 0;
}

// The requirements for __iterator:
template<class V, class F>
concept IterMinusMinus = requires(std::ranges::iterator_t<std::ranges::transform_view<V, F>> iter) {
  iter--;
};
static_assert(IterMinusMinus<BidirectionalView, PlusPlusConst>);
static_assert(!IterMinusMinus<ForwardView, PlusPlusConst>);

template<class V, class F>
concept MinusMinusIter = requires(std::ranges::iterator_t<std::ranges::transform_view<V, F>> iter) {
  --iter;
};
static_assert(MinusMinusIter<BidirectionalView, PlusPlusConst>);
static_assert(!MinusMinusIter<ForwardView, PlusPlusConst>);

template<class V, class F>
concept IterPlusEquals = requires(std::ranges::iterator_t<std::ranges::transform_view<V, F>> iter) {
  iter += 1;
};
static_assert(IterPlusEquals<RandomAccessView, PlusPlusConst>);
static_assert(!IterPlusEquals<BidirectionalView, PlusPlusConst>);

template<class V, class F>
concept IterMinusEquals = requires(std::ranges::iterator_t<std::ranges::transform_view<V, F>> iter) {
  iter -= 1;
};
static_assert(IterMinusEquals<RandomAccessView, PlusPlusConst>);
static_assert(!IterMinusEquals<BidirectionalView, PlusPlusConst>);

template<class V, class F>
concept IterSubscript = requires(std::ranges::iterator_t<std::ranges::transform_view<V, F>> iter) {
  iter[0];
};
static_assert(IterSubscript<RandomAccessView, PlusPlusConst>);
static_assert(!IterSubscript<BidirectionalView, PlusPlusConst>);

template<class V, class F>
concept IterLess = requires(std::ranges::iterator_t<std::ranges::transform_view<V, F>> iter) {
  iter < iter;
};
static_assert(IterLess<RandomAccessView, PlusPlusConst>);
static_assert(!IterLess<BidirectionalView, PlusPlusConst>);

template<class V, class F>
concept IterGreater = requires(std::ranges::iterator_t<std::ranges::transform_view<V, F>> iter) {
  iter > iter;
};
static_assert(IterGreater<RandomAccessView, PlusPlusConst>);
static_assert(!IterGreater<BidirectionalView, PlusPlusConst>);

template<class V, class F>
concept IterLessEq = requires(std::ranges::iterator_t<std::ranges::transform_view<V, F>> iter) {
  iter <= iter;
};
static_assert(IterLessEq<RandomAccessView, PlusPlusConst>);
static_assert(!IterLessEq<BidirectionalView, PlusPlusConst>);

template<class V, class F>
concept IterGreaterEq = requires(std::ranges::iterator_t<std::ranges::transform_view<V, F>> iter) {
  iter >= iter;
};
static_assert(IterGreaterEq<RandomAccessView, PlusPlusConst>);
static_assert(!IterGreaterEq<BidirectionalView, PlusPlusConst>);

template<class V, class F>
concept IterPlusN = requires(std::ranges::iterator_t<std::ranges::transform_view<V, F>> iter) {
  iter + 1;
};
static_assert(IterPlusN<RandomAccessView, PlusPlusConst>);
static_assert(!IterPlusN<BidirectionalView, PlusPlusConst>);

template<class V, class F>
concept NPlusIter = requires(std::ranges::iterator_t<std::ranges::transform_view<V, F>> iter) {
  std::declval<typename decltype(iter)::difference_type>() + iter;
};
static_assert(NPlusIter<RandomAccessView, PlusPlusConst>);
static_assert(!NPlusIter<BidirectionalView, PlusPlusConst>);

template<class V, class F>
concept IterMinusN = requires(std::ranges::iterator_t<std::ranges::transform_view<V, F>> iter) {
  iter - 1;
};
static_assert(IterMinusN<RandomAccessView, PlusPlusConst>);
static_assert(!IterMinusN<BidirectionalView, PlusPlusConst>);

template<class V, class F>
concept IterMinusIter = requires(std::ranges::iterator_t<std::ranges::transform_view<V, F>> iter) {
  iter - iter;
};
static_assert(IterMinusIter<RandomAccessView, PlusPlusConst>);
static_assert(!IterMinusIter<BidirectionalView, PlusPlusConst>);
