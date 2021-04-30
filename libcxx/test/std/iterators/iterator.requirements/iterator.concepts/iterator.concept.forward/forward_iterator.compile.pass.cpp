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

// std::forward_iterator;

#include <iterator>

#include <concepts>

#include "test_iterators.h"

static_assert(std::forward_iterator<forward_iterator<int*> >);
static_assert(std::forward_iterator<bidirectional_iterator<int*> >);
static_assert(std::forward_iterator<random_access_iterator<int*> >);
static_assert(std::forward_iterator<contiguous_iterator<int*> >);

// TODO: add new `cxx20_*_iterator`s as they're added
static_assert(!std::forward_iterator<cpp20_input_iterator<int*> >);

static_assert(std::forward_iterator<int*>);
static_assert(std::forward_iterator<int const*>);
static_assert(std::forward_iterator<int volatile*>);
static_assert(std::forward_iterator<int const volatile*>);

struct not_input_iterator {
  using difference_type = std::ptrdiff_t;
  using iterator_concept = std::forward_iterator_tag;

  int operator*() const;

  not_input_iterator& operator++();
  not_input_iterator operator++(int);
};
static_assert(std::input_or_output_iterator<not_input_iterator> && !std::input_iterator<not_input_iterator> &&
              !std::forward_iterator<not_input_iterator>);

struct not_equality_comparable {
  using value_type = int;
  using difference_type = std::ptrdiff_t;
  using iterator_concept = std::forward_iterator_tag;

  int operator*() const;

  not_equality_comparable& operator++();
  not_equality_comparable operator++(int);
};
static_assert(std::input_iterator<not_equality_comparable> && !std::forward_iterator<not_equality_comparable>);
