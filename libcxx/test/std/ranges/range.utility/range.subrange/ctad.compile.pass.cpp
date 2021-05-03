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

using std::ranges::subrange;

static_assert(std::same_as<decltype(subrange(forward_iterator<int*>(nullptr), forward_iterator<int*>(nullptr))), subrange<forward_iterator<int*>, forward_iterator<int*>, std::ranges::subrange_kind::unsized>>);
static_assert(std::same_as<decltype(subrange(static_cast<int*>(nullptr), static_cast<int*>(nullptr), 0)), subrange<int*, int*, std::ranges::subrange_kind::sized>>);
static_assert(std::same_as<decltype(subrange(static_cast<int*>(nullptr), nullptr, 0)), subrange<int*, nullptr_t, std::ranges::subrange_kind::sized>>);

struct ForwardRange {
  forward_iterator<int*> begin() const;
  forward_iterator<int*> end() const;
};
template<>
inline constexpr bool std::ranges::enable_borrowed_range<ForwardRange> = true;

struct SizedRange {
  int *begin();
  int *end();
};
template<>
inline constexpr bool std::ranges::enable_borrowed_range<SizedRange> = true;

static_assert(std::same_as<decltype(subrange(ForwardRange())), subrange<forward_iterator<int*>, forward_iterator<int*>, std::ranges::subrange_kind::unsized>>);
static_assert(std::same_as<decltype(subrange(SizedRange())), subrange<int*, int*, std::ranges::subrange_kind::sized>>);
static_assert(std::same_as<decltype(subrange(SizedRange(), 8)), subrange<int*, int*, std::ranges::subrange_kind::sized>>);
