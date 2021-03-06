// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STRING_PIECE_H_
#define STRING_PIECE_H_

#include <string.h>

#include <string>

class StringPiece {
 public:
  StringPiece() : str_(nullptr), len_(0) {}

  StringPiece(const std::string& str)  // NOLINT(runtime/explicit)
      : str_(str.data()),
        len_(str.size()) {}
  StringPiece(const char* str)  // NOLINT(runtime/explicit)
      : str_(str),
        len_(strlen(str)) {}
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

#endif  // STRING_PIECE_H_
