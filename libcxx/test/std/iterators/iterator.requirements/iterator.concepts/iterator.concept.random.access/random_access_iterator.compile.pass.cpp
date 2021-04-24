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
// XFAIL: msvc && clang

// template<class T>
// concept random_access_iterator;

#include <iterator>

#include "test_iterators.h"

static_assert(!std::random_access_iterator<input_iterator<int*> >);
static_assert(!std::random_access_iterator<forward_iterator<int*> >);
static_assert(!std::random_access_iterator<bidirectional_iterator<int*> >);
static_assert( std::random_access_iterator<random_access_iterator<int*> >);
static_assert( std::random_access_iterator<contiguous_iterator<int*> >);

static_assert(std::random_access_iterator<int*>);
static_assert(std::random_access_iterator<int const*>);
static_assert(std::random_access_iterator<int volatile*>);
static_assert(std::random_access_iterator<int const volatile*>);

template<class Parent>
struct simple_bidirectional_iterator {
    typedef std::random_access_iterator_tag iterator_category;
    typedef int                             value_type;
    typedef std::ptrdiff_t                  difference_type;
    typedef int*                            pointer;
    typedef int&                            reference;
    typedef Parent                          self;

    reference operator*() const;
    pointer operator->() const;
    friend bool operator==(const self&, const self&);
    friend bool operator< (const self&, const self&);
    friend bool operator<=(const self&, const self&);
    friend bool operator> (const self&, const self&);
    friend bool operator>=(const self&, const self&);

    self& operator++();
    self operator++(int);

    self& operator--();
    self operator--(int);
};

struct simple_random_access_iterator
  : simple_bidirectional_iterator<simple_random_access_iterator> {

    self& operator+=(difference_type n);
    self operator+(difference_type n) const;
    friend self operator+(difference_type n, self x);

    self& operator-=(difference_type n);
    self operator-(difference_type n) const;
    difference_type operator-(const self&) const;

    reference operator[](difference_type n) const;
};
static_assert(std::bidirectional_iterator<simple_random_access_iterator>);
static_assert(std::random_access_iterator<simple_random_access_iterator>);

struct no_plus_equals
  : simple_bidirectional_iterator<no_plus_equals> {

 /* self& operator+=(difference_type n); */
    self operator+(difference_type n) const;
    friend self operator+(difference_type n, self x);

    self& operator-=(difference_type n);
    self operator-(difference_type n) const;
    difference_type operator-(const self&) const;

    reference operator[](difference_type n) const;
};
static_assert( std::bidirectional_iterator<no_plus_equals>);
static_assert(!std::random_access_iterator<no_plus_equals>);

struct no_plus_difference_type
  : simple_bidirectional_iterator<no_plus_difference_type> {

    self& operator+=(difference_type n);
 /* self operator+(difference_type n) const; */
    friend self operator+(difference_type n, self x);

    self& operator-=(difference_type n);
    self operator-(difference_type n) const;
    difference_type operator-(const self&) const;

    reference operator[](difference_type n) const;
};
static_assert( std::bidirectional_iterator<no_plus_difference_type>);
static_assert(!std::random_access_iterator<no_plus_difference_type>);

struct difference_type_no_plus
  : simple_bidirectional_iterator<difference_type_no_plus> {

    self& operator+=(difference_type n);
    self operator+(difference_type n) const;
 /* friend self operator+(difference_type n, self x); */

    self& operator-=(difference_type n);
    self operator-(difference_type n) const;
    difference_type operator-(const self&) const;

    reference operator[](difference_type n) const;
};
static_assert( std::bidirectional_iterator<difference_type_no_plus>);
static_assert(!std::random_access_iterator<difference_type_no_plus>);

struct no_minus_equals
  : simple_bidirectional_iterator<no_minus_equals> {

    self& operator+=(difference_type n);
    self operator+(difference_type n) const;
    friend self operator+(difference_type n, self x);

 /* self& operator-=(difference_type n); */
    self operator-(difference_type n) const;
    difference_type operator-(const self&) const;

    reference operator[](difference_type n) const;
};
static_assert( std::bidirectional_iterator<no_minus_equals>);
static_assert(!std::random_access_iterator<no_minus_equals>);

struct no_minus
  : simple_bidirectional_iterator<no_minus> {

    self& operator+=(difference_type n);
    self operator+(difference_type n) const;
    friend self operator+(difference_type n, self x);

    self& operator-=(difference_type n);
 /* self operator-(difference_type n) const; */
    difference_type operator-(const self&) const;

    reference operator[](difference_type n) const;
};
static_assert( std::bidirectional_iterator<no_minus>);
static_assert(!std::random_access_iterator<no_minus>);

struct not_sized_sentinel
  : simple_bidirectional_iterator<not_sized_sentinel> {

    self& operator+=(difference_type n);
    self operator+(difference_type n) const;
    friend self operator+(difference_type n, self x);

    self& operator-=(difference_type n);
    self operator-(difference_type n) const;
 /* difference_type operator-(const self&) const; */

    reference operator[](difference_type n) const;
};
static_assert( std::bidirectional_iterator<not_sized_sentinel>);
static_assert(!std::random_access_iterator<not_sized_sentinel>);

struct no_subscript
  : simple_bidirectional_iterator<no_subscript> {

    self& operator+=(difference_type n);
    self operator+(difference_type n) const;
    friend self operator+(difference_type n, self x);

    self& operator-=(difference_type n);
    self operator-(difference_type n) const;
    difference_type operator-(const self&) const;

 /* reference operator[](difference_type n) const; */
};
static_assert( std::bidirectional_iterator<no_subscript>);
static_assert(!std::random_access_iterator<no_subscript>);
