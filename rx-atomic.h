#pragma once
/*

T rx_atomic_swap(T *ptr, T value)
  Atomically swap integers or pointers in memory. Note that this is more than just CAS.
  E.g: int old_value = rx_atomic_swap(&value, new_value);

void rx_atomic_add32(i32* operand, i32 delta)
  Increment a 32-bit integer `operand` by `delta`. There's no return value.

T rx_atomic_add_fetch(T* operand, T delta)
  Add `delta` to `operand` and return the resulting value of `operand`

T rx_atomic_sub_fetch(T* operand, T delta)
  Subtract `delta` from `operand` and return the resulting value of `operand`

bool rx_atomic_cas_bool(T* ptr, T oldval, T newval)
  If the current value of *ptr is oldval, then write newval into *ptr. Returns true if the
  operation was successful and newval was written.

T rx_atomic_cas(T* ptr, T oldval, T newval)
  If the current value of *ptr is oldval, then write newval into *ptr. Returns the contents of
  *ptr before the operation.

-----------------------------------------------------------------------------*/

#ifndef _RX_INDIRECT_INCLUDE_
#error "do not include this file directly"
#endif

#define _RX_ATOMIC_HAS_SYNC_BUILTINS \
  defined(__clang__) || (defined(__GNUC__) && (__GNUC__ >= 4))

// T rx_atomic_swap(T *ptr, T value)
#if RX_WITHOUT_SMP
  #define rx_atomic_swap(ptr, value)  \
    ({ __typeof__ (value) oldval = *(ptr); \
       *(ptr) = (value); \
       oldval; })
#elif defined(__clang__)
  // This is more efficient than the below fallback
  #define rx_atomic_swap __sync_swap
#elif _RX_ATOMIC_HAS_SYNC_BUILTINS
  static inline void* RX_UNUSED _rx_atomic_swap(void* volatile* ptr, void* value) {
    void* oldval;
    do {
      oldval = *ptr;
    } while (__sync_val_compare_and_swap(ptr, oldval, value) != oldval);
    return oldval;
  }
  #define rx_atomic_swap(ptr, value) \
    _rx_atomic_swap((void* volatile*)(ptr), (void*)(value))
#else
  #error "Unsupported compiler: Missing support for atomic operations"
#endif

// void rx_atomic_add32(T* operand, T delta)
#if RX_WITHOUT_SMP
  #define rx_atomic_add32(operand, delta) (*(operand) += (delta))
#elif RX_TARGET_ARCH_X64 || RX_TARGET_ARCH_X86
  inline static void RX_UNUSED rx_atomic_add32(i32* operand, i32 delta) {
    // From http://www.memoryhole.net/kyle/2007/05/atomic_incrementing.html
    __asm__ __volatile__ (
      "lock xaddl %1, %0\n" // add delta to operand
      : // no output
      : "m" (*operand), "r" (delta)
    );
  }
  #ifdef __cplusplus
  inline static void RX_UNUSED rx_atomic_add32(u32* o, u32 d) {
    rx_atomic_add32((i32*)o, static_cast<i32>(d)); }
  inline static void RX_UNUSED rx_atomic_add32(volatile i32* o, volatile i32 d) {
    rx_atomic_add32((i32*)o, static_cast<i32>(d)); }
  inline static void RX_UNUSED rx_atomic_add32(volatile u32* o, volatile u32 d) {
    rx_atomic_add32((i32*)o, static_cast<i32>(d)); }
  #endif
#elif _RX_ATOMIC_HAS_SYNC_BUILTINS
  #define rx_atomic_add32 __sync_add_and_fetch
#else
  #error "Unsupported compiler: Missing support for atomic operations"
#endif

// T rx_atomic_sub_fetch(T* operand, T delta)
#if RX_WITHOUT_SMP
  #define rx_atomic_sub_fetch(operand, delta) (*(operand) -= (delta))
#elif _RX_ATOMIC_HAS_SYNC_BUILTINS
  #define rx_atomic_sub_fetch __sync_sub_and_fetch
#else
  #error "Unsupported compiler: Missing support for atomic operations"
#endif

// T rx_atomic_add_fetch(T* operand, T delta)
#if RX_WITHOUT_SMP
  #define rx_atomic_add_fetch(operand, delta) (*(operand) += (delta))
#elif _RX_ATOMIC_HAS_SYNC_BUILTINS
  #define rx_atomic_add_fetch __sync_add_and_fetch
#else
  #error "Unsupported compiler: Missing support for atomic operations"
#endif


// bool rx_atomic_cas_bool(T* ptr, T oldval, T newval)
#if RX_WITHOUT_SMP
  #define rx_atomic_cas_bool(ptr, oldval, newval)  \
    (*(ptr) == (oldval) && (*(ptr) = (newval)))
#elif _RX_ATOMIC_HAS_SYNC_BUILTINS
  #define rx_atomic_cas_bool(ptr, oldval, newval) \
    __sync_bool_compare_and_swap((ptr), (oldval), (newval))
#else
  #error "Unsupported compiler: Missing support for atomic operations"
#endif


// T rx_atomic_cas(T* ptr, T oldval, T newval)
#if RX_WITHOUT_SMP
  #define rx_atomic_cas(ptr, oldval, newval)  \
    ({ __typeof__(oldval) prevv = *(ptr); \
       if (*(ptr) == (oldval)) { *(ptr) = (newval); } \
       prevv; })
#elif _RX_ATOMIC_HAS_SYNC_BUILTINS
  #define rx_atomic_cas(ptr, oldval, newval) \
    __sync_val_compare_and_swap((ptr), (oldval), (newval))
#else
  #error "Unsupported compiler: Missing support for atomic operations"
#endif


// void rx_atomic_barrier()
#if RX_WITHOUT_SMP
#define rx_atomic_barrier() do{}while(0)
#else
#define rx_atomic_barrier() __sync_synchronize()
#endif

typedef volatile long rx_once_t;
#define RX_ONCE_INIT 0L
inline static bool RX_UNUSED rx_once(rx_once_t* token) {
  return *token == 0L && rx_atomic_cas_bool(token, 0L, 1L);
}


// Spinlock
#ifdef __cplusplus
namespace rx {

struct Spinlock {
  void lock() noexcept { while (!try_lock()); }
  bool try_lock() noexcept { return rx_atomic_cas_bool(&_v, 0L, 1L); }
  void unlock() noexcept { _v = 0L; }
private:
  volatile long _v = 0L;
};

struct ScopedSpinlock {
  ScopedSpinlock(Spinlock& lock) : _lock{lock} { _lock.lock(); }
  ~ScopedSpinlock() { _lock.unlock(); }
private:
  Spinlock& _lock;
};

} // namespace
#endif // __cplusplus
