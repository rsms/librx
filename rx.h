#pragma once

#define _RX_INDIRECT_INCLUDE_

// The classic stringifier macro
#define RX_STR1(str) #str
#define RX_STR(str) RX_STR1(str)

// Configuration
#ifndef RX_DEBUG
  #define RX_DEBUG 0
#elif RX_DEBUG && defined(NDEBUG)
  #undef NDEBUG
#endif

// Defines the host target
#include "rx-target.h"

#define RX_ABORT(fmt, ...) do { \
    fprintf(stderr, "*** " fmt " at " __FILE__ ":" RX_STR(__LINE__) "\n", \
      ##__VA_ARGS__); abort(); } while (0)

#define RX_NOT_IMPLEMENTED \
  RX_ABORT("NOT IMPLEMENTED in %s", __PRETTY_FUNCTION__)

#define RX_MAX(a,b) \
  ({ __typeof__ (a) _a = (a); \
     __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define RX_MIN(a,b) \
  ({ __typeof__ (a) _a = (a); \
     __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#ifndef __has_attribute
  #define __has_attribute(x) 0
#endif
#ifndef __has_builtin
  #define __has_builtin(x) 0
#endif

#if __has_attribute(always_inline)
  #define RX_ALWAYS_INLINE __attribute__((always_inline))
#else
  #define RX_ALWAYS_INLINE
#endif
#if __has_attribute(noinline)
  #define RX_NO_INLINE __attribute__((noinline))
#else
  #define RX_NO_INLINE
#endif
#if __has_attribute(unused)
  // Attached to a function, means that the function is meant to be possibly
  // unused. The compiler will not produce a warning for this function.
  #define RX_UNUSED __attribute__((unused))
#else
  #define RX_UNUSED
#endif
#if __has_attribute(warn_unused_result)
  #define RX_WUNUSEDR __attribute__((warn_unused_result))
#else
  #define RX_WUNUSEDR
#endif
#if __has_attribute(packed)
  #define RX_PACKED __attribute((packed))
#else
  #define RX_PACKED
#endif
#if __has_attribute(aligned)
  #define RX_ALIGNED(bytes) __attribute__((aligned (bytes)))
#else
  #warning "No align attribute available. Things might break"
  #define RX_ALIGNED
#endif

#if __has_builtin(__builtin_unreachable)
  #define RX_UNREACHABLE do { \
    assert(!"Declared UNREACHABLE but was reached"); __builtin_unreachable(); } while(0);
#else
  #define RX_UNREACHABLE assert(!"Declared UNREACHABLE but was reached");
#endif

#if __has_attribute(noreturn)
  #define RX_NORETURN __attribute__((noreturn))
#else
  #define RX_NORETURN 
#endif

typedef signed char           i8;
typedef unsigned char         u8;
typedef short                 i16;
typedef unsigned short        u16;
typedef int                   i32;
typedef unsigned int          u32;
#if defined(_WIN32)
  typedef __int64             i64;
  typedef unsigned __int64    u64;
#else
  typedef long long           i64;
  typedef unsigned long long  u64;
#endif

#if defined(__clang__) || (defined(__GNUC__) && (__GNUC__ >= 4))
  #define RX_I128_INTEGRAL 1
  typedef __uint128_t         u128;
// #else
//   #define RX_I128_STRUCT 1
//   typedef struct { unsigned long long high; unsigned long long low; } u128;
#endif

// libc
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "rx-atomic.h"
#ifdef __cplusplus
  // Caution: Must only include header-only libc++ headers here since we don't link with libc++
  #include <utility>
  
  #ifndef RX_USE_STDLIBCXX
  // If we don't link with libc++, we need to provide `new` and `delete`
  struct nothrow_t {};
  inline void* operator new (size_t z) { return malloc(z); }
  inline void* operator new (size_t z, const nothrow_t&) noexcept {
    return malloc(z); }
  // inline void* operator new (size_t, void* p) noexcept { return p; }
  inline void operator delete (void* p) noexcept { free(p); }
  inline void operator delete (void* p, const nothrow_t&) noexcept {
    free(p); }
  // inline void operator delete (void* p, void*) noexcept {}
  #endif // RX_USE_STDLIBCXX

  #include "rx-ref.hh"  // RX_REF*, rx::ref_counted
  #include "rx-func.hh" // rx::func
#endif

#undef _RX_INDIRECT_INCLUDE_
