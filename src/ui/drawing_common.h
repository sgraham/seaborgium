// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_DRAWING_COMMON_H_
#define UI_DRAWING_COMMON_H_

#include "ui/dockable.h"
#include "core/gfx.h"
#include "nanovg.h"

void UiDrawWindow(const char* title,
                  bool acive,
                  float x,
                  float y,
                  float w,
                  float h);

class ScopedRenderOffset {
 public:
  ScopedRenderOffset(Dockable* parent, Dockable* child, bool scissor) {
    nvgSave(core::VG);
    Rect relative = child->GetScreenRect().RelativeTo(parent->GetScreenRect());
    nvgTranslate(core::VG, relative.x, relative.y);
    if (scissor)
      nvgScissor(core::VG, 0, 0, relative.w, relative.h);
  }

  ScopedRenderOffset(float dx, float dy) {
    nvgSave(core::VG);
    nvgTranslate(core::VG, dx, dy);
  }

  ~ScopedRenderOffset() {
    nvgRestore(core::VG);
  }
};

#endif  // UI_DRAWING_COMMON_H_
