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

// template<input_or_output_iterator I, sentinel_for<I> S>
//  requires (!same_as<I, S> && copyable<I>)
// class common_iterator;

#include <iterator>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

template <class It>
class simple_iterator
{
    It it_;

public:
    typedef          std::input_iterator_tag                   iterator_category;
    typedef typename std::iterator_traits<It>::value_type      value_type;
    typedef typename std::iterator_traits<It>::difference_type difference_type;
    typedef It                                                 pointer;
    typedef typename std::iterator_traits<It>::reference       reference;

    constexpr It base() const {return it_;}

    simple_iterator() = default;
    explicit constexpr simple_iterator(It it) : it_(it) {}

    constexpr reference operator*() const {return *it_;}

    constexpr simple_iterator& operator++() {++it_; return *this;}
    constexpr simple_iterator operator++(int)
        {simple_iterator tmp(*this); ++(*this); return tmp;}
};

template <class It>
class value_iterator
{
    It it_;

public:
    typedef          std::input_iterator_tag                   iterator_category;
    typedef typename std::iterator_traits<It>::value_type      value_type;
    typedef typename std::iterator_traits<It>::difference_type difference_type;
    typedef It                                                 pointer;
    typedef typename std::iterator_traits<It>::reference       reference;

    constexpr It base() const {return it_;}

    value_iterator() = default;
    explicit constexpr value_iterator(It it) : it_(it) {}

    constexpr value_type operator*() const {return *it_;}

    constexpr value_iterator& operator++() {++it_; return *this;}
    constexpr value_iterator operator++(int)
        {value_iterator tmp(*this); ++(*this); return tmp;}
};


template<class T>
struct sentienl_type {
  T base;

  template<class U>
  friend constexpr bool operator==(const sentienl_type& lhs, const U& rhs) { return lhs.base == rhs.base(); }
  template<class U>
  friend constexpr bool operator==(const U& lhs, const sentienl_type& rhs) { return lhs.base() == rhs.base; }
};

template<class T>
struct sized_sentienl_type {
  T base;

  template<class U>
  friend constexpr bool operator==(const sized_sentienl_type& lhs, const U& rhs) { return lhs.base - rhs.base(); }
  template<class U>
  friend constexpr bool operator==(const U& lhs, const sized_sentienl_type& rhs) { return lhs.base() - rhs.base; }
  template<class U>
  friend constexpr auto operator- (const sized_sentienl_type& lhs, const U& rhs) { return lhs.base - rhs.base(); }
  template<class U>
  friend constexpr auto operator- (const U& lhs, const sized_sentienl_type& rhs) { return lhs.base() - rhs.base; }
};

void testCtor() {
  int buffer[8] = {1, 2, 3, 4, 5, 6, 7, 8};

  static_assert(std::is_default_constructible_v<std::common_iterator<int*, sentienl_type<int*>>>);

//   {
//     auto iter1 = cpp20_input_iterator<int*>(buffer);
//     auto commonIter1 = std::common_iterator<decltype(iter1), sentienl_type<int*>>(iter1);
//     auto commonSent1 = std::common_iterator<decltype(iter1), sentienl_type<int*>>(sentienl_type<int*>{buffer + 8});
//
//     assert(*iter1 == 1);
//     assert(*commonIter1 == 1);
//     assert(commonIter1 != commonSent1);
//   }
  {
    auto iter1 = cpp17_input_iterator<int*>(buffer);
    auto commonIter1 = std::common_iterator<decltype(iter1), sentienl_type<int*>>(iter1);
    auto commonSent1 = std::common_iterator<decltype(iter1), sentienl_type<int*>>(sentienl_type<int*>{buffer + 8});

    assert(*iter1 == 1);
    assert(*commonIter1 == 1);
    assert(commonIter1 != commonSent1);
  }
  {
    auto iter1 = forward_iterator<int*>(buffer);
    auto commonIter1 = std::common_iterator<decltype(iter1), sentienl_type<int*>>(iter1);
    auto commonSent1 = std::common_iterator<decltype(iter1), sentienl_type<int*>>(sentienl_type<int*>{buffer + 8});

    assert(*iter1 == 1);
    assert(*commonIter1 == 1);
    assert(commonIter1 != commonSent1);
  }
  {
    auto iter1 = random_access_iterator<int*>(buffer);
    auto commonIter1 = std::common_iterator<decltype(iter1), sentienl_type<int*>>(iter1);
    auto commonSent1 = std::common_iterator<decltype(iter1), sentienl_type<int*>>(sentienl_type<int*>{buffer + 8});

    assert(*iter1 == 1);
    assert(*commonIter1 == 1);
    assert(commonIter1 != commonSent1);
  }
}

