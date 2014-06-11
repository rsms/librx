// Copyright (c) 2012-2014 Rasmus Andersson <http://rsms.me/> See README.md for full MIT license.
#pragma once
#include "rx.h"
#include <set>

namespace rx {

struct StateMachine;

struct State {
  typedef func<void(const State& from_state)> TransFunc;
  struct SetCmp { bool operator()(const State*, const State*) const; };
  typedef std::set<const State*, SetCmp> NextStates;

  State(const NextStates&& states);
    // A non-terminal state which can be transitioned into `states`

  State(const NextStates&& states, const TransFunc on_enter) : _states{states}, _on_enter{on_enter} {};
  State(const NextStates&& states, const func<void()> on_enter)
      : State{std::move(states), [on_enter](const State&){ on_enter(); }} {}
    // A non-terminal state which can be transitioned into `states`, with trigger-on enter callback

  State() {}
    // A terminal state

  State(const TransFunc on_enter) : State{NextStates{}, on_enter} {}
  State(const func<void()> on_enter) : State{NextStates{}, on_enter} {}
    // A terminal state with trigger-on enter callback

  bool can_transition(const State& to) const;
    // Returns true if there's a direct path from `this` state `to` state.

private:
  friend struct StateMachine;
  const NextStates _states;
  const TransFunc  _on_enter;
};


struct StateMachine {
  StateMachine(const State& initial_state);

  bool transition(const State& from, const State& to);
    // Transition `this` mutable state variable `from` state `to` state in an atomic manner.
    // Returns true if successful.

  bool transition(const State& to);
    // Transition `this` mutable state variable from any state `to` state in an atomic manner.
    // Returns true if successful, or false if either `can_transition(to)` is false or if another
    // thread transitioned `this` variable before the calling thread tried.


// ------------------------------------------------------------------------------------------------
  static void __dealloc(State* self) {} // no delete
  static void __retain(State*) {} // no retain
  static bool __release(State*) { return false; } // no release
  RX_REF_MIXIN_BODY(StateMachine, State)
};

inline State::State(const NextStates&& states) : State{std::move(states), (TransFunc)nullptr} {};

inline bool State::SetCmp::operator()(const State* lhs, const State* rhs) const {
  return (intptr_t)lhs < (intptr_t)rhs;
}

inline bool State::can_transition(const State& to) const {
  return (_states.find(&to) != _states.end());
}

inline StateMachine::StateMachine(const State& initial_state)
  : self{const_cast<State* volatile>(&initial_state)} {}

inline bool StateMachine::transition(const State& from, const State& to) {
  auto from_state = self; // explicit load
  return (from_state != &from || from_state->_states.find(&to) == from_state->_states.end())
    ? false
    : compare_and_swap_self(from_state, &to)
      ? (to._on_enter != nullptr) ? ({ to._on_enter(*from_state); true; }) : true
      : false;
}

inline bool StateMachine::transition(const State& to) {
  auto from_state = self; // explicit load
  return (from_state->_states.find(&to) == from_state->_states.end())
    ? false
    : compare_and_swap_self(from_state, &to)
      ? (to._on_enter != nullptr) ? ({ to._on_enter(*from_state); true; }) : true
      : false;
}

} // namespace
