// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#ifndef _LIBCPP___RANGES_REF_VIEW_H
#define _LIBCPP___RANGES_REF_VIEW_H

#include <__config>
#include <__iterator/iterator_traits.h>
#include <__iterator/concepts.h>
#include <__ranges/access.h>
#include <__ranges/data.h>
#include <__ranges/view_interface.h>
#include <type_traits>

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#pragma GCC system_header
#endif

_LIBCPP_PUSH_MACROS
#include <__undef_macros>

_LIBCPP_BEGIN_NAMESPACE_STD

#if !defined(_LIBCPP_HAS_NO_RANGES)

// clang-format off
namespace ranges {

  template<class _Range>
  concept borrowed_range =
      range<_Range> &&
      (is_lvalue_reference_v<_Range> || enable_borrowed_range<remove_cvref_t<_Range>>);

  template<class _Range>
  concept sized_range = range<_Range> && requires(_Range& t) {
    ranges::size(t);
  };

  template<range _Range>
    requires is_object_v<_Range>
  class ref_view : public view_interface<ref_view<_Range>> {
    _Range *__range = nullptr;

public:
    constexpr ref_view() noexcept = default;

    static void __test(_Range&);
    static void __test(_Range&&) = delete;

    template<class _Tp>
      requires (!same_as<_Tp, ref_view>) &&
        convertible_to<_Tp, _Range&> && requires { __test(declval<_Tp>()); }
    constexpr ref_view(_Tp&& __t)
      : __range(_VSTD::addressof(static_cast<_Range&>(_VSTD::forward<_Tp>(__t))))
    {}

    constexpr _Range& base() const { return *__range; }

    constexpr iterator_t<_Range> begin() const { return ranges::begin(*__range); }
    constexpr sentinel_t<_Range> end() const { return ranges::end(*__range); }

    constexpr bool empty() const
      requires requires { ranges::empty(*__range); }
    { return ranges::empty(*__range); }

    constexpr auto size() const requires sized_range<_Range>
    { return ranges::size(*__range); }

    // TODO: This needs to use contiguous_range.
    constexpr auto data() const requires contiguous_iterator<iterator_t<_Range>>
    { return ranges::data(*__range); }
  };

  template<class _Range>
  ref_view(_Range&) -> ref_view<_Range>;

} // namespace ranges

// clang-format off

#endif // !defined(_LIBCPP_HAS_NO_RANGES)

_LIBCPP_END_NAMESPACE_STD

_LIBCPP_POP_MACROS

#endif // _LIBCPP___RANGES_REF_VIEW_H
