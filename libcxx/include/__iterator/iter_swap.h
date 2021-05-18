// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#ifndef _LIBCPP___ITERATOR_SWAP_H
#define _LIBCPP___ITERATOR_SWAP_H

#include <__config>
#include <__iterator/iterator_traits.h>
#include <__iterator/concepts.h>
#include <__ranges/access.h>
#include <type_traits>

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#pragma GCC system_header
#endif

_LIBCPP_PUSH_MACROS
#include <__undef_macros>

_LIBCPP_BEGIN_NAMESPACE_STD

#if !defined(_LIBCPP_HAS_NO_RANGES)

namespace ranges {
namespace __iter_swap {
  template<class _I1, class _I2>
  void iter_swap(_I1, _I2) = delete;

  template<class _T1, class _T2>
  concept __unqualified_iter_swap = requires(_T1&& __x, _T2&& __y) {
    iter_swap(_VSTD::forward<_T1>(__x), _VSTD::forward<_T2>(__y));
  };

  template<class _T1, class _T2>
  concept __readable_swappable =
    indirectly_readable<_T1> && indirectly_readable<_T2> &&
    swappable_with<decltype(*declval<_T1>()), decltype(*declval<_T2>())>;

  template<class _T1, class _T2>
  constexpr iter_value_t<_T1> __iter_exchange_move(_T1&& __x, _T2 __y)
    noexcept(noexcept(iter_value_t<_T1>(iter_move(__x))) &&
             noexcept(*__x = iter_move(__y)))
  {
    iter_value_t<_T1> __old(iter_move(__x));
    *__x = iter_move(__y);
    return __old;
  }

  struct __fn {
    template <class _T1, class _T2>
      requires __unqualified_iter_swap<_T1, _T2>
    constexpr void operator()(_T1&& __x, _T2&& __y) const
      noexcept(noexcept(iter_swap(_VSTD::forward<_T1>(__x), _VSTD::forward<_T2>(__y))))
    {
      (void)iter_swap(_VSTD::forward<_T1>(__x), _VSTD::forward<_T2>(__y));
    }

    template <class _T1, class _T2>
      requires (!__unqualified_iter_swap<_T1, _T2>) &&
               __readable_swappable<_T1, _T2>
    constexpr void operator()(_T1&& __x, _T2&& __y) const
      noexcept(noexcept(ranges::swap(*_VSTD::forward<_T1>(__x), *_VSTD::forward<_T2>(__y))))
    {
      ranges::swap(*_VSTD::forward<_T1>(__x), *_VSTD::forward<_T2>(__y));
    }

    template <class _T1, class _T2>
      requires (!__unqualified_iter_swap<_T1, _T2> &&
                !__readable_swappable<_T1, _T2>) &&
               indirectly_movable_storable<_T1, _T2> &&
               indirectly_movable_storable<_T2, _T1>
    constexpr void operator()(_T1&& __x, _T2&& __y) const
      noexcept(noexcept(__iter_exchange_move(_VSTD::forward<_T2>(__y), _VSTD::forward<_T1>(__x))))
    {
      (void)(*_VSTD::forward<_T1>(__x) = __iter_exchange_move(_VSTD::forward<_T2>(__y), _VSTD::forward<_T1>(__x)));
    }
  };
} // end namespace __iter_swap

inline namespace __cpo {
  inline constexpr auto iter_swap = __iter_swap::__fn{};
} // namespace __cpo

} // namespace ranges

#endif // !defined(_LIBCPP_HAS_NO_RANGES)

_LIBCPP_END_NAMESPACE_STD

_LIBCPP_POP_MACROS

#endif // _LIBCPP___ITERATOR_SWAP_H
