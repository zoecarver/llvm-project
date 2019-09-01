// RUN: %clang_cc1 -std=c++11 %s

template<class T>
struct test
{
  static const bool value = __is_same(__add_lvalue_reference(T), T&);
};

struct Foo { };

template<class T>
struct Bar { };

template<class T>
class Baz { };

static_assert(test<int>::value, "");
static_assert(test<int[]>::value, "");
static_assert(test<int[8]>::value, "");
static_assert(test<Foo>::value, "");
static_assert(test<Bar<int>>::value, "");
static_assert(test<Baz<int>>::value, "");

// only adds lvalue
static_assert(__is_same(__add_lvalue_reference(int&),  int&));
static_assert(__is_same(__add_lvalue_reference(int&&), int&));



// static_assert(__is_same(__add_lvalue_reference(int&),  int&&));
// static_assert(__is_same(__add_lvalue_reference(int&&), int&&));