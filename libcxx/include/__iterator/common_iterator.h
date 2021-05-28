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

// REVIEW-NOTE: This is *not* part of the transform_view patch and will be added by a *separate* PR.
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
// END-REVIEW-NOTE.

struct default_sentinel_t {};

template<input_or_output_iterator _Iter, sentinel_for<_Iter> _Sent>
  requires (!same_as<_Iter, _Sent> && copyable<_Iter>)
class common_iterator {
  union _Hold {
    _Iter __iter; // __index == 0;
    _Sent __sent; // __index == 1;

    _Hold() = default;
    _Hold(_Iter __iter) : __iter(_VSTD::move(__iter)) {}
    _Hold(_Sent __sent) : __sent(_VSTD::move(__sent)) {}
  } __u;
  bool __index;

  class __proxy {
    friend common_iterator;

    iter_value_t<_Iter> __value;
    __proxy(iter_reference_t<_Iter>&& __x)
      : __value(__x) {}

  public:
    const iter_value_t<_Iter>* operator->() const {
      return _VSTD::addressof(__value);
    }
  };

  class __postfix_proxy {
    friend common_iterator;

    iter_value_t<_Iter> __value;
    __postfix_proxy(iter_reference_t<_Iter>&& __x)
      : __value(__x) {}

  public:
    const iter_value_t<_Iter>& operator*() const {
      return __value;
    }
  };

public:
  common_iterator() = default;

  constexpr common_iterator(_Iter __i) : __u(_VSTD::move(__i)), __index(false) {}
  constexpr common_iterator(_Sent __s) : __u(_VSTD::move(__s)), __index(true ) {}

  template<class _I2, class _S2>
    requires convertible_to<const _I2, _Iter> && convertible_to<const _S2&, _Sent>
  constexpr common_iterator(const common_iterator<_I2, _S2>& __other)
    : __u(__other.__u), __index(__other.__index) {}

  template<class _I2, class _S2>
    requires convertible_to<const _I2, _Iter> && convertible_to<const _S2&, _Sent> &&
             assignable_from<const _I2, _Iter> && assignable_from<const _S2&, _Sent>
  common_iterator& operator=(const common_iterator<_I2, _S2>& __other) {
    __u = __other.__u;
    __index = __other.__index;
  }

  decltype(auto) operator*() { return *__u.__iter; }
  decltype(auto) operator*() const
    requires __dereferenceable<const _Iter>
  { return *__u.__iter; }

  decltype(auto) operator->() const
    requires indirectly_readable<const _Iter> &&
    (requires(const _Iter& __i) { __i.operator->(); } ||
     is_reference_v<iter_reference_t<_Iter>> ||
     constructible_from<iter_value_t<_Iter>, iter_reference_t<_Iter>>) {
    if constexpr (is_pointer_v<_Iter> || requires(const _Iter& __i) { __i.operator->(); })    {
      return __u.__iter;
    } else if constexpr (is_reference_v<iter_reference_t<_Iter>>) {
      auto&& __tmp = *__u.__iter;
      return _VSTD::addressof(__tmp);
    } else {
      return __proxy(*__u.__iter);
    }
  }

  common_iterator& operator++() { ++__u.__iter; return *this; }
  decltype(auto) operator++(int) {
    if constexpr (forward_iterator<_Iter>) {
      auto __tmp = *this;
      ++*this;
      return __tmp;
    } else if constexpr (requires (_Iter& __i) { { *__i++ } -> __referenceable; } ||
                         !constructible_from<iter_value_t<_Iter>, iter_reference_t<_Iter>>) {
      return __u.__iter++;
    } else {
      __postfix_proxy __p(*this);
      ++*this;
      return __p;
    }
  }

  template<class _I2, sentinel_for<_Iter> _S2>
    requires sentinel_for<_Sent, _I2>
  friend bool operator==(const common_iterator& __x, const common_iterator<_I2, _S2>& __y) {
    if (__x.__index == __y.__index)
      return true;

    if (__x.__index == 0)
      return __x.__u.__iter == __y.__u.__sent;

    return __x.__u.__sent == __y.__u.__iter;
  }

  template<class _I2, sentinel_for<_Iter> _S2>
    requires sentinel_for<_Sent, _I2> && equality_comparable_with<_Iter, _I2>
  friend bool operator==(const common_iterator& __x, const common_iterator<_I2, _S2>& __y) {
    if (__x.__index && __y.__index)
      return true;

    if (__x.__index == 0 && __y.__index == 0)
      return __x.__u.__iter == __y.__u.__iter;

    if (__x.__index == 0)
      return __x.__u.__iter == __y.__u.__sent;

    return __x.__u.__sent == __y.__u.__iter;
  }

  template<sized_sentinel_for<_Iter> _I2, sized_sentinel_for<_Iter> _S2>
    requires sized_sentinel_for<_Sent, _I2>
  friend iter_difference_t<_I2> operator-(const common_iterator& __x, const common_iterator<_I2, _S2>& __y) {
    if (__x.__index && __y.__index)
      return 0;

    if (__x.__index == 0 && __y.__index == 0)
      return __x.__u.__iter - __y.__u.__iter;

    if (__x.__index == 0)
      return __x.__u.__iter - __y.__u.__sent;

    return __x.__u.__sent - __y.__u.__iter;
  }

  friend iter_rvalue_reference_t<_Iter> iter_move(const common_iterator& __i)
    noexcept(noexcept(ranges::iter_move(declval<const _Iter&>())))
      requires input_iterator<_Iter>
  { return ranges::iter_move(__i.__u.__iter); }

  template<indirectly_swappable<_Iter> _I2, class _S2>
    friend void iter_swap(const common_iterator& __x, const common_iterator<_I2, _S2>& __y)
      noexcept(noexcept(ranges::iter_swap(declval<const _Iter&>(), declval<const _I2&>())))
  { return ranges::iter_swap(__x.__u.__iter, __y.__u.__iter); }
};

template<class _Iter, class _Sent>
struct incrementable_traits<common_iterator<_Iter, _Sent>> {
  using difference_type = iter_difference_t<_Iter>;
};

template<class _Iter>
concept __denotes_forward_iter = requires {
  { iterator_traits<_Iter>::iterator_category } -> derived_from<forward_iterator_tag>;
};

template<class _Iter, class _Sent>
concept __common_iter_has_ptr_op = requires(const common_iterator<_Iter, _Sent> __a) {
  __a.operator->();
};

template<class _Iter, class _Sent>
struct iterator_traits<common_iterator<_Iter, _Sent>> {
  using iterator_concept = conditional_t<forward_iterator<_Iter>,
                                         forward_iterator_tag,
                                         input_iterator_tag>;
  using iterator_category = conditional_t<__denotes_forward_iter<_Iter>,
                                          forward_iterator_tag,
                                          input_iterator_tag>;
  using pointer = conditional_t<__common_iter_has_ptr_op<_Iter, _Sent>,
                                decltype(declval<const common_iterator<_Iter, _Sent>>().operator->()),
                                void>;
  using value_type = iter_value_t<_Iter>;
  using difference_type = iter_difference_t<_Iter>;
  using reference = iter_reference_t<_Iter>;
};


#endif // !defined(_LIBCPP_HAS_NO_RANGES)

_LIBCPP_END_NAMESPACE_STD

_LIBCPP_POP_MACROS

#endif // _LIBCPP___ITERATOR_COMMON_ITERATOR_H
