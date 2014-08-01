// Copyright (c) 2012-2014 Rasmus Andersson <http://rsms.me/> See README.md for full MIT license.
#pragma once
#include <set>
#include <unordered_map>
#include <initializer_list>

namespace rx {

template <typename ID>
struct State {
  using Handler = rx::func<void()>;
  using Map     = std::unordered_map<ID, Handler>;
  rx::func<bool(const ID& from, const ID& to)> should_transition;
  
  State& operator()(ID new_identity) {
    auto I = _states.find(new_identity);
    assert(I != _states.end());
    if (!should_transition || should_transition(_identity, new_identity)) {
      _identity = new_identity;
      I->second();
    }
    return *this;
  }
  
  State(std::initializer_list<typename Map::value_type>&& il) : _states{il} {}
  State() {}
  State(const State&) = delete;
  State(State&&) = default;
  
  const ID& identity() const { return _identity; }
  Handler& operator[] (const ID& identity) { return _states[identity]; }
  
  rx::func<void()> deferred(const ID& next_identity) {
    return [=]() mutable {  operator()(next_identity);  };
  }
  
  rx::func<void(Status)> deferred(const ID& ok_identity, const ID& error_identity) {
    return [=](Status st) mutable {  operator()(st.ok() ? ok_identity : error_identity);  };
  }
  
  rx::func<void(Status)> deferredWithStatus(const ID& next_identity, rx::Status::Code ignore_status_code=-1) {
    return [=](Status st) mutable {
      if (!st.ok() && st.code() != ignore_status_code) {
        std::cerr << _identity << " error: " << st << std::endl;
      }
      operator()(next_identity);
    };
  }

  void clear() {
    _identity.clear();
    _states.clear();
  }
  
private:
  ID _identity;
  Map _states;
};

} // namespace
