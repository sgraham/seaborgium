// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_DRAWING_COMMON_H_
#define UI_DRAWING_COMMON_H_

#include "ui/dockable.h"
#include "core/gfx.h"
#include "nanovg.h"

void UiDrawWindow(const char* title,
                  bool active,
                  int xi,
                  int yi,
                  int wi,
                  int hi);

class ScopedRenderOffset {
 public:
  ScopedRenderOffset(Dockable* parent, Dockable* child, bool scissor) {
    nvgSave(core::VG);
    Rect relative = child->GetScreenRect().RelativeTo(parent->GetScreenRect());
    nvgTranslate(core::VG,
                 static_cast<float>(relative.x),
                 static_cast<float>(relative.y));
    if (scissor) {
      nvgScissor(core::VG,
                 0,
                 0,
                 static_cast<float>(relative.w),
                 static_cast<float>(relative.h));
    }
  }

  ScopedRenderOffset(int dx, int dy) {
    nvgSave(core::VG);
    nvgTranslate(core::VG, static_cast<float>(dx), static_cast<float>(dy));
  }

  ~ScopedRenderOffset() {
    nvgRestore(core::VG);
  }
};

#endif  // UI_DRAWING_COMMON_H_
