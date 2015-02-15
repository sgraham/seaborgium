// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_STRING_PIECE_H_
#define CORE_STRING_PIECE_H_

#include <string.h>

#include <string>

class StringPiece {
 public:
  StringPiece() : str_(nullptr), len_(0) {}

  // Intentionally not explicit.
  StringPiece(const std::string& str) : str_(str.data()), len_(str.size()) {}
  StringPiece(const char* str) : str_(str), len_(strlen(str)) {}
  StringPiece(const char* str, size_t len) : str_(str), len_(len) {}

  bool operator==(const StringPiece& other) const {
    return len_ == other.len_ && memcmp(str_, other.str_, len_) == 0;
  }
  bool operator!=(const StringPiece& other) const { return !(*this == other); }

  std::string AsString() const {
    return len_ ? std::string(str_, len_) : std::string();
  }

  const char* data() const { return str_; }
  size_t size() const { return len_; }

 private:
  const char* str_;
  size_t len_;
};

#endif  // CORE_STRING_PIECE_H_
