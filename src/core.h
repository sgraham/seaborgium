// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Derived from bx:
// Copyright 2010-2013 Branimir Karadzic. All rights reserved.
// License: http://www.opensource.org/licenses/BSD-2-Clause

#ifndef CORE_H_
#define CORE_H_

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// --------------------------------------------------------------------------
//
// Platform.
//
// --------------------------------------------------------------------------

#define COMPILER_CLANG 0
#define COMPILER_GCC 0
#define COMPILER_MSVC 0

#define PLATFORM_LINUX 0
#define PLATFORM_OSX 0
#define PLATFORM_WINDOWS 0

#define CPU_X86 0

#define ARCH_32BIT 0
#define ARCH_64BIT 0

#define CPU_ENDIAN_BIG 0
#define CPU_ENDIAN_LITTLE 1

// Order is important here for clang is sneaky at emulation.
#if defined(__clang__)
#undef COMPILER_CLANG
#define COMPILER_CLANG 1
#elif defined(_MSC_VER)
#undef COMPILER_MSVC
#define COMPILER_MSVC 1
#elif defined(__GNUC__)
#undef COMPILER_GCC
#define COMPILER_GCC 1
#else
#error "COMPILER_* not defined!"
#endif

#if defined(_WIN32) || defined(_WIN64)
#undef PLATFORM_WINDOWS
#if !defined(WINVER) && !defined(_WIN32_WINNT)
// Windows 7+.
#define WINVER 0x0601
#define _WIN32_WINNT 0x0601
#endif  // !defined(WINVER) && !defined(_WIN32_WINNT)
#define PLATFORM_WINDOWS _WIN32_WINNT
#elif defined(__linux__)
#undef PLATFORM_LINUX
#define PLATFORM_LINUX 1
#elif defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__)
#undef PLATFORM_OSX
#define PLATFORM_OSX 1
#else
#error "PLATFORM_* is not defined!"
#endif

#define PLATFORM_POSIX (PLATFORM_OSX || PLATFORM_LINUX)

#if defined(_M_IX86) || defined(_M_X64) || defined(__i386__) || \
    defined(__x86_64__)
#undef CPU_X86
#define CPU_X86 1
#define CACHE_LINE_SIZE 64
#endif

#if defined(__x86_64__) || defined(_M_X64) || defined(__64BIT__)
#undef ARCH_64BIT
#define ARCH_64BIT 64
#else
#undef ARCH_32BIT
#define ARCH_32BIT 32
#endif

#if COMPILER_GCC
#define COMPILER_NAME "GCC"
#elif COMPILER_CLANG
#define COMPILER_NAME "Clang"
#elif COMPILER_MSVC
#define COMPILER_NAME "MSVC"
#endif

#if PLATFORM_LINUX
#define PLATFORM_NAME "Linux"
#elif PLATFORM_OSX
#define PLATFORM_NAME "OSX"
#elif PLATFORM_WINDOWS
#define PLATFORM_NAME "Windows"
#endif

#if CPU_X86
#define CPU_NAME "x86"
#endif

#if ARCH_32BIT
#define ARCH_NAME "32-bit"
#elif ARCH_64BIT
#define ARCH_NAME "64-bit"
#endif

// Includes, now that we know what we're targetting.
#if COMPILER_MSVC
#include <math.h>
#include <intrin.h>
#include <windows.h>
#endif

#if PLATFORM_POSIX
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <time.h>
#elif PLATFORM_WINDOWS
#include <limits.h>
#include <windows.h>  // NOLINT(build/include)
#endif

#undef DrawText
#undef GetObject

// --------------------------------------------------------------------------
//
// Helper macros.
//
// --------------------------------------------------------------------------

#define STRINGIZE(_x) STRINGIZE_(_x)
#define STRINGIZE_(_x) #_x

#define CONCATENATE(_x, _y) CONCATENATE_(_x, _y)
#define CONCATENATE_(_x, _y) _x##_y

#define FILE_LINE_LITERAL "" __FILE__ "(" STRINGIZE(__LINE__) "): "

#define ALIGN_MASK(_value, _mask) (((_value) + (_mask)) & ((~0) & (~(_mask))))
#define ALIGN_16(_value) ALIGN_MASK(_value, 0xf)
#define ALIGN_256(_value) ALIGN_MASK(_value, 0xff)
#define ALIGN_4096(_value) ALIGN_MASK(_value, 0xfff)

#define ALIGNOF(_type) __alignof(_type)

template <typename T, size_t N>
char(&COUNTOF_REQUIRES_ARRAY_ARGUMENT(const T(&)[N]))[N];  // NOLINT
#define COUNTOF(_x) sizeof(::COUNTOF_REQUIRES_ARRAY_ARGUMENT(_x))

