// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#ifndef _LIBCPP___RANGES_TRANSFORM_VIEW_H
#define _LIBCPP___RANGES_TRANSFORM_VIEW_H

#include <__config>
#include <__iterator/iterator_traits.h>
#include <__iterator/concepts.h>
#include <__ranges/access.h>
#include <__ranges/concepts.h>
#include <__ranges/empty.h>
#include <__ranges/semiregular_box.h>
#include <__ranges/view.h>
#include <__ranges/view_interface.h>
#include <type_traits>

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#pragma GCC system_header
#endif

_LIBCPP_PUSH_MACROS
#include <__undef_macros>

_LIBCPP_BEGIN_NAMESPACE_STD

#if !defined(_LIBCPP_HAS_NO_RANGES)

// REVIEW-NOTE: This is *not* part of the transform_view patch and will be added by a *separate* PR.
template<class _I1, class _I2 = _I1>
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

namespace ranges {

template<input_range _View, copy_constructible _Fn>
  requires view<_View> && is_object_v<_Fn> &&
           regular_invocable<_Fn&, range_reference_t<_View>> &&
           __referenceable<invoke_result_t<_Fn&, range_reference_t<_View>>>
class transform_view : public view_interface<transform_view<_View, _Fn>> {
  template<bool> class __iterator;
  template<bool> class __sentinel;

  _View __base = _View();
  __semiregular_box<_Fn> __func;

public:
  transform_view() = default;
  constexpr transform_view(_View __base, _Fn __func)
    : __base(_VSTD::move(__base)), __func(_VSTD::move(__func)) {}

  constexpr _View base() const& requires copy_constructible<_View> { return __base; }
  constexpr _View base() && { return _VSTD::move(__base); }

  constexpr __iterator<false> begin() { return __iterator<false>{*this, ranges::begin(__base)}; }
  constexpr __iterator<true>  begin() const
    requires range<const _View> &&
             regular_invocable<const _Fn&, range_reference_t<const _View>>
  {
    return __iterator<true>{*this, ranges::begin(__base)};
  }

  constexpr __sentinel<false> end() { return __sentinel<false>{ranges::end(__base)}; }
  constexpr __iterator<false> end()
    requires common_range<_View>
  {
    return __iterator<false>{*this, ranges::end(__base)};
  }
  constexpr __sentinel<true> end() const
    requires range<const _View> &&
             regular_invocable<const _Fn&, range_reference_t<const _View>>
  {
    return __sentinel<true>{ranges::end(__base)};
  }
  constexpr __iterator<true> end() const
    requires common_range<const _View> &&
             regular_invocable<const _Fn&, range_reference_t<const _View>>
  {
    return __iterator<true>{*this, ranges::end(__base)};
  }

  constexpr auto size() requires sized_range<_View> { return ranges::size(__base); }
  constexpr auto size() const requires sized_range<const _View> { return ranges::size(__base); }
};

template<class _Range, class _Fn>
transform_view(_Range&&, _Fn)
  -> transform_view<decltype(views::all(std::declval<_Range>())), _Fn>;

template<class _View>
struct __iterator_concept {
  using type = conditional_t<
    random_access_range<_View>,
    random_access_iterator_tag,
    conditional_t<
      bidirectional_range<_View>,
      bidirectional_iterator_tag,
      conditional_t<
        forward_range<_View>,
        forward_iterator_tag,
        input_iterator_tag
      >
    >
  >;
};

template<class, class>
struct __iterator_category_base {};

template<forward_range _View, class _Fn>
struct __iterator_category_base<_View, _Fn> {
  using _Cat = typename iterator_traits<iterator_t<_View>>::iterator_category;

