// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP___ITERATOR_FUNCTIONISH_H
#define _LIBCPP___ITERATOR_FUNCTIONISH_H

#include <__config>

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#pragma GCC system_header
#endif

_LIBCPP_PUSH_MACROS
#include <__undef_macros>

_LIBCPP_BEGIN_NAMESPACE_STD

#if !defined(_LIBCPP_HAS_NO_RANGES)

namespace ranges {
// Per [range.iter.ops.general] and [algorithms.requirements], functions in namespace std::ranges
// can't be found by ADL and inhibit ADL when found by unqualified lookup. The easiest way to
// facilitate this is to use function objects.
//
// Since these are still standard library functions, we use `__functionish` to eliminate most of
// the properties that function objects get by default (e.g. semiregularity, addressability), to
// limit the surface area of the unintended public interface, so as to curb the effect of Hyrum's
// law.
struct __functionish {
  __functionish() = delete;
  __functionish(__functionish const&) = delete;
  __functionish& operator=(__functionish const&) = delete;

  void operator&() const = delete;

  enum __tag { __value };

protected:
  constexpr explicit __functionish(__tag) noexcept {}
  ~__functionish() = default;
};
} // namespace ranges

#endif // !defined(_LIBCPP_HAS_NO_RANGES)

_LIBCPP_END_NAMESPACE_STD

#endif // _LIBCPP___ITERATOR_FUNCTIONISH_H