void testMembers() {
  int buffer[8] = {1, 2, 3, 4, 5, 6, 7, 8};

  {
    auto iter1 = simple_iterator<int*>(buffer);
    auto commonIter1 = std::common_iterator<decltype(iter1), sentienl_type<int*>>(iter1);
    auto commonSent1 = std::common_iterator<decltype(iter1), sentienl_type<int*>>(sentienl_type<int*>{buffer + 8});

    const auto iter2 = simple_iterator<int*>(buffer);
    const auto commonIter2 = std::common_iterator<decltype(iter1), sentienl_type<int*>>(iter1);
    const auto commonSent2 = std::common_iterator<decltype(iter1), sentienl_type<int*>>(sentienl_type<int*>{buffer + 8});

    assert(*iter1 == 1);
    assert(*commonIter1 == 1);
    assert(commonIter1 != commonSent1);
    assert(commonIter1.operator->() == buffer);

    assert(*iter2 == 1);
    assert(*commonIter2 == 1);
    assert(commonIter2 != commonSent2);
    assert(commonIter2.operator->() == buffer);

    assert(*(commonIter1++) == 1);
    assert(*commonIter1 == 2);
    assert(*(++commonIter1) == 3);
    assert(*commonIter1 == 3);

    for (auto i = 3; commonIter1 != commonSent1; ++i) {
      assert(*(commonIter1++) == i);
    }
    assert(commonIter1 == commonSent1);
  }
  {
    auto iter1 = value_iterator<int*>(buffer);
    auto commonIter1 = std::common_iterator<decltype(iter1), sentienl_type<int*>>(iter1);
    auto commonSent1 = std::common_iterator<decltype(iter1), sentienl_type<int*>>(sentienl_type<int*>{buffer + 8});

    const auto iter2 = value_iterator<int*>(buffer);
    const auto commonIter2 = std::common_iterator<decltype(iter1), sentienl_type<int*>>(iter1);
    const auto commonSent2 = std::common_iterator<decltype(iter1), sentienl_type<int*>>(sentienl_type<int*>{buffer + 8});

    assert(*iter1 == 1);
    assert(*commonIter1 == 1);
    assert(commonIter1 != commonSent1);
    assert(*commonIter1.operator->().operator->() == 1);

    assert(*iter2 == 1);
    assert(*commonIter2 == 1);
    assert(commonIter2 != commonSent2);
    assert(*commonIter2.operator->().operator->() == 1);

    assert(*(commonIter1++) == 1);
    assert(*commonIter1 == 2);
    assert(*(++commonIter1) == 3);
    assert(*commonIter1 == 3);

    for (auto i = 3; commonIter1 != commonSent1; ++i) {
      assert(*(commonIter1++) == i);
    }
    assert(commonIter1 == commonSent1);
  }
  {
    auto iter1 = cpp17_input_iterator<int*>(buffer);
    auto commonIter1 = std::common_iterator<decltype(iter1), sentienl_type<int*>>(iter1);
    auto commonSent1 = std::common_iterator<decltype(iter1), sentienl_type<int*>>(sentienl_type<int*>{buffer + 8});

    const auto iter2 = cpp17_input_iterator<int*>(buffer);
    const auto commonIter2 = std::common_iterator<decltype(iter1), sentienl_type<int*>>(iter1);
    const auto commonSent2 = std::common_iterator<decltype(iter1), sentienl_type<int*>>(sentienl_type<int*>{buffer + 8});

    assert(*iter1 == 1);
    assert(*commonIter1 == 1);
    assert(commonIter1 != commonSent1);
    assert(commonIter1.operator->().base() == buffer);

    assert(*iter2 == 1);
    assert(*commonIter2 == 1);
    assert(commonIter2 != commonSent2);
    assert(commonIter2.operator->().base() == buffer);

    assert(*(commonIter1++) == 1);
    assert(*commonIter1 == 2);
    assert(*(++commonIter1) == 3);
    assert(*commonIter1 == 3);

    for (auto i = 3; commonIter1 != commonSent1; ++i) {
      assert(*(commonIter1++) == i);
    }
    assert(commonIter1 == commonSent1);
  }
  {
    auto iter1 = forward_iterator<int*>(buffer);
    auto commonIter1 = std::common_iterator<decltype(iter1), sentienl_type<int*>>(iter1);
    auto commonSent1 = std::common_iterator<decltype(iter1), sentienl_type<int*>>(sentienl_type<int*>{buffer + 8});

    const auto iter2 = forward_iterator<int*>(buffer);
    const auto commonIter2 = std::common_iterator<decltype(iter1), sentienl_type<int*>>(iter1);
    const auto commonSent2 = std::common_iterator<decltype(iter1), sentienl_type<int*>>(sentienl_type<int*>{buffer + 8});

    assert(*iter1 == 1);
    assert(*commonIter1 == 1);
    assert(commonIter1 != commonSent1);
    assert(commonIter1.operator->().base() == buffer);

    assert(*iter2 == 1);
    assert(*commonIter2 == 1);
    assert(commonIter2 != commonSent2);
    assert(commonIter2.operator->().base() == buffer);

    assert(*(commonIter1++) == 1);
    assert(*commonIter1 == 2);
    assert(*(++commonIter1) == 3);
    assert(*commonIter1 == 3);

    for (auto i = 3; commonIter1 != commonSent1; ++i) {
      assert(*(commonIter1++) == i);
    }
    assert(commonIter1 == commonSent1);
  }
  {
    auto iter1 = random_access_iterator<int*>(buffer);
    auto commonIter1 = std::common_iterator<decltype(iter1), sentienl_type<int*>>(iter1);
    auto commonSent1 = std::common_iterator<decltype(iter1), sentienl_type<int*>>(sentienl_type<int*>{buffer + 8});

    const auto iter2 = random_access_iterator<int*>(buffer);
    const auto commonIter2 = std::common_iterator<decltype(iter1), sentienl_type<int*>>(iter1);
    const auto commonSent2 = std::common_iterator<decltype(iter1), sentienl_type<int*>>(sentienl_type<int*>{buffer + 8});

    assert(*iter1 == 1);
    assert(*commonIter1 == 1);
    assert(commonIter1 != commonSent1);
    assert(commonIter1.operator->().base() == buffer);

    assert(*iter2 == 1);
    assert(*commonIter2 == 1);
    assert(commonIter2 != commonSent2);
    assert(commonIter2.operator->().base() == buffer);

    assert(*(commonIter1++) == 1);
    assert(*commonIter1 == 2);
    assert(*(++commonIter1) == 3);
    assert(*commonIter1 == 3);

    for (auto i = 3; commonIter1 != commonSent1; ++i) {
      assert(*(commonIter1++) == i);
    }
    assert(commonIter1 == commonSent1);
  }
  {
    auto iter1 = random_access_iterator<int*>(buffer);
    auto commonIter1 = std::common_iterator<decltype(iter1), sized_sentienl_type<int*>>(iter1);
    auto commonSent1 = std::common_iterator<decltype(iter1), sized_sentienl_type<int*>>(sized_sentienl_type<int*>{buffer + 8});
    assert(commonIter1 - commonSent1 == -8);
    assert(commonSent1 - commonIter1 == 8);
    assert(commonIter1 - commonIter1 == 0);
    assert(commonSent1 - commonSent1 == 0);
  }
}

void testCust() {
  {

  }
  {

  }
}

bool test() {
  testCtor();
  testMembers();

  return true;
}

int main(int, char**) {
  test();
//   static_assert(test());

  return 0;
}