  using iterator_category = conditional_t<
    is_lvalue_reference_v<invoke_result_t<_Fn&, range_reference_t<_View>>>,
    conditional_t<
      derived_from<_Cat, contiguous_iterator_tag>,
      random_access_iterator_tag,
      _Cat
    >,
    input_iterator_tag
  >;
};

template<input_range _View, copy_constructible _Fn>
  requires view<_View> && is_object_v<_Fn> &&
           regular_invocable<_Fn&, range_reference_t<_View>> &&
           __referenceable<invoke_result_t<_Fn&, range_reference_t<_View>>>
template<bool _Const>
class transform_view<_View, _Fn>::__iterator
  : public __iterator_category_base<_View, _Fn> {

  using _Parent = __maybe_const<_Const, transform_view>;
  using _Base = __maybe_const<_Const, _View>;

  iterator_t<_Base> __current = iterator_t<_Base>();
  _Parent *__parent = nullptr;

  template<bool>
  friend class transform_view<_View, _Fn>::__sentinel;

public:
  using iterator_concept = typename __iterator_concept<_View>::type;
  using value_type = remove_cvref_t<invoke_result_t<_Fn&, range_reference_t<_Base>>>;
  using difference_type = range_difference_t<_Base>;

  __iterator() = default;

  constexpr __iterator(_Parent& __parent, iterator_t<_Base> __current)
    : __current(_VSTD::move(__current)), __parent(_VSTD::addressof(__parent)) {}
  constexpr __iterator(__iterator<!_Const> __i)
    requires _Const && convertible_to<iterator_t<_View>, iterator_t<_Base>>
    : __current(_VSTD::move(__i.__current)), __parent(__i.__parent) {}

  constexpr iterator_t<_Base> base() const&
    requires copyable<iterator_t<_Base>>
  {
    return __current;
  }
  constexpr iterator_t<_Base> base() && {
    return _VSTD::move(__current);
  }

  constexpr decltype(auto) operator*() const { return _VSTD::invoke(*__parent->__func, *__current); }

  constexpr __iterator& operator++() {
    ++__current;
    return *this;
  }
  constexpr void        operator++(int) { ++__current; }
  constexpr __iterator  operator++(int)
    requires forward_range<_Base>
  {
    auto __tmp = *this;
    ++*this;
    return __tmp;
  }

  constexpr __iterator& operator--()
    requires bidirectional_range<_Base>
  {
    --__current;
    return *this;
  }
  constexpr __iterator  operator--(int)
    requires bidirectional_range<_Base>
  {
    auto __tmp = *this;
    --*this;
    return __tmp;
  }

  constexpr __iterator& operator+=(difference_type __n)
    requires random_access_range<_Base>
  {
    __current += __n;
    return *this;
  }
  constexpr __iterator& operator-=(difference_type __n)
    requires random_access_range<_Base>
  {
    __current -= __n;
    return *this;
  }
  constexpr decltype(auto) operator[](difference_type __n) const
    requires random_access_range<_Base>
  {
    return _VSTD::invoke(*__parent->__func, __current[__n]);
  }

  friend constexpr bool operator==(const __iterator& __x, const __iterator& __y)
    requires equality_comparable<iterator_t<_Base>>
  {
    return __x.__current == __y.__current;
  }

  friend constexpr bool operator<(const __iterator& __x, const __iterator& __y)
    requires random_access_range<_Base>
  {
    return __x.__current < __y.__current;
  }
  friend constexpr bool operator>(const __iterator& __x, const __iterator& __y)
    requires random_access_range<_Base>
  {
    return __x.__current > __y.__current;
  }
  friend constexpr bool operator<=(const __iterator& __x, const __iterator& __y)
    requires random_access_range<_Base>
  {
    return __x.__current <= __y.__current;
  }
  friend constexpr bool operator>=(const __iterator& __x, const __iterator& __y)
    requires random_access_range<_Base>
  {
    return __x.__current >= __y.__current;
  }
//   friend constexpr auto operator<=>(const __iterator& __x, const __iterator& __y)
//     requires random_access_range<_Base> && three_way_comparable<iterator_t<_Base>>
//   {
//     return __x.__current <=> __y.__current;
//   }

  friend constexpr __iterator operator+(__iterator __i, difference_type __n)
    requires random_access_range<_Base>
  {
    return __iterator{*__i.__parent, __i.__current + __n};
  }
  friend constexpr __iterator operator+(difference_type __n, __iterator __i)
    requires random_access_range<_Base>
  {
    return __iterator{*__i.__parent, __i.__current + __n};
  }

  friend constexpr __iterator operator-(__iterator __i, difference_type __n)
    requires random_access_range<_Base>
  {
    return __iterator{*__i.__parent, __i.__current - __n};
  }
  friend constexpr difference_type operator-(const __iterator& __x, const __iterator& __y)
    requires sized_sentinel_for<iterator_t<_Base>, iterator_t<_Base>>
  {
    return __x.__current - __y.__current;
  }

  friend constexpr decltype(auto) iter_move(const __iterator& __i)
    // TODO: this noexcept looks wrong: why do we care about func's noexceptness?
    noexcept(noexcept(_VSTD::invoke(*__i.__parent->__func, *__i.__current)))
  {
    // TODO: this looks wrong: shouldn't this be the oposite.
    if constexpr (is_lvalue_reference_v<decltype(*__i)>)
      return _VSTD::move(*__i);
    else
      return *__i;
  }

  friend constexpr void iter_swap(const __iterator& __x, const __iterator& __y)
    noexcept(noexcept(ranges::iter_swap(__x.__current, __y.__current)))
    requires indirectly_swappable<iterator_t<_Base>>
  {
    ranges::iter_swap(__x.__current, __y.__current);
  }
};

template<input_range _View, copy_constructible _Fn>
  requires view<_View> && is_object_v<_Fn> &&
           regular_invocable<_Fn&, range_reference_t<_View>> &&
           __referenceable<invoke_result_t<_Fn&, range_reference_t<_View>>>
template<bool _Const>
class transform_view<_View, _Fn>::__sentinel {
  using _Parent = __maybe_const<_Const, transform_view>;
  using _Base = __maybe_const<_Const, _View>;

