//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef CALLABLE_TYPES_H
#define CALLABLE_TYPES_H

#include "type_id.h"

///////////////////////////////////////////////////////////////////////////////
//                       CALLABLE TEST TYPES
///////////////////////////////////////////////////////////////////////////////

inline bool returns_true() { return true; }

template <class Ret = bool>
struct MoveOnlyCallable {
  MoveOnlyCallable(MoveOnlyCallable const&) = delete;
  MoveOnlyCallable(MoveOnlyCallable&& other)
      : value(other.value)
  { other.value = !other.value; }

  template <class ...Args>
  Ret operator()(Args&&...) { return Ret{value}; }

  explicit MoveOnlyCallable(bool x) : value(x) {}
  Ret value;
};

template <class Ret = bool>
struct CopyCallable {
  CopyCallable(CopyCallable const& other)
      : value(other.value) {}

  CopyCallable(CopyCallable&& other)
      : value(other.value) { other.value = !other.value; }

  template <class ...Args>
  Ret operator()(Args&&...) { return Ret{value}; }

  explicit CopyCallable(bool x) : value(x)  {}
  Ret value;
};


template <class Ret = bool>
struct ConstCallable {
  ConstCallable(ConstCallable const& other)
      : value(other.value) {}

  ConstCallable(ConstCallable&& other)
      : value(other.value) { other.value = !other.value; }

  template <class ...Args>
  Ret operator()(Args&&...) const { return Ret{value}; }

  explicit ConstCallable(bool x) : value(x)  {}
  Ret value;
};



template <class Ret = bool>
struct NoExceptCallable {
  NoExceptCallable(NoExceptCallable const& other)
      : value(other.value) {}

  template <class ...Args>
  Ret operator()(Args&&...) noexcept { return Ret{value}; }

  template <class ...Args>
  Ret operator()(Args&&...) const noexcept { return Ret{value}; }

  explicit NoExceptCallable(bool x) : value(x)  {}
  Ret value;
};

struct CopyAssignableWrapper {
  CopyAssignableWrapper(CopyAssignableWrapper const&) = default;
  CopyAssignableWrapper(CopyAssignableWrapper&&) = default;
  CopyAssignableWrapper& operator=(CopyAssignableWrapper const&) = default;
  CopyAssignableWrapper& operator=(CopyAssignableWrapper &&) = default;

  template <class ...Args>
  bool operator()(Args&&...) { return value; }

  explicit CopyAssignableWrapper(bool x) : value(x) {}
  bool value;
};


struct MoveAssignableWrapper {
  MoveAssignableWrapper(MoveAssignableWrapper const&) = delete;
  MoveAssignableWrapper(MoveAssignableWrapper&&) = default;
  MoveAssignableWrapper& operator=(MoveAssignableWrapper const&) = delete;
  MoveAssignableWrapper& operator=(MoveAssignableWrapper &&) = default;

  template <class ...Args>
  bool operator()(Args&&...) { return value; }

  explicit MoveAssignableWrapper(bool x) : value(x) {}
  bool value;
};

struct MemFunCallable {
  explicit MemFunCallable(bool x) : value(x) {}

  bool return_value() const { return value; }
  bool return_value_nc() { return value; }
  bool value;
};

enum CallType : unsigned {
  CT_None,
  CT_NonConst = 1,
  CT_Const = 2,
  CT_LValue = 4,
  CT_RValue = 8
};

inline constexpr CallType operator|(CallType LHS, CallType RHS) {
    return static_cast<CallType>(static_cast<unsigned>(LHS) | static_cast<unsigned>(RHS));
}

struct ForwardingCallObject {

  template <class ...Args>
  bool operator()(Args&&...) & {
      set_call<Args&&...>(CT_NonConst | CT_LValue);
      return true;
  }

  template <class ...Args>
  bool operator()(Args&&...) const & {
      set_call<Args&&...>(CT_Const | CT_LValue);
      return true;
  }

  // Don't allow the call operator to be invoked as an rvalue.
  template <class ...Args>
  bool operator()(Args&&...) && {
      set_call<Args&&...>(CT_NonConst | CT_RValue);
      return true;
  }

  template <class ...Args>
  bool operator()(Args&&...) const && {
      set_call<Args&&...>(CT_Const | CT_RValue);
      return true;
  }

  template <class ...Args>
  static void set_call(CallType type) {
      assert(last_call_type == CT_None);
      assert(last_call_args == nullptr);
      last_call_type = type;
      last_call_args = &makeArgumentID<Args...>();
  }

  template <class ...Args>
  static bool check_call(CallType type) {
      bool result =
           last_call_type == type
        && last_call_args
        && *last_call_args == makeArgumentID<Args...>();
      last_call_type = CT_None;
      last_call_args = nullptr;
      return result;
  }

  static CallType      last_call_type;
  static TypeID const* last_call_args;
};

CallType ForwardingCallObject::last_call_type = CT_None;
TypeID const* ForwardingCallObject::last_call_args = nullptr;

#endif // CALLABLE_TYPES_H