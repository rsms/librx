// Copyright (c) 2012-2014 Rasmus Andersson <http://rsms.me/> See README.md for full MIT license.
#pragma once
namespace rx {

struct NetReachability { RX_REF_MIXIN_NOVTABLE(NetReachability)
  enum State {
    UNREACHABLE,
    REACHABLE,
  };
  using Callback = rx::func<void(State)>;
  NetReachability(); // == nullptr
  NetReachability(const std::string& hostname, Callback);
  State state() const;
};

} // namespace