#if COMPILER_GCC || COMPILER_CLANG
#define ALIGN_STRUCT(_align, struct) struct __attribute__((aligned(_align)))
#define ALLOW_UNUSED __attribute__((unused))
#define FORCE_INLINE \
  __extension__ static __inline __attribute__((__always_inline__))
#define FUNCTION __PRETTY_FUNCTION__
#define NO_INLINE __attribute__((noinline))
#define NO_RETURN __attribute__((noreturn))
#define NO_VTABLE
#elif COMPILER_MSVC
#define ALIGN_STRUCT(_align, struct) __declspec(align(_align)) struct
#define ALLOW_UNUSED
#define FORCE_INLINE __forceinline
#define FUNCTION __FUNCTION__
#define NO_INLINE __declspec(noinline)
#define NO_RETURN
#define NO_VTABLE __declspec(novtable)
#define THREAD __declspec(thread)
#else
#error "Unknown COMPILER_"
#endif

#define CACHE_LINE_ALIGN_MARKER() \
  ALIGN_STRUCT(CACHE_LINE_SIZE, struct) {}
#define CACHE_LINE_ALIGN(_def) \
  CACHE_LINE_ALIGN_MARKER();   \
  _def;                        \
  CACHE_LINE_ALIGN_MARKER()

#define ALIGN_STRUCT_16(struct) ALIGN_STRUCT(16, struct)
#define ALIGN_STRUCT_256(struct) ALIGN_STRUCT(256, struct)

#define UNUSED(a1)                         \
  do {                                     \
    (void)(true ? (void)0 : ((void)(a1))); \
  } while (0)

#ifndef CONFIG_DEBUG
#ifndef NDEBUG
#define CONFIG_DEBUG 1
#else
#define CONFIG_DEBUG 0
#endif
#endif

