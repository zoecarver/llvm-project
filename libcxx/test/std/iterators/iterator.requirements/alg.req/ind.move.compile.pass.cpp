//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03, c++11, c++14, c++17
// UNSUPPORTED: libcpp-no-concepts
// UNSUPPORTED: gcc-10

// template<class In, class Out>
// concept indirectly_movable;

// template<class In, class Out>
// concept indirectly_movable_storable;

#include <iterator>

#include "test_macros.h"

struct IndirectlyMovableWithInt {
  int& operator*() const;
};

struct Empty {};

struct MoveOnlyConvertible;
struct AssignableToMoveOnly;

struct MoveOnly {
  MoveOnly(MoveOnly&&) = default;
  MoveOnly(MoveOnly const&) = delete;
  MoveOnly& operator=(MoveOnly&&) = default;
  MoveOnly& operator=(MoveOnly const&) = delete;
  MoveOnly() = default;

  MoveOnly& operator=(MoveOnlyConvertible const&) = delete;
  MoveOnly& operator=(AssignableToMoveOnly const&);
};

struct MoveOnlyWrapper {
  using value_type = MoveOnly;
  MoveOnly& operator*() const;
};

static_assert( std::indirectly_movable<int*, int*>);
static_assert( std::indirectly_movable<const int*, int *>);
static_assert(!std::indirectly_movable<int*, const int *>);
static_assert(!std::indirectly_movable<const int*, const int *>);
static_assert( std::indirectly_movable<int*, int[2]>);
static_assert(!std::indirectly_movable<int[2], int*>);
static_assert(!std::indirectly_movable<int[2], int[2]>);
static_assert(!std::indirectly_movable<int(&)[2], int(&)[2]>);
static_assert(!std::indirectly_movable<int, int*>);
static_assert(!std::indirectly_movable<int, int>);
static_assert( std::indirectly_movable<Empty*, Empty*>);
static_assert(!std::indirectly_movable<Empty*, IndirectlyMovableWithInt>);
static_assert( std::indirectly_movable<int*, IndirectlyMovableWithInt>);
static_assert( std::indirectly_movable<MoveOnly*, MoveOnly*>);
static_assert(!std::indirectly_movable<MoveOnly*, const MoveOnly*>);
static_assert(!std::indirectly_movable<const MoveOnly*, const MoveOnly*>);
static_assert(!std::indirectly_movable<const MoveOnly*, MoveOnly*>);
static_assert( std::indirectly_movable<MoveOnlyWrapper, MoveOnlyWrapper>);
static_assert( std::indirectly_movable<MoveOnly*, MoveOnlyWrapper>);

struct MoveOnlyConvertible {
  operator MoveOnly&() const;
};

struct MoveOnlyConvertibleWrapper {
  using value_type = MoveOnlyConvertible;
  friend MoveOnly iter_move(MoveOnlyConvertibleWrapper const&);
  MoveOnly& operator*() const;
};

struct AssignableToMoveOnly {
  AssignableToMoveOnly() = default;
  AssignableToMoveOnly(const MoveOnly&);
};

struct AssignableToMoveOnlyWrapper {
  using value_type = AssignableToMoveOnly;
  friend MoveOnly iter_move(AssignableToMoveOnlyWrapper const&);
  MoveOnly& operator*() const;
};

struct DeletedMoveCtor {
  DeletedMoveCtor(DeletedMoveCtor&&) = delete;
  DeletedMoveCtor& operator=(DeletedMoveCtor&&) = default;
};

struct CommonType { };

struct NotConstructibleFromRefIn {
  struct ValueType {
    operator CommonType&() const;
  };

  struct ReferenceType {
    operator CommonType&() const;
  };

  using value_type = ValueType;
  ReferenceType& operator*() const;
};

template <template <class> class X, template <class> class Y>
struct std::basic_common_reference<NotConstructibleFromRefIn::ValueType,
                                   NotConstructibleFromRefIn::ReferenceType, X, Y> {
  using type = CommonType&;
};

template <template <class> class X, template <class> class Y>
struct std::basic_common_reference<NotConstructibleFromRefIn::ReferenceType,
                                   NotConstructibleFromRefIn::ValueType, X, Y> {
  using type = CommonType&;
};

struct NotAssignableFromRefIn {
  struct ReferenceType;

  struct ValueType {
    ValueType(ReferenceType);
    ValueType& operator=(ReferenceType) = delete;
    operator CommonType&() const;
  };

  struct ReferenceType {
    operator CommonType&() const;
  };

  using value_type = ValueType;
  ReferenceType& operator*() const;
};

template <template <class> class X, template <class> class Y>
struct std::basic_common_reference<NotAssignableFromRefIn::ValueType,
                                   NotAssignableFromRefIn::ReferenceType, X, Y> {
  using type = CommonType&;
};

template <template <class> class X, template <class> class Y>
struct std::basic_common_reference<NotAssignableFromRefIn::ReferenceType,
                                   NotAssignableFromRefIn::ValueType, X, Y> {
  using type = CommonType&;
};

struct AnyWritable {
  template<class T>
  AnyWritable& operator=(T&&);
};

struct AnyOutput {
  using value_type = AnyWritable;
  AnyWritable& operator*() const;
};

static_assert( std::indirectly_movable_storable<int*, int*>);
static_assert( std::indirectly_movable_storable<const int*, int *>);
static_assert( std::indirectly_movable_storable<int*, int[2]>);
static_assert( std::indirectly_movable_storable<Empty*, Empty*>);
static_assert( std::indirectly_movable_storable<MoveOnly*, MoveOnly*>);
static_assert( std::indirectly_movable<int*, IndirectlyMovableWithInt>);
static_assert( std::indirectly_movable_storable<MoveOnlyWrapper, MoveOnlyWrapper>);
static_assert( std::indirectly_movable<MoveOnlyConvertibleWrapper, MoveOnlyConvertibleWrapper>);
static_assert(!std::indirectly_movable_storable<MoveOnlyConvertibleWrapper, MoveOnlyConvertibleWrapper>);
static_assert(!std::indirectly_movable_storable<DeletedMoveCtor*, DeletedMoveCtor*>);
static_assert( std::indirectly_movable_storable<AssignableToMoveOnlyWrapper, AssignableToMoveOnlyWrapper>);
static_assert(!std::indirectly_movable_storable<NotConstructibleFromRefIn, AnyOutput>);
static_assert(!std::indirectly_movable_storable<NotAssignableFromRefIn, AnyOutput>);
