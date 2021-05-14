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

// __semiregular_box

#include <__ranges/semiregular_box.h>

#include <concepts>
#include <optional>

static_assert(std::derived_from<std::ranges::__semiregular_box<int>, std::optional<int> >);

template <class T>
constexpr bool valid_semiregular_box = requires {
  typename std::ranges::__semiregular_box<T>;
};

static_assert(!valid_semiregular_box<void>);
static_assert(!valid_semiregular_box<int&>);

struct not_copy_constructible {
  not_copy_constructible() = default;
  not_copy_constructible(not_copy_constructible&&) = default;
  not_copy_constructible(not_copy_constructible const&) = delete;
  not_copy_constructible& operator=(not_copy_constructible&&) = default;
  not_copy_constructible& operator=(not_copy_constructible const&) = default;
};

static_assert(!valid_semiregular_box<not_copy_constructible>);
