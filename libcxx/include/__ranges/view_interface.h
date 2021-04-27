// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#ifndef _LIBCPP___RANGES_VIEW_INTERFACE_H
#define _LIBCPP___RANGES_VIEW_INTERFACE_H

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

struct view_base { };

template <class _Tp>
inline constexpr bool enable_view = derived_from<_Tp, view_base>;

template <class _Tp>
concept view =
  range<_Tp> &&
  movable<_Tp> &&
  default_initializable<_Tp> &&
  enable_view<_Tp>;

template<class _Derived>
  requires is_class_v<_Derived> && same_as<_Derived, remove_cv_t<_Derived>>
class view_interface : public view_base {
  [[nodiscard]] constexpr _Derived& __derived() noexcept {
    return static_cast<_Derived&>(*this);
  }

  [[nodiscard]] constexpr _Derived const& __derived() const noexcept {
    return static_cast<_Derived const&>(*this);
  }

public:
  [[nodiscard]] constexpr bool empty()
    noexcept(noexcept(ranges::begin(__derived()) == ranges::end(__derived())))
    requires forward_range<_Derived>
  {
    return ranges::begin(__derived()) == ranges::end(__derived());
  }

  [[nodiscard]] constexpr bool empty() const
    noexcept(noexcept(ranges::begin(__derived()) == ranges::end(__derived())))
    requires forward_range<_Derived>
  {
    return ranges::begin(__derived()) == ranges::end(__derived());
  }

  [[nodiscard]] constexpr explicit operator bool()
    noexcept(noexcept(ranges::empty(__derived())))
    requires requires { ranges::empty(__derived()); }
  {
    return !ranges::empty(__derived());
  }

  [[nodiscard]] constexpr explicit operator bool() const
    noexcept(noexcept(ranges::empty(__derived())))
    requires requires { ranges::empty(__derived()); }
  {
    return !ranges::empty(__derived());
  }

  [[nodiscard]] constexpr auto data()
    noexcept(noexcept(_VSTD::to_address(ranges::begin(__derived()))))
    requires contiguous_iterator<iterator_t<_Derived>>
  {
    return _VSTD::to_address(ranges::begin(__derived()));
  }

  [[nodiscard]] constexpr auto data() const
    noexcept(noexcept(_VSTD::to_address(ranges::begin(__derived()))))
    requires range<const _Derived> && contiguous_iterator<iterator_t<const _Derived>>
  {
    return _VSTD::to_address(ranges::begin(__derived()));
  }

  [[nodiscard]] constexpr auto size()
    noexcept(noexcept(ranges::end(__derived()) - ranges::begin(__derived())))
    requires forward_range<_Derived>
      && sized_sentinel_for<sentinel_t<_Derived>, iterator_t<_Derived>>
  {
    return ranges::end(__derived()) - ranges::begin(__derived());
  }

  [[nodiscard]] constexpr auto size() const
    noexcept(noexcept(ranges::end(__derived()) - ranges::begin(__derived())))
    requires forward_range<_Derived>
      && sized_sentinel_for<sentinel_t<_Derived>, iterator_t<_Derived>>
  {
    return ranges::end(__derived()) - ranges::begin(__derived());
  }

  [[nodiscard]] constexpr decltype(auto) front()
    noexcept(noexcept(*ranges::begin(__derived())))
    requires forward_range<_Derived>
  {
    _LIBCPP_ASSERT(!empty(), "");
    return *ranges::begin(__derived());
  }

  [[nodiscard]] constexpr decltype(auto) front() const
    noexcept(noexcept(*ranges::begin(__derived())))
    requires forward_range<const _Derived>
  {
    _LIBCPP_ASSERT(!empty(), "");
    return *ranges::begin(__derived());
  }

  [[nodiscard]] constexpr decltype(auto) back()
    noexcept(noexcept(*(--ranges::end(__derived()))))
    requires bidirectional_range<_Derived> && common_range<_Derived>
  {
    _LIBCPP_ASSERT(!empty(), "");
    return *(--ranges::end(__derived()));
  }

  [[nodiscard]] constexpr decltype(auto) back() const
    noexcept(noexcept(*(--ranges::end(__derived()))))
    requires bidirectional_range<const _Derived> && common_range<const _Derived>
  {
    _LIBCPP_ASSERT(!empty(), "");
    return *(--ranges::end(__derived()));
  }

  template<random_access_range _RARange = _Derived>
  [[nodiscard]] constexpr decltype(auto) operator[](range_difference_t<_RARange> __index)
    noexcept(noexcept(ranges::begin(__derived())[__index]))
  {
    return ranges::begin(__derived())[__index];
  }

  template<random_access_range _RARange = const _Derived>
  [[nodiscard]] constexpr decltype(auto) operator[](range_difference_t<_RARange> __index) const
    noexcept(noexcept(ranges::begin(__derived())[__index]))
  {
    return ranges::begin(__derived())[__index];
  }
};

}

#endif // !defined(_LIBCPP_HAS_NO_RANGES)

_LIBCPP_END_NAMESPACE_STD

_LIBCPP_POP_MACROS

#endif // _LIBCPP___RANGES_VIEW_INTERFACE_H
