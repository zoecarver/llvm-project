//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This is a dummy feature that prevents this test from running by default.

// The table below compares the compile time and object size for each of the
// variants listed in the RUN script.
//
//  Impl                       Compile Time      Object Size
// ----------------------------------------------------------
// new_remove_reference:       22.849 s          121 K
// std::remove_reference:      25.643 s          121 K
//
// RUN: %cxx %flags %compile_flags -c %s -o %S/orig.o -ggdb  -ggnu-pubnames -ftemplate-depth=5000 -ftime-trace -std=c++17
// RUN: %cxx %flags %compile_flags -c %s -o %S/new.o -ggdb  -ggnu-pubnames -ftemplate-depth=5000 -ftime-trace -std=c++17 -DTEST_NEW

#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "template_cost_testing.h"

template <int N> struct Arg { enum { value = 1 }; };

#ifdef TEST_NEW

template <class T>
struct new_remove_reference
{
  typedef __remove_reference(T) type;
};

#define TEST_CASE_NOP()  new_remove_reference< Arg< __COUNTER__ > >{},
#define TEST_CASE_TYPE() typename new_remove_reference< Arg< __COUNTER__ > >::type,

#else

#define TEST_CASE_NOP()  std::remove_reference< Arg< __COUNTER__ > >{},
#define TEST_CASE_TYPE() typename std::remove_reference< Arg< __COUNTER__ > >::type,

#endif

int sink(...);

int x = sink(
  REPEAT_10000(TEST_CASE_NOP)
  REPEAT_10000(TEST_CASE_NOP) 42
);

void Foo( REPEAT_10000(TEST_CASE_TYPE) int) { }

void escape() {

sink(&x);
sink(&Foo);
}


