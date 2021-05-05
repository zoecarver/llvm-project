// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP___ITERATOR_PRIMITIVES_H
#define _LIBCPP___ITERATOR_PRIMITIVES_H

#include <__config>
#include <__debug>
#include <__iterator/concepts.h>
#include <__iterator/functionish.h>
#include <__iterator/incrementable_traits.h>
#include <concepts>
#include <limits>
#include <utility>

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#pragma GCC system_header
#endif

_LIBCPP_PUSH_MACROS
#include <__undef_macros>

_LIBCPP_BEGIN_NAMESPACE_STD

#if !defined(_LIBCPP_HAS_NO_RANGES)

namespace ranges {
// [range.iter.op.advance]
struct __advance_fn final : __functionish {
private:
  template <signed_integral _Tp>
  [[nodiscard]] static constexpr auto __abs(_Tp const __n) noexcept {
    _LIBCPP_ASSERT(__n != numeric_limits<_Tp>::min(), "`-numeric_limits<I>::min()` is undefined");
    return __n < 0 ? -__n : __n;
  }

public:
  constexpr explicit __advance_fn(__tag __x) noexcept : __functionish(__x) {}

  // Preconditions: If `I` does not model `bidirectional_iterator`, `n` is not negative.
  template <input_or_output_iterator _Ip>
  constexpr void operator()(_Ip& __i, iter_difference_t<_Ip> __n) const {
    _LIBCPP_ASSERT(__n >= 0 || bidirectional_iterator<_Ip>,
                   "If `n < 0`, then `bidirectional_iterator<I>` must be true.");

    // If `I` models `random_access_iterator`, equivalent to `i += n`.
    if constexpr (random_access_iterator<_Ip>) {
      __i += __n;
      return;
    } else {
      // Otherwise, if `n` is non-negative, increments `i` by `n`.
      while (__n > 0) {
        --__n;
        ++__i;
      }

      // Otherwise, decrements `i` by `-n`.
      if constexpr (bidirectional_iterator<_Ip>) {
        while (__n < 0) {
          ++__n;
          --__i;
        }
      }
    }
  }

  // Preconditions: Either `assignable_from<I&, S> || sized_sentinel_for<S, I>` is modeled, or [i, bound) denotes a range.
  template <input_or_output_iterator _Ip, sentinel_for<_Ip> _Sp>
  constexpr void operator()(_Ip& __i, _Sp __bound) const {
    // If `I` and `S` model `assignable_from<I&, S>`, equivalent to `i = std::move(bound)`.
    if constexpr (assignable_from<_Ip&, _Sp>) {
      __i = std::move(__bound);
    }
    // Otherwise, if `S` and `I` model `sized_sentinel_for<S, I>`, equivalent to `ranges::advance(i, bound - i)`.
    else if constexpr (sized_sentinel_for<_Sp, _Ip>) {
      (*this)(__i, __bound - __i);
    }
    // Otherwise, while `bool(i != bound)` is true, increments `i`.
    else {
      while (__i != __bound) {
        ++__i;
      }
    }
  }

  // Preconditions:
  //   * If `n > 0`, [i, bound) denotes a range.
  //   * If `n == 0`, [i, bound) or [bound, i) denotes a range.
  //   * If `n < 0`, [bound, i) denotes a range, `I` models `bidirectional_iterator`, and `I` and `S` model `same_as<I, S>`.
  // Returns: `n - M`, where `M` is the difference between the the ending and starting position.
  template <input_or_output_iterator _Ip, sentinel_for<_Ip> _Sp>
  [[nodiscard]] constexpr iter_difference_t<_Ip> operator()(_Ip& __i, iter_difference_t<_Ip> __n, _Sp __bound) const {
    _LIBCPP_ASSERT(__n >= 0 || (bidirectional_iterator<_Ip> && same_as<_Ip, _Sp>),
                   "If `n < 0`, then `bidirectional_iterator<I> && same_as<I, S>` must be true.");
    // If `S` and `I` model `sized_sentinel_for<S, I>`:
    if constexpr (sized_sentinel_for<_Sp, _Ip>) {
      // If |n| >= |bound - i|, equivalent to `ranges::advance(i, bound)`.
      if (const auto __M = __bound - __i; __abs(__n) >= __abs(__M)) {
        (*this)(__i, __bound);
        return __n - __M;
      }

      // Otherwise, equivalent to `ranges::advance(i, n)`.
      (*this)(__i, __n);
      return 0;
    } else {
      // Otherwise, if `n` is non-negative, while `bool(i != bound)` is true, increments `i` but at
      // most `n` times.
      while (__i != __bound && __n > 0) {
        ++__i;
        --__n;
      }

      // Otherwise, while `bool(i != bound)` is true, decrements `i` but at most `-n` times.
      if constexpr (bidirectional_iterator<_Ip> && same_as<_Ip, _Sp>) {
        while (__i != __bound && __n < 0) {
          --__i;
          ++__n;
        }
      }
      return __n;
    }
  }
};

inline constexpr auto advance = __advance_fn(__functionish::__value);
} // namespace ranges

#endif // !defined(_LIBCPP_HAS_NO_RANGES)

_LIBCPP_END_NAMESPACE_STD

#endif // _LIBCPP___ITERATOR_PRIMITIVES_H
