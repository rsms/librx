#pragma once

#ifndef _RX_INDIRECT_INCLUDE_
#error "do not include this file directly"
#endif

namespace rx {

// Reference-counted POD-style objects
struct ref_counted {
  volatile u32 __refcount = 1;
  virtual ~ref_counted() = default;
    // force vtable so that implementations can use vtables (or __refcount offset will be incorrect)
  void __retain() {
    rx_atomic_add_fetch((i32*)&__refcount, 1);
  }
  bool __release() {
    if (rx_atomic_sub_fetch(&__refcount, 1) == 0) {
      __dealloc();
      return true;
    }
    return false;
  }
  virtual void __dealloc() {
    // A `Imp` struct can override this. In the case you do override this, you must declare your
    // `Imp` publicly, like this:
    //
    //   struct foo { RX_REF_MIXIN(foo)
    //     struct Imp : ref_counted { void __dealloc(); };
    //   };
    //
    // Then internally define a subclass for actual implementation:
    //
    //   struct foo::S2 : foo::Imp {
    //     (actual members)
    //   };
    //
    // And finally provide an implementation for __dealloc:
    //
    //   void foo::Imp::__dealloc() {
    //     printf("About to deallocate foo::Imp\n");
    //     delete this;
    //   }
    //
    delete this;
  }
};

// Example:
//
//   lolcat.h:
//     struct lolcat { RX_REF_MIXIN(lolcat)
//       lolcat(const char* name);
//       const char* name() const; };
//  
//   lolcat.cc:
//     struct lolcat::Imp : rx::ref_counted { const char* name; };
//     lolcat::lolcat(const char* name) : self(new Imp{name}) {}
//     const char* lolcat::name() const { return self->name; }
//  
//   main.cc:
//     lolcat make_furball(bool b) {
//       return cat(b); }
//     int main() {
//       lolcat cat = make_furball("Busta Rhymes");
//       std::cout << cat.name() << "\n";
//       return 0; } // `cat` deallocated here
//
#define RX_REF_MIXIN(T) \
  public: \
    struct Imp; friend Imp; \
    RX_REF_MIXIN_IMPL(T, Imp)

#define RX_REF_MIXIN_IMPL(T,Imp) \
  Imp* self = nullptr; \
  \
  static void __retain(Imp* p) { if (p) ((::rx::ref_counted*)p)->__retain(); } \
  static void __release(Imp* p) { if (p) ((::rx::ref_counted*)p)->__release(); } \
  T(std::nullptr_t) : self(nullptr) {} \
  explicit T(Imp* p, bool add_ref=false) : self(p) { if (add_ref) { __retain(self); } } \
  T(T const& rhs) : self(rhs.self) { __retain(self); } \
  T(const T* rhs) : self(rhs->self) { __retain(self); } \
  T(T&& rhs) { self = std::move(rhs.self); rhs.self = 0; } \
  ~T() { __release(self); } \
  T& reset_self(const Imp* p = nullptr) const { \
    Imp* old = self; \
    __retain((const_cast<T*>(this)->self = const_cast<Imp*>(p))); \
    __release(old); \
    return *const_cast<T*>(this); \
  } \
  Imp* steal_self() { \
    Imp* p = self; \
    self = 0; \
    return std::move(p); \
  } \
  T& operator=(T&& rhs) { \
    if (self != rhs.self) { \
      __release(self); \
      self = rhs.self; \
    } \
    rhs.self = 0; \
    return *this; \
  } \
  T& operator=(const T& rhs) { return reset_self(rhs.self); } \
  T& operator=(Imp* rhs) { return reset_self(rhs); } \
  T& operator=(const Imp* rhs) { return reset_self(rhs); } \
  T& operator=(std::nullptr_t) { return reset_self(nullptr); } \
  Imp* operator->() const { return self; } \
  bool operator==(const T& other) const { return self == other.self; } \
  bool operator!=(const T& other) const { return self != other.self; } \
  bool operator==(std::nullptr_t) const { return self == nullptr; } \
  bool operator!=(std::nullptr_t) const { return self != nullptr; } \
  operator bool() const { return self != nullptr; }

// end of RX_REF_MIXIN

} // namespace
