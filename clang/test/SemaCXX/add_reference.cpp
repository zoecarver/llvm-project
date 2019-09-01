// RUN: %clang_cc1 -std=c++11 %s

template<class T>
struct test_lval_trait
{
  typedef __add_lvalue_reference(T) type;
};

template<class T>
struct test_lval
{
  static const bool value = __is_same(typename test_lval_trait<T>::type, T&)  &&
                            __is_same(typename test_lval_trait<T&>::type, T&) &&
                            __is_same(typename test_lval_trait<T&&>::type, T&);
};

template<class T>
struct test_rval_trait
{
  typedef __add_rvalue_reference(T) type;
};

template<class T>
struct test_rval
{
  static const bool value = __is_same(typename test_rval_trait<T>::type, T&&)  &&
                            __is_same(typename test_rval_trait<T&>::type, T&&) &&
                            __is_same(typename test_rval_trait<T&&>::type, T&&);
};

template<class T>
struct test_remove_ref_trait
{
  typedef __remove_reference(T) type;
};

template<class T>
struct test_remove_ref
{
  static const bool value = __is_same(typename test_remove_ref_trait<T>::type, T)  &&
                            __is_same(typename test_remove_ref_trait<T&>::type, T) &&
                            __is_same(typename test_remove_ref_trait<T&&>::type, T);
};

template<class T>
struct test
{
  static const bool value = test_remove_ref<T>::value &&
                            test_rval<T>::value &&
                            test_lval<T>::value;
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
