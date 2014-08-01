// Copyright (c) 2012-2014 Rasmus Andersson <http://rsms.me/> See README.md for full MIT license.
#include "netreach.hh"
#include <iostream>

#include <dispatch/dispatch.h>
#include <SystemConfiguration/SCNetworkReachability.h>

using std::cerr;
using std::endl;

namespace rx {

struct NetReachability::Imp : rx::ref_counted_novtable {
  Callback                 callback;
  SCNetworkReachabilityRef netReachRef = NULL;
  volatile State           state = State::UNREACHABLE;

  Imp(const std::string&, Callback&&);
  ~Imp() {
    if (netReachRef) {
      SCNetworkReachabilityUnscheduleFromRunLoop(netReachRef, CFRunLoopGetMain(), kCFRunLoopCommonModes);
      SCNetworkReachabilitySetCallback(netReachRef, NULL, NULL);
      CFRelease(netReachRef);
    }
  }
};

void NetReachability::__dealloc(NetReachability::Imp* self) { delete self; }


static NetReachability::State StateForFlags(SCNetworkConnectionFlags flags) {
  if ((flags & kSCNetworkFlagsReachable) && !(flags & kSCNetworkFlagsConnectionRequired)) {
    return NetReachability::REACHABLE;
  } else {
    return NetReachability::UNREACHABLE;
  }
}


static void ReachabilityCallback(
   SCNetworkReachabilityRef target,
   SCNetworkConnectionFlags flags,
   void* userdata)
{
  NetReachability::Imp* self = (NetReachability::Imp*)userdata;
  NetReachability::State state = StateForFlags(flags);

  // Observed flags:
  // - nearly gone: kSCNetworkFlagsReachable alone (ignored)
  // - gone: kSCNetworkFlagsTransientConnection | kSCNetworkFlagsReachable | kSCNetworkFlagsConnectionRequired
  // - connected: kSCNetworkFlagsIsDirect | kSCNetworkFlagsReachable

  auto prevState = rx_atomic_swap(&self->state, state);
  if (prevState != state) {
    if (self->callback) self->callback(state);
  }
}


NetReachability::NetReachability(const std::string& hostname, Callback cb)
  : self{new Imp{hostname, std::move(cb)}}
  {}


NetReachability::Imp::Imp(const std::string& hostname, Callback&& callback)
    : callback{callback}
{
  netReachRef = SCNetworkReachabilityCreateWithName(kCFAllocatorDefault, hostname.c_str());
  assert(netReachRef != NULL); // FIXME
  SCNetworkReachabilityContext context = {0, (void*)this, NULL, NULL, NULL};
  SCNetworkConnectionFlags flags;
  if (SCNetworkReachabilityGetFlags(netReachRef, &flags)) {
    state = StateForFlags(flags);
  }
  SCNetworkReachabilitySetCallback(netReachRef, ReachabilityCallback, &context);
  SCNetworkReachabilitySetDispatchQueue(netReachRef, dispatch_get_global_queue(
    #ifdef QOS_CLASS_UTILITY
    QOS_CLASS_UTILITY,
    #else
    DISPATCH_QUEUE_PRIORITY_LOW,
    #endif
    0));
}


NetReachability::State NetReachability::state() const {
  return self ? self->state : State::UNREACHABLE;
}


} // namespace
