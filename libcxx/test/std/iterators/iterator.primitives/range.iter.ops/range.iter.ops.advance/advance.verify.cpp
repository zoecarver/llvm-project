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

// ranges::advance

#include <iterator>

namespace std::ranges {
class forward_iterator {
public:
  using value_type = int;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::forward_iterator_tag;

  forward_iterator() = default;

  value_type operator*() const;
  forward_iterator& operator++();
  forward_iterator operator++(int);

  bool operator==(forward_iterator const&) const = default;
};
} // namespace std::ranges

// The function templates defined in [range.iter.ops] are not found by argument-dependent name lookup ([basic.lookup.argdep]).
void no_adl_participation() {
  std::ranges::forward_iterator x;
  advance(x, 0); // expected-error{{use of undeclared identifier 'advance'}}
  advance(x, x); // expected-error {{use of undeclared identifier 'advance'}}
  // expected-error@*:*{{no matching function for call to '__convert_to_integral'}}
  advance(x, 0, x); // expected-error {{use of undeclared identifier 'advance'}}
}

namespace test {
template <class>
class forward_iterator {
public:
  using value_type = int;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::forward_iterator_tag;

  forward_iterator() = default;

  value_type operator*() const;
  forward_iterator& operator++();
  forward_iterator operator++(int);

  bool operator==(forward_iterator const&) const = default;
};

template <class I>
void advance(forward_iterator<I>&, std::ptrdiff_t) {
  static_assert(std::same_as<I, I*>);
}

template <class I>
void advance(forward_iterator<I>&, forward_iterator<I>) {
  static_assert(std::same_as<I, I*>);
}

template <class I>
void advance(forward_iterator<I>&, std::ptrdiff_t, forward_iterator<I>) {
  static_assert(std::same_as<I, I*>);
}
} // namespace test

// When found by unqualified ([basic.lookup.unqual]) name lookup for the postfix-expression in a
// function call ([expr.call]), they inhibit argument-dependent name lookup.
void adl_inhibition() {
  test::forward_iterator<int*> x;

  using std::ranges::advance;
  advance(x, 0);
  advance(x, x);
  (void)advance(x, 0, x);
}

void standard_function_properties() {
  &std::ranges::advance; // expected-error {{overload resolution selected deleted operator '&'}}
  auto move = std::move(std::ranges::advance);
  // expected-error@-1{{call to implicitly-deleted copy constructor of 'std::__1::ranges::__advance_fn'}}
  auto copy = std::ranges::advance;
  // expected-error@-1{{call to implicitly-deleted copy constructor of 'std::__1::ranges::__advance_fn'}}
  decltype(std::ranges::advance) x;                    // expected-error {{}}
  struct advance2 : decltype(std::ranges::advance) {}; // expected-error {{base '__advance_fn' is marked 'final'}}
}
