#pragma once
//
// Example:
//
//   foo.h:
//     struct foo {
//       foo(const char* name);
//       const char* name() const;
//       RX_REF_MIXIN(foo)
//     };
//  
//   foo.cc:
//     struct foo::Imp : rx::ref_counted { const char* name; };
//     foo::foo(const char* name) : self(new Imp{name}) {}
//     const char* foo::name() const { return self->name; }
//  
//   main.cc:
//     foo make_furball(bool b) {
//       return cat(b);
//     }
//     int main() {
//       foo cat = make_furball("Busta Rhymes");
//       std::cout << cat.name() << "\n";
//       return 0;
//     } // <- `cat` deallocated here
//
//
// Example without a vtable:
//
//   foo-novtable.h:
//     struct foo {
//       foo(const char* name);
//       const char* name() const;
//       RX_REF_MIXIN_NOVTABLE(foo)
//     };
//  
//   foo-novtable.cc:
//     struct foo::Imp : rx::ref_counted_novtable { const char* name; };
//     foo::foo(const char* name) : self(new Imp{name}) {}
//     const char* foo::name() const { return self->name; }
//     void foo::__dealloc(Imp* p) { delete p; }
//

#ifndef _RX_INDIRECT_INCLUDE_
#error "do not include this file directly"
#endif

namespace rx {

typedef u32 refcount_t;
#define RX_REF_COUNT_MEMBER volatile ::rx::refcount_t __refcount
#define RX_REF_COUNT_INIT ((::rx::refcount_t)1)
#define RX_REF_COUNT_CONSTANT ((::rx::refcount_t)0xffffffffu)

// Reference-counted POD-style objects
struct ref_counted {
  RX_REF_COUNT_MEMBER = RX_REF_COUNT_INIT;
  virtual ~ref_counted() = default;
  virtual void __dealloc() {
    // A `Imp` struct can override this. In the case you do override this, you must declare your
    // `Imp` publicly, like this:
    //
    //   struct foo { RX_REF_MIXIN(foo)
    //     struct Imp : rx::ref_counted { void __dealloc(); };
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

struct ref_counted_novtable {
  // If you don't want a vtable, this is the way (at the expense of no virtual destructors)
  RX_REF_COUNT_MEMBER = RX_REF_COUNT_INIT;
};


static inline RX_UNUSED void refcount_retain(volatile refcount_t& __refcount) {
  rx_atomic_add_fetch((i32*)&__refcount, 1);
}

static inline RX_UNUSED bool refcount_release(volatile refcount_t& __refcount) {
  return rx_atomic_sub_fetch(&__refcount, 1) == 0;
}


#define RX_REF_MIXIN(T) \
  public: \
    struct Imp; friend Imp; \
    RX_REF_MIXIN_IMPL_VTABLE(T, ::rx::ref_counted, Imp)

#define RX_REF_MIXIN_NOVTABLE(T) \
  public: \
    struct Imp; friend Imp; \
    static void __dealloc(Imp*); \
    RX_REF_MIXIN_IMPL_NOVTABLE(T, ::rx::ref_counted_novtable, Imp)


#define RX_REF_MIXIN_IMPL_VTABLE(T, OpaqueImp, Imp) \
  static void __retain(Imp* p) { \
    if (p && ((OpaqueImp*)p)->__refcount != RX_REF_COUNT_CONSTANT) \
      ::rx::refcount_retain(((OpaqueImp*)p)->__refcount); \
  } \
  static bool __release(Imp* p) { \
    return (p && ((OpaqueImp*)p)->__refcount != RX_REF_COUNT_CONSTANT && \
            ::rx::refcount_release(((OpaqueImp*)p)->__refcount) && \
            ({ ((OpaqueImp*)p)->__dealloc(); true; }) ); \
  } \
  RX_REF_MIXIN_BODY(T, Imp)


#define RX_REF_MIXIN_IMPL_NOVTABLE(T, OpaqueImp, Imp) \
  static void __retain(Imp* p) { \
    if (p && ((OpaqueImp*)p)->__refcount != RX_REF_COUNT_CONSTANT) \
      ::rx::refcount_retain(((OpaqueImp*)p)->__refcount); \
  } \
  static bool __release(Imp* p) { \
    return (p && ((OpaqueImp*)p)->__refcount != RX_REF_COUNT_CONSTANT && \
            ::rx::refcount_release(((OpaqueImp*)p)->__refcount) && ({ T::__dealloc(p); true; }) \
           ); \
  } \
  RX_REF_MIXIN_BODY(T, Imp)

// --------------------------------------------------------------------------------------------

#define RX_REF_MIXIN_BODY(T,Imp) \
  Imp* volatile self = nullptr; \
  \
  T(std::nullptr_t) : self{nullptr} {} \
  explicit T(Imp* p, bool add_ref=false) : self{p} { if (add_ref) { __retain(self); } } \
  T(T const& rhs) : self{rhs.self} { __retain(self); } \
  T(const T* rhs) : self{rhs->self} { __retain(self); } \
  T(T&& rhs) { self = std::move(rhs.self); rhs.self = 0; } \
  ~T() { __release(self); } \
  T& reset_self(const Imp* p = nullptr) const { \
    Imp* old = self; \
    __retain((const_cast<T*>(this)->self = const_cast<Imp*>(p))); \
    __release(old); \
    return *const_cast<T*>(this); \
  } \
  bool compare_and_swap_self(const Imp* old_self, const Imp* new_self) { \
    bool r = rx_atomic_cas(&self, const_cast<Imp*>(old_self), const_cast<Imp*>(new_self)); \
    return r ? ({ __release(const_cast<Imp*>(old_self)); __retain(self); true; }) : false; \
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
