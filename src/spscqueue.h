// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SPSCQUEUE_H_
#define SPSCQUEUE_H_

#include "threading.h"

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

#endif  // SPSCQUEUE_H_
