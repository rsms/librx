// Copyright (c) 2012-2014 Rasmus Andersson <http://rsms.me/> See README.md for full MIT license.
#pragma once
#include <stdlib.h>

#ifndef _RX_INDIRECT_INCLUDE_
#error "do not include this file directly"
#endif

//-- begin RX_TARGET_ARCH_*
#if defined(__i386) || defined(__i386__) || defined(_M_IX86)
  #define RX_TARGET_ARCH_X86 1
#elif defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || \
      defined(_M_AMD64)
  #define RX_TARGET_ARCH_X64 1
#elif defined(__arm__) || defined(__arm) || defined(__ARM__) || defined(__ARM) || defined(__arm64__)
  #define RX_TARGET_ARCH_ARM 1
  #if defined(__arm64__)
    #define RX_TARGET_ARCH_ARM64 1
  #endif
// #elif defined(__ppc__) || defined(__ppc) || defined(__PPC__) || \
//       defined(__PPC) || defined(__powerpc__) || defined(__powerpc) || \
//       defined(__POWERPC__) || defined(__POWERPC) || defined(_M_PPC)
//   #ifdef __NO_FPRS__
//     #define RX_TARGET_ARCH_PPCSPE 1
//   #else
//     #define RX_TARGET_ARCH_PPC 1
//   #endif
// #elif defined(__mips__) || defined(__mips) || defined(__MIPS__) || \
//       defined(__MIPS)
//   #define RX_TARGET_ARCH_MIPS 1
#else
  #error "Unsupported target architecture"
#endif

#if RX_TARGET_ARCH_X64
  #define RX_TARGET_ARCH_NAME     "x64"
  #define RX_TARGET_ARCH_SIZE     64
  #define RX_TARGET_ARCH_LE       1
#elif RX_TARGET_ARCH_X86
  #define RX_TARGET_ARCH_NAME     "x86"
  #define RX_TARGET_ARCH_SIZE     32
  #define RX_TARGET_ARCH_LE       1
#elif RX_TARGET_ARCH_ARM
  #if defined(__ARMEB__)
    #error "Unsupported target architecture: Big endian ARM"
  #endif
  #if RX_TARGET_ARCH_ARM64
    #define RX_TARGET_ARCH_NAME     "arm64"
    #define RX_TARGET_ARCH_SIZE     64
  #else
    #define RX_TARGET_ARCH_NAME     "arm"
    #define RX_TARGET_ARCH_SIZE     32
  #endif
  #define RX_TARGET_ARCH_LE       1
#else
  #define RX_TARGET_ARCH_NAME     "?"
  #define RX_TARGET_ARCH_SIZE     0
  #define RX_TARGET_ARCH_LE       0
#endif
//-- end RX_TARGET_ARCH_*

//-- begin RX_TARGET_OS_*
#if defined(_WIN32) && !defined(_XBOX_VER)
  #define RX_TARGET_OS_WINDOWS 1
  #define RX_TARGET_OS_NAME "win32"
#elif defined(__linux__)
  #define RX_TARGET_OS_LINUX 1
  #define RX_TARGET_OS_POSIX 1
  #define RX_TARGET_OS_NAME "linux"
#elif defined(__MACH__) && defined(__APPLE__)
  #include <TargetConditionals.h>
  #define RX_TARGET_OS_DARWIN 1
  #define RX_TARGET_OS_POSIX 1
  #if TARGET_OS_IPHONE
    #define RX_TARGET_OS_IOS 1
    #if TARGET_IPHONE_SIMULATOR
      #define RX_TARGET_OS_NAME "ios-simulator"
      #define RX_TARGET_OS_IOS_SIMULATOR 1
    #else
      #define RX_TARGET_OS_NAME "ios"
      #define RX_TARGET_OS_IOS_SIMULATOR 0
    #endif
  #elif TARGET_OS_MAC
    #define RX_TARGET_OS_OSX 1
    #define RX_TARGET_OS_NAME "osx"
  #else
    #define RX_TARGET_OS_NAME "darwin"
  #endif
#elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || \
      defined(__NetBSD__) || defined(__OpenBSD__)
  #define RX_TARGET_OS_BSD 1
  #define RX_TARGET_OS_POSIX 1
  #define RX_TARGET_OS_NAME "bsd"
#elif (defined(__sun__) && defined(__svr4__)) || defined(__solaris__) || \
      defined(__CYGWIN__)
  #define RX_TARGET_OS_POSIX 1
  #define RX_TARGET_OS_NAME "posix"
#else
  #define RX_TARGET_OS_UNKNOWN 1
  #define RX_TARGET_OS_NAME "?"
#endif
//-- end RX_TARGET_OS_*

//-- begin RX_TARGET_BUILD_*
#if !defined(__GXX_RTTI) || !__GXX_RTTI
  #define RX_TARGET_BUILD_NO_CXX_RTTI
#endif

#if RX_TARGET_OS_WINDOWS && !defined(_CPPUNWIND)
  #define RX_TARGET_BUILD_NO_CXX_EXCEPTIONS
#elif !defined(__EXCEPTIONS) || !__EXCEPTIONS
  #define RX_TARGET_BUILD_NO_CXX_EXCEPTIONS
#endif
//-- end RX_TARGET_BUILD_*
