//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03, c++11, c++14
// <optional>

// optional<T>& operator=(nullopt_t) noexcept;

#include <optional>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "archetypes.h"

using std::optional;
using std::nullopt_t;
using std::nullopt;

#if TEST_STD_VER >= 20
consteval bool test_consteval()
{
    enum class State { inactive, constructed, destroyed };
    State state = State::inactive;

    struct StateTracker {
      constexpr StateTracker(State& s)
      : state_(&s)
      {
        s = State::constructed;
      }
      constexpr ~StateTracker() { *state_ = State::destroyed; }

      State* state_;
    };
    {
        optional<int> opt;
        opt = nullopt;
        assert(static_cast<bool>(opt) == false);
    }
    {
        optional<int> opt(3);
        opt = nullopt;
        assert(static_cast<bool>(opt) == false);
    }
    {
        optional<StateTracker> opt;
        opt = nullopt;
        assert(state == State::inactive);
        assert(static_cast<bool>(opt) == false);
    }
    {
        optional<StateTracker> opt(state);
        assert(state == State::constructed);
        opt = nullopt;
        assert(state == State::destroyed);
        assert(static_cast<bool>(opt) == false);
    }
    return true;
}

static_assert(test_consteval());
#endif

int main(int, char**)
{
    {
        optional<int> opt;
        static_assert(noexcept(opt = nullopt) == true, "");
        opt = nullopt;
        assert(static_cast<bool>(opt) == false);
    }
    {
        optional<int> opt(3);
        opt = nullopt;
        assert(static_cast<bool>(opt) == false);
    }
    using TT = TestTypes::TestType;
    TT::reset();
    {
        optional<TT> opt;
        static_assert(noexcept(opt = nullopt) == true, "");
        assert(TT::destroyed == 0);
        opt = nullopt;
        assert(TT::constructed == 0);
        assert(TT::alive == 0);
        assert(TT::destroyed == 0);
        assert(static_cast<bool>(opt) == false);
    }
    assert(TT::alive == 0);
    assert(TT::destroyed == 0);
    TT::reset();
    {
        optional<TT> opt(42);
        assert(TT::destroyed == 0);
        TT::reset_constructors();
        opt = nullopt;
        assert(TT::constructed == 0);
        assert(TT::alive == 0);
        assert(TT::destroyed == 1);
        assert(static_cast<bool>(opt) == false);
    }
    assert(TT::alive == 0);
    assert(TT::destroyed == 1);
    TT::reset();

  return 0;
}
