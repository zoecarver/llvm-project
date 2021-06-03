// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#ifndef _LIBCPP___ALGORITHM_ALGORITHM_FUNCTIONAL_H
#define _LIBCPP___ALGORITHM_ALGORITHM_FUNCTIONAL_H

#include <__config>
#include <type_traits>

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#pragma GCC system_header
#endif

_LIBCPP_PUSH_MACROS
#include <__undef_macros>

_LIBCPP_BEGIN_NAMESPACE_STD

// I'd like to replace these with _VSTD::equal_to<void>, but can't because that only works with C++14 and later.
template <class _T1, class _T2 = _T1>
struct __equal_to {
  _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX11 bool operator()(const _T1& __x, const _T1& __y) const {
    return __x == __y;
  }
  _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX11 bool operator()(const _T1& __x, const _T2& __y) const {
    return __x == __y;
  }
  _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX11 bool operator()(const _T2& __x, const _T1& __y) const {
    return __x == __y;
  }
  _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX11 bool operator()(const _T2& __x, const _T2& __y) const {
    return __x == __y;
  }
};

template <class _T1>
struct __equal_to<_T1, _T1> {
  _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX11 bool operator()(const _T1& __x, const _T1& __y) const {
    return __x == __y;
  }
};

template <class _T1>
struct __equal_to<const _T1, _T1> {
  _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX11 bool operator()(const _T1& __x, const _T1& __y) const {
    return __x == __y;
  }
};

template <class _T1>
struct __equal_to<_T1, const _T1> {
  _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX11 bool operator()(const _T1& __x, const _T1& __y) const {
    return __x == __y;
  }
};

template <class _T1, class _T2 = _T1>
struct __less {
  _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX11 bool operator()(const _T1& __x, const _T1& __y) const {
    return __x < __y;
  }

  _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX11 bool operator()(const _T1& __x, const _T2& __y) const {
    return __x < __y;
  }

  _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX11 bool operator()(const _T2& __x, const _T1& __y) const {
    return __x < __y;
  }

  _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX11 bool operator()(const _T2& __x, const _T2& __y) const {
    return __x < __y;
  }
};

template <class _T1>
struct __less<_T1, _T1> {
  _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX11 bool operator()(const _T1& __x, const _T1& __y) const {
    return __x < __y;
  }
};

template <class _T1>
struct __less<const _T1, _T1> {
  _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX11 bool operator()(const _T1& __x, const _T1& __y) const {
    return __x < __y;
  }
};

template <class _T1>
struct __less<_T1, const _T1> {
  _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR_AFTER_CXX11 bool operator()(const _T1& __x, const _T1& __y) const {
    return __x < __y;
  }
};

template <class _Predicate>
class __invert // invert the sense of a comparison
{
private:
  _Predicate __p_;

public:
  _LIBCPP_INLINE_VISIBILITY __invert() {}

  _LIBCPP_INLINE_VISIBILITY
  explicit __invert(_Predicate __p) : __p_(__p) {}

  template <class _T1>
  _LIBCPP_INLINE_VISIBILITY bool operator()(const _T1& __x) {
    return !__p_(__x);
  }

  template <class _T1, class _T2>
  _LIBCPP_INLINE_VISIBILITY bool operator()(const _T1& __x, const _T2& __y) {
    return __p_(__y, __x);
  }
};

// Perform division by two quickly for positive integers (llvm.org/PR39129)

template <typename _Integral>
_LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR typename enable_if<is_integral<_Integral>::value, _Integral>::type
__half_positive(_Integral __value) {
  return static_cast<_Integral>(static_cast<typename make_unsigned<_Integral>::type>(__value) / 2);
}

template <typename _Tp>
_LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR typename enable_if<!is_integral<_Tp>::value, _Tp>::type
__half_positive(_Tp __value) {
  return __value / 2;
}

#ifdef _LIBCPP_DEBUG

template <class _Compare>
struct __debug_less {
  _Compare& __comp_;
  _LIBCPP_CONSTEXPR_AFTER_CXX17
  __debug_less(_Compare& __c) : __comp_(__c) {}

  template <class _Tp, class _Up>
  _LIBCPP_CONSTEXPR_AFTER_CXX17 bool operator()(const _Tp& __x, const _Up& __y) {
    bool __r = __comp_(__x, __y);
    if (__r)
      __do_compare_assert(0, __y, __x);
    return __r;
  }

  template <class _Tp, class _Up>
  _LIBCPP_CONSTEXPR_AFTER_CXX17 bool operator()(_Tp& __x, _Up& __y) {
    bool __r = __comp_(__x, __y);
    if (__r)
      __do_compare_assert(0, __y, __x);
    return __r;
  }

  template <class _LHS, class _RHS>
  _LIBCPP_CONSTEXPR_AFTER_CXX17 inline _LIBCPP_INLINE_VISIBILITY decltype((void)declval<_Compare&>()(declval<_LHS&>(),
                                                                                                     declval<_RHS&>()))
      __do_compare_assert(int, _LHS& __l, _RHS& __r) {
    _LIBCPP_ASSERT(!__comp_(__l, __r), "Comparator does not induce a strict weak ordering");
  }

  template <class _LHS, class _RHS>
  _LIBCPP_CONSTEXPR_AFTER_CXX17 inline _LIBCPP_INLINE_VISIBILITY void __do_compare_assert(long, _LHS&, _RHS&) {}
};

#endif // _LIBCPP_DEBUG

template <class _Comp>
struct __comp_ref_type {
  // Pass the comparator by lvalue reference. Or in debug mode, using a
  // debugging wrapper that stores a reference.
#ifndef _LIBCPP_DEBUG
  typedef typename add_lvalue_reference<_Comp>::type type;
#else
  typedef __debug_less<_Comp> type;
#endif
};

_LIBCPP_END_NAMESPACE_STD

_LIBCPP_POP_MACROS

#endif // _LIBCPP___ALGORITHM_ALGORITHM_FUNCTIONAL_H
