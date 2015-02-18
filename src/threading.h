// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THREADING_H_
#define THREADING_H_

#include "core.h"

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

#endif  // THREADING_H_
