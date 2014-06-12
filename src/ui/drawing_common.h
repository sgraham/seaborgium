// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_DRAWING_COMMON_H_
#define UI_DRAWING_COMMON_H_

#include "ui/dockable.h"
#include "core/gfx.h"
#include "nanovg.h"

#include <string>

void DrawWindow(const char* title,
                bool acive,
                float x,
                float y,
                float w,
                float h);

void DrawSolidRect(const Rect& rect, const NVGcolor& color);
void DrawSolidRoundedRect(const Rect& rect,
                          const NVGcolor& color,
                          float radius);
void DrawVerticalLine(const NVGcolor& color, float x, float y0, float y1);
void DrawHorizontalLine(const NVGcolor& color, float x0, float x1, float y);
void DrawTextInRect(const Rect& rect,
                    const std::string& text,
                    const NVGcolor& color,
                    float x_padding = 0.f);

struct ScopedRenderOffset {
  ScopedRenderOffset(Dockable* parent, Dockable* child, bool scissor) {
    nvgSave(core::VG);
    Rect relative = child->GetScreenRect().RelativeTo(parent->GetScreenRect());
    nvgTranslate(core::VG, relative.x, relative.y);
    if (scissor)
      nvgScissor(core::VG, 0, 0, relative.w, relative.h);
  }

  ScopedRenderOffset(const Rect& rect, bool scissor) {
    nvgSave(core::VG);
    nvgTranslate(core::VG, rect.x, rect.y);
    if (scissor)
      nvgScissor(core::VG, 0, 0, rect.w, rect.h);
  }

  ScopedRenderOffset(float dx, float dy) {
    nvgSave(core::VG);
    nvgTranslate(core::VG, dx, dy);
  }

  virtual ~ScopedRenderOffset() { nvgRestore(core::VG); }
};

struct ScopedTextSetup {
  ScopedTextSetup(const char* name, float size) {
    nvgSave(core::VG);
    nvgFontSize(core::VG, size);
    nvgFontFace(core::VG, name);
  }
  virtual ~ScopedTextSetup() { nvgRestore(core::VG); }
};

struct ScopedMonoSetup : public ScopedTextSetup {
  ScopedMonoSetup() : ScopedTextSetup("mono", 14.f) {}
};

struct ScopedSansSetup : public ScopedTextSetup {
  ScopedSansSetup() : ScopedTextSetup("sans", 14.f) {}
};

#endif  // UI_DRAWING_COMMON_H_
