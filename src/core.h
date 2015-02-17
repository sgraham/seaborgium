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
      ::DebugBreak();                 \
    }                                 \
  } while (0)
#endif  // CHECK

#ifndef DCHECK
#if CONFIG_DEBUG
#define DCHECK(condition, ...)         \
  do {                                 \
    if (!(condition)) {                \
      TRACE("DCHECK ", ##__VA_ARGS__); \
      ::DebugBreak();                  \
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

inline void DebugBreak() {
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

// --------------------------------------------------------------------------
//
// Mutex.
//
// --------------------------------------------------------------------------

class Futex {
 public:
#if defined(PLATFORM_WINDOWS)
  Futex() { InitializeCriticalSection(&handle_); }
  ~Futex() { DeleteCriticalSection(&handle_); }
  void Lock() { EnterCriticalSection(&handle_); }
  void Unlock() { LeaveCriticalSection(&handle_); }
#else
  Futex() { pthread_mutex_init(&handle_, NULL); }
  ~Futex() { pthread_mutex_destroy(&handle_); }
  void Lock() { pthread_mutex_lock(&handle_); }
  void Unlock() { pthread_mutex_unlock(&handle_); }
#endif

 private:
  Futex(const Futex&);             // no copy constructor
  Futex& operator=(const Futex&);  // no assignment operator

#if defined(PLATFORM_WINDOWS)
  CRITICAL_SECTION handle_;
#else
  pthread_mutex_t handle_;
#endif
};

class ScopedFutex {
 public:
  explicit ScopedFutex(Futex* futex) : futex_(futex) { futex_->Lock(); }
  ~ScopedFutex() { futex_->Unlock(); }

 private:
  ScopedFutex();                               // no default constructor
  ScopedFutex(const ScopedFutex&);             // no copy constructor
  ScopedFutex& operator=(const ScopedFutex&);  // no assignment operator

  Futex* futex_;
};

// --------------------------------------------------------------------------
//
// Semaphore.
//
// --------------------------------------------------------------------------

#if PLATFORM_OSX

#error "TODO: don't think sem_* works on OSX, need mutex+cond implementation."

#elif PLATFORM_POSIX

class Semaphore {
 public:
  Semaphore() {
    int32_t result = sem_init(&handle_, 0, 0);
    CHECK(0 == result, "sem_init failed. errno %d", errno);
    UNUSED(result);
  }

  ~Semaphore() {
    int32_t result = sem_destroy(&handle_);
    CHECK(0 == result, "sem_destroy failed. errno %d", errno);
    UNUSED(result);
  }

  void Post(uint32_t count = 1) {
    int32_t result;
    for (uint32_t ii = 0; ii < count; ++ii) {
      result = sem_post(&handle_);
      CHECK(0 == result, "sem_post failed. errno %d", errno);
    }
    UNUSED(result);
  }

  bool Wait(int32_t _msecs = -1) {
    if (0 > _msecs) {
      int32_t result;
      do {
        result = sem_wait(&handle_);
        // Keep waiting when interrupted by a signal handler...
      } while (-1 == result && EINTR == errno);
      CHECK(0 == result, "sem_wait failed. errno %d", errno);
      return 0 == result;
    }

    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += _msecs / 1000;
    ts.tv_nsec += (_msecs % 1000) * 1000;
    return 0 == sem_timedwait(&handle_, &ts);
  }

 private:
  Semaphore(const Semaphore&);             // no copy constructor
  Semaphore& operator=(const Semaphore&);  // no assignment operator

  sem_t handle_;
};

#elif PLATFORM_WINDOWS

class Semaphore {
 public:
  Semaphore() {
    handle_ = CreateSemaphore(NULL, 0, LONG_MAX, NULL);
    CHECK(NULL != handle_, "Failed to create Semaphore!");
  }

  ~Semaphore() { CloseHandle(handle_); }

  void Post(uint32_t count = 1) const {
    ReleaseSemaphore(handle_, count, NULL);
  }

  bool Wait(int32_t _msecs = -1) const {
    DWORD milliseconds = (0 > _msecs) ? INFINITE : _msecs;
    return WAIT_OBJECT_0 == WaitForSingleObject(handle_, milliseconds);
  }

 private:
  Semaphore(const Semaphore&);             // no copy constructor
  Semaphore& operator=(const Semaphore&);  // no assignment operator

  HANDLE handle_;
};

#endif

// --------------------------------------------------------------------------
//
// Thread.
//
// --------------------------------------------------------------------------

typedef int32_t (*ThreadFn)(void* user_data);

class Thread {
 public:
  Thread()
#if PLATFORM_WINDOWS
      : handle_(INVALID_HANDLE_VALUE)
#elif PLATFORM_POSIX
      : handle_(0)
#endif
        ,
        thread_func_(NULL),
        user_data_(NULL),
        stack_size_(0),
        exit_code_(0 /*EXIT_SUCCESS*/),
        running_(false) {
  }

  virtual ~Thread() {
    if (running_) {
      Shutdown();
    }
  }

  void Init(ThreadFn thread_func,
            void* user_data = NULL,
            uint32_t stack_size = 0) {
    CHECK(!running_, "Already running!");

    thread_func_ = thread_func;
    user_data_ = user_data;
    stack_size_ = stack_size;
    running_ = true;

#if PLATFORM_WINDOWS
    handle_ = CreateThread(NULL, stack_size_, ThreadFunc, this, 0, NULL);
#elif PLATFORM_POSIX
    int result;
    UNUSED(result);

    pthread_attr_t attr;
    result = pthread_attr_init(&attr);
    CHECK(0 == result, "pthread_attr_init failed! %d", result);

    if (0 != stack_size_) {
      result = pthread_attr_setstacksize(&attr, stack_size_);
      CHECK(0 == result, "pthread_attr_setstacksize failed! %d", result);
    }

    result = pthread_create(&handle_, &attr, &ThreadFunc, this);
    CHECK(0 == result, "pthread_attr_setschedparam failed! %d", result);
#endif

    sem_.Wait();
  }

  void Shutdown() {
    CHECK(running_, "Not running!");
#if PLATFORM_WINDOWS
    WaitForSingleObject(handle_, INFINITE);
    GetExitCodeThread(handle_, reinterpret_cast<DWORD*>(&exit_code_));
    CloseHandle(handle_);
    handle_ = INVALID_HANDLE_VALUE;
#elif PLATFORM_POSIX
    union {
      void* ptr;
      int32_t i;
    } cast;
    pthread_join(handle_, &cast.ptr);
    exit_code_ = cast.i;
    handle_ = 0;
#endif
    running_ = false;
  }

  bool IsRunning() const { return running_; }

 private:
  int32_t Entry() {
    sem_.Post();
    return thread_func_(user_data_);
  }

#if PLATFORM_WINDOWS
  static DWORD WINAPI ThreadFunc(LPVOID arg) {
    Thread* thread = static_cast<Thread*>(arg);
    int32_t result = thread->Entry();
    return result;
  }
#else
  static void* ThreadFunc(void* arg) {
    Thread* thread = static_cast<Thread*>(arg);
    union {
      void* ptr;
      int32_t i;
    } cast;
    cast.i = thread->Entry();
    return cast.ptr;
  }
#endif

#if PLATFORM_WINDOWS
  HANDLE handle_;
#elif PLATFORM_POSIX
  pthread_t handle_;
#endif

  ThreadFn thread_func_;
  void* user_data_;
  Semaphore sem_;
  uint32_t stack_size_;
  int32_t exit_code_;
  bool running_;

  DISALLOW_COPY_AND_ASSIGN(Thread);
};

// --------------------------------------------------------------------------
//
// Single producer, single consumer queue.
//
// --------------------------------------------------------------------------

// http://drdobbs.com/article/print?articleId=210604448&siteSectionName=
template <typename Ty>
class SpScQueue {
 public:
  SpScQueue() : first_(new Node(NULL)), divider_(first_), last_(first_) {}

  ~SpScQueue() {
    while (NULL != first_) {
      Node* node = first_;
      first_ = node->next_;
      delete node;
    }
  }

  // Producer only.
  void push(Ty* ptr) {
    last_->next_ = new Node(reinterpret_cast<void*>(ptr));
    AtomicExchangePtr(reinterpret_cast<void**>(&last_), last_->next_);
    while (first_ != divider_) {
      Node* node = first_;
      first_ = first_->next_;
      delete node;
    }
  }

  // Consumer only.
  Ty* Peek() {
    if (divider_ != last_) {
      Ty* ptr = reinterpret_cast<Ty*>(divider_->next_->ptr_);
      return ptr;
    }
    return NULL;
  }

  // Consumer only.
  Ty* Pop() {
    if (divider_ != last_) {
      Ty* ptr = reinterpret_cast<Ty*>(divider_->next_->ptr_);
      AtomicExchangePtr(reinterpret_cast<void**>(&divider_), divider_->next_);
      return ptr;
    }
    return NULL;
  }

 private:
  struct Node {
    explicit Node(void* ptr) : ptr_(ptr), next_(NULL) {}

    void* ptr_;
    Node* next_;
  };

  Node* first_;
  Node* divider_;
  Node* last_;

  DISALLOW_COPY_AND_ASSIGN(SpScQueue);
};

template <typename Ty>
class SpScBlockingQueue {
 public:
  SpScBlockingQueue() {}
  ~SpScBlockingQueue() {}

  // Producer only.
  void Push(Ty* ptr) {
    queue_.push(reinterpret_cast<void*>(ptr));
    count_.post();
  }

  // Consumer only.
  Ty* Peek() { return reinterpret_cast<Ty*>(queue_.peek()); }

  // Consumer only.
  Ty* Pop(int32_t _msecs = -1) {
    if (count_.wait(_msecs)) {
      return reinterpret_cast<Ty*>(queue_.pop());
    }
    return NULL;
  }

 private:
  Semaphore count_;
  SpScQueue<void> queue_;

  DISALLOW_COPY_AND_ASSIGN(SpScBlockingQueue);
};

#endif  // CORE_H_
