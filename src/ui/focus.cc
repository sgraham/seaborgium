// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/focus.h"

namespace {

Widget* g_focused;

}  // namespace

// TODO(focus): Probably some sort of OnFocus/OnBlur?

Widget* GetFocusedContents() {
  return g_focused;
}

void SetFocusedContents(Widget* contents) {
  g_focused = contents;
}
