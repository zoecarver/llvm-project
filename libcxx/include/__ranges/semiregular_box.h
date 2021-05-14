// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP___RANGES_SEMIREGULAR_BOX_H
#define _LIBCPP___RANGES_SEMIREGULAR_BOX_H

// These customization variables are used in <span> and <string_view>. The
// separate header is used to avoid including the entire <ranges> header in
// <span> and <string_view>.

#include <__config>
#include <concepts>
#include <optional>
#include <type_traits>

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#pragma GCC system_header
#endif

_LIBCPP_PUSH_MACROS
#include <__undef_macros>

_LIBCPP_BEGIN_NAMESPACE_STD

#if _LIBCPP_STD_VER > 17 && !defined(_LIBCPP_HAS_NO_RANGES)

// clang-format off

namespace ranges {
  template<copy_constructible _Tp>
  requires is_object_v<_Tp>
  class __semiregular_box : public optional<_Tp> {
  public:
    using optional<_Tp>::optional;
    __semiregular_box() = default;
    __semiregular_box(__semiregular_box&&) = default;
    __semiregular_box(const __semiregular_box&) = default;
    __semiregular_box& operator=(__semiregular_box&&) requires std::movable<_Tp> = default;
    __semiregular_box& operator=(const __semiregular_box&) requires std::copyable<_Tp> = default;

    constexpr __semiregular_box() noexcept(is_nothrow_default_constructible_v<_Tp>)
    requires default_initializable<_Tp>
    : optional<_Tp>{in_place} // braces intentional
    {}

    constexpr __semiregular_box& operator=(const __semiregular_box& other)
    noexcept(is_nothrow_copy_constructible_v<_Tp>)
    {
      if (other) {
        optional<_Tp>::emplace(*other);
      }
      else {
        optional<_Tp>::reset();
      }
      return *this;
    }

    constexpr __semiregular_box& operator=(__semiregular_box&& other)
    noexcept(is_nothrow_move_constructible_v<_Tp>)
    {
      if (other) {
        optional<_Tp>::emplace(std::move(*other));
      }
      else {
        optional<_Tp>::reset();
      }
      return *this;
    }
  };
} // namespace ranges

// clang-format on

#endif // _LIBCPP_STD_VER > 17 && !defined(_LIBCPP_HAS_NO_RANGES)

_LIBCPP_END_NAMESPACE_STD

_LIBCPP_POP_MACROS

#endif // _LIBCPP___RANGES_SEMIREGULAR_BOX_H
