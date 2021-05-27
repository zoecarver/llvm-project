// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP___ITERATOR_COMMON_ITERATOR_H
#define _LIBCPP___ITERATOR_COMMON_ITERATOR_H

#include <__config>
#include <__iterator/concepts.h>
#include <__iterator/iter_swap.h>
#include <__iterator/iterator_traits.h>
#include <__iterator/readable_traits.h>
// #include <variant>

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#pragma GCC system_header
#endif

_LIBCPP_PUSH_MACROS
#include <__undef_macros>

_LIBCPP_BEGIN_NAMESPACE_STD

#if !defined(_LIBCPP_HAS_NO_RANGES)

template<class _I1, class _I2>
concept indirectly_swappable =
    indirectly_readable<_I1> &&
    indirectly_readable<_I2> &&
    requires(const _I1 i1, const _I2 i2) {
        ranges::iter_swap(i1, i1);
        ranges::iter_swap(i1, i2);
        ranges::iter_swap(i2, i1);
        ranges::iter_swap(i2, i2);
    };

template<input_or_output_iterator _Iter, sentinel_for<_Iter> _Sent>
  requires (!same_as<_Iter, _Sent> && copyable<_Iter>)
class common_iterator {
  union {
    _Iter __iter;
    _Sent __sent;
  };

public:
  common_iterator() = default;

  constexpr common_iterator(_Iter __i);
  constexpr common_iterator(_Sent __s);

  template<class _I2, class _S2>
    requires convertible_to<const _I2, _Iter> && convertible_to<const _S2&, _Sent>
  common_iterator(const common_iterator<_I2, _S2>& __other);

  template<class _I2, class _S2>
    requires convertible_to<const _I2, _Iter> && convertible_to<const _S2&, _Sent> &&
             assignable_from<const _I2, _Iter> && assignable_from<const _S2&, _Sent>
  common_iterator& operator=(const common_iterator<_I2, _S2>& __other);

  decltype(auto) operator*();
  decltype(auto) operator*() const
    requires __dereferenceable<const _Iter>;
  decltype(auto) operator->() const
    requires indirectly_readable<const _Iter> &&
    (requires(const _Iter& __i) { __i.operator->(); } ||
     is_reference_v<iter_reference_t<_Iter>> ||
     constructible_from<iter_value_t<_Iter>, iter_reference_t<_Iter>>);

  common_iterator& operator++();
  decltype(auto) operator++(int);

  template<class _I2, sentinel_for<_Iter> _S2>
    requires sentinel_for<_Sent, _I2>
  friend bool operator==(const common_iterator& __x, const common_iterator<_I2, _S2>& __y);

  template<class _I2, sentinel_for<_Iter> _S2>
    requires sentinel_for<_Sent, _I2> && equality_comparable_with<_Iter, _I2>
  friend bool operator==(const common_iterator& __x, const common_iterator<_I2, _S2>& __y);

  template<sized_sentinel_for<_Iter> _I2, sized_sentinel_for<_Iter> _S2>
    requires sized_sentinel_for<_Sent, _I2>
  friend iter_difference_t<_I2> operator-(const common_iterator& __x, const common_iterator<_I2, _S2>& __y);

  friend iter_rvalue_reference_t<_Iter> iter_move(const common_iterator& __i)
    noexcept(noexcept(ranges::iter_move(declval<const _Iter&>())))
      requires input_iterator<_Iter>;

  template<indirectly_swappable<_Iter> _I2, class _S2>
    friend void iter_swap(const common_iterator& __x, const common_iterator<_I2, _S2>& __y)
      noexcept(noexcept(ranges::iter_swap(declval<const _Iter&>(), declval<const _I2&>())));
};

#endif // !defined(_LIBCPP_HAS_NO_RANGES)

_LIBCPP_END_NAMESPACE_STD

_LIBCPP_POP_MACROS

#endif // _LIBCPP___ITERATOR_COMMON_ITERATOR_H