#ifndef CHECK
#define CHECK(condition, ...)         \
  do {                                \
    if (!(condition)) {               \
      TRACE("CHECK ", ##__VA_ARGS__); \
      ::DbgBreak();                   \
    }                                 \
  } while (0)
#endif  // CHECK

#ifndef DCHECK
#if CONFIG_DEBUG
#define DCHECK(condition, ...)         \
  do {                                 \
    if (!(condition)) {                \
      TRACE("DCHECK ", ##__VA_ARGS__); \
      ::DbgBreak();                    \
    }                                  \
  } while (0)
#else  // CONFIG_DEBUG
#define DCHECK(condition, ...) \
  do {                         \
  } while (0)
#endif  // CONFIG_DEBUG
#endif  // DCHECK

#ifndef TRACE
#if CONFIG_DEBUG
#define TRACE(format, ...)                                       \
  do {                                                           \
    ::DebugPrintf(FILE_LINE_LITERAL format "\n", ##__VA_ARGS__); \
  } while (0)
#else  // CONFIG_DEBUG
#define TRACE(...) \
  do {             \
  } while (0)
#endif  // CONFIG_DEBUG
#endif  // TRACE

#ifndef NOTREACHED
#define NOTREACHED() TRACE("Not reached!")
#endif

#define DISALLOW_COPY(TypeName) TypeName(const TypeName&)
#define DISALLOW_ASSIGN(TypeName) void operator=(const TypeName&)
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

#if COMPILER_MSVC
#pragma warning(disable : 4127)  // Conditional expression is constant.
#endif                           // COMPILER_MSVC

// --------------------------------------------------------------------------
//
// CPU-specific helpers.
//
// --------------------------------------------------------------------------

#if COMPILER_MSVC
extern "C" void _ReadBarrier();
extern "C" void _WriteBarrier();
extern "C" void _ReadWriteBarrier();
#pragma intrinsic(_ReadBarrier)
#pragma intrinsic(_WriteBarrier)
#pragma intrinsic(_ReadWriteBarrier)
#pragma intrinsic(_InterlockedIncrement)
#pragma intrinsic(_InterlockedDecrement)
#endif

inline void ReadBarrier() {
#if COMPILER_MSVC
  _ReadBarrier();
#elif COMPILER_GCC || COMPILER_CLANG
  asm volatile("" ::: "memory");
#endif
}

inline void WriteBarrier() {
#if COMPILER_MSVC
  _WriteBarrier();
#elif COMPILER_GCC || COMPILER_CLANG
  asm volatile("" ::: "memory");
#endif
}

inline void ReadWriteBarrier() {
#if COMPILER_MSVC
  _ReadWriteBarrier();
#elif COMPILER_GCC || COMPILER_CLANG
  asm volatile("" ::: "memory");
#endif
}

inline void MemBarrier() {
#if COMPILER_MSVC
  _mm_mfence();
#else
  __sync_synchronize();
#endif
}

inline int32_t AtomicIncr(volatile void* var) {
#if COMPILER_MSVC
  return _InterlockedIncrement(reinterpret_cast<volatile LONG*>(var));
#elif COMPILER_GCC || COMPILER_CLANG
  return __sync_fetch_and_add(reinterpret_cast<volatile int32_t*>(var), 1);
#endif
}

inline int32_t AtomicDecr(volatile void* var) {
#if COMPILER_MSVC
  return _InterlockedDecrement(reinterpret_cast<volatile LONG*>(var));
#elif COMPILER_GCC || COMPILER_CLANG
  return __sync_fetch_and_sub(reinterpret_cast<volatile int32_t*>(var), 1);
#endif
}

inline void* AtomicExchangePtr(void** _target, void* ptr) {
#if COMPILER_MSVC
  return InterlockedExchangePointer(_target, ptr);
#elif COMPILER_GCC || COMPILER_CLANG
  return __sync_lock_test_and_set(_target, ptr);
#endif
}

// --------------------------------------------------------------------------
//
// Timer.
//
// --------------------------------------------------------------------------

inline int64_t GetHPCounter() {
#if PLATFORM_WINDOWS
  LARGE_INTEGER li;
  // Performance counter value may unexpectedly leap forward
  // http://support.microsoft.com/kb/274323
  QueryPerformanceCounter(&li);
  int64_t i64 = li.QuadPart;
#else
  struct timeval now;
  gettimeofday(&now, 0);
  int64_t i64 = now.tv_sec * INT64_C(1000000) + now.tv_usec;
#endif
  return i64;
}

inline int64_t GetHPFrequency() {
#if PLATFORM_WINDOWS
  LARGE_INTEGER li;
  QueryPerformanceFrequency(&li);
  return li.QuadPart;
#else
  return INT64_C(1000000);
#endif
}

// --------------------------------------------------------------------------
//
// Debug.
//
// --------------------------------------------------------------------------

#if PLATFORM_OSX
#if defined(__OBJC__)
#import <Foundation/NSObjCRuntime.h>
#else
#include <CoreFoundation/CFString.h>
extern "C" void NSLog(CFStringRef format, ...);
#endif  // defined(__OBJC__)
#endif

inline void DbgBreak() {
#if COMPILER_MSVC
  __debugbreak();
#elif CPU_X86 && (COMPILER_GCC || COMPILER_CLANG)
  __asm__("int $3");
#else  // cross platform implementation
  int* int3 = reinterpret_cast<int*>(3L);
  *int3 = 3;
#endif
}

inline void DebugOutput(const char* out) {
#if PLATFORM_WINDOWS
  OutputDebugStringA(out);
  fputs(out, stdout);
  fflush(stdout);
#elif PLATFORM_OSX
#if defined(__OBJC__)
  NSLog(@"%s", out);
#else
  NSLog(__CFStringMakeConstantString("%s"), out);
#endif  // defined(__OBJC__)
#else
  fputs(out, stdout);
  fflush(stdout);
#endif
}

// Cross platform implementation of vsnprintf that returns number of characters
// which would have been written to the final string if enough space had been
// available.
inline int32_t Vsnprintf(char* str,
                         size_t count,
                         const char* format,
                         va_list arg_list) {
#if COMPILER_MSVC
  int32_t len = ::vsnprintf_s(str, count, count, format, arg_list);
  return len == -1 ? ::_vscprintf(format, arg_list) : len;
#else
  return ::vsnprintf(str, count, format, arg_list);
#endif  // COMPILER_MSVC
}

#if COMPILER_MSVC
inline int32_t snprintf(char* str, size_t count, const char* format, ...) {
  va_list arg_list;
  va_start(arg_list, format);
  int32_t len = Vsnprintf(str, count, format, arg_list);
  va_end(arg_list);
  return len;
}
#endif  // COMPILER_MSVC

inline void DebugPrintfVargs(const char* format, va_list arg_list) {
  char temp[8192];
  char* out = temp;
  int32_t len = Vsnprintf(out, sizeof(temp), format, arg_list);
  if ((int32_t)sizeof(temp) < len) {
    out = reinterpret_cast<char*>(_alloca(len + 1));
    len = Vsnprintf(out, len, format, arg_list);
  }
  out[len] = '\0';
  DebugOutput(out);
}

inline void DebugPrintf(const char* format, ...) {
  va_list arg_list;
  va_start(arg_list, format);
  DebugPrintfVargs(format, arg_list);
  va_end(arg_list);
}

#endif  // CORE_H_
