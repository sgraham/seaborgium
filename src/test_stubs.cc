// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "entry.h"

// These are stubs for main -> entry that are linked into test binaries so the
// main code can call these functions without adding another layer of
// interface to the main binary.

void SetWindowSize(uint32_t /*width*/, uint32_t /*height*/) {
}

void SetMouseCursor(MouseCursor::Enum /*cursor*/) {
}
