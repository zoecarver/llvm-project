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

constexpr bool test() {

  return true;
}

int main(int, char**) {
  test();
  static_assert(test());

  return 0;
}
