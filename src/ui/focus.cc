// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/focus.h"

namespace {

Dockable* g_focused;

}  // namespace

// TODO(focus): Probably some sort of OnFocus/OnBlur?

Dockable* GetFocusedContents() {
  return g_focused;
}

void SetFocusedContents(Dockable* contents) {
  g_focused = contents;
}