  sentinel_t<_Base> __end = sentinel_t<_Base>();

  template<bool>
  friend class transform_view<_View, _Fn>::__iterator;

  template<bool>
  friend class transform_view<_View, _Fn>::__sentinel;

public:
  __sentinel() = default;

  constexpr explicit __sentinel(sentinel_t<_Base> __end) : __end(__end) {}
  constexpr __sentinel(__sentinel<false> __i)
    requires _Const && convertible_to<sentinel_t<_View>, sentinel_t<_Base>>
    : __end(_VSTD::move(__i.__end)) {}

  constexpr sentinel_t<_Base> base() const { return __end; }

  template<bool _OtherConst>
    requires sentinel_for<sentinel_t<_Base>, iterator_t<__maybe_const<_OtherConst, _View>>>
  friend constexpr bool operator==(const __iterator<_OtherConst>& __x, const __sentinel __y) {
    return __x.__current == __y.__end;
  }

  template<bool _OtherConst>
    requires sized_sentinel_for<sentinel_t<_Base>, iterator_t<__maybe_const<_OtherConst, _View>>>
  friend constexpr range_difference_t<__maybe_const<_OtherConst, _View>>
  operator-(const __iterator<_OtherConst>& __x, const __sentinel& __y) {
    return __x.__current - __y.__end;
  }

  template<bool _OtherConst>
    requires sized_sentinel_for<sentinel_t<_Base>, iterator_t<__maybe_const<_OtherConst, _View>>>
  friend constexpr range_difference_t<__maybe_const<_OtherConst, _View>>
  operator-(const __sentinel& __x, const __iterator<_OtherConst>& __y) {
    return __x.__end - __y.__current;
  }
};

} // namespace ranges

#endif // !defined(_LIBCPP_HAS_NO_RANGES)

_LIBCPP_END_NAMESPACE_STD

_LIBCPP_POP_MACROS

#endif // _LIBCPP___RANGES_TRANSFORM_VIEW_H
