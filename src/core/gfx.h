// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_GFX_H_
#define CORE_GFX_H_

#include <inttypes.h>

#include "core/geometric_types.h"

class Widget;

namespace core {

void GfxInit();
void GfxResize(uint32_t width, uint32_t height);
void GfxFrame();
void GfxShutdown();

float GfxText(float x, float y, const char* string);
float GfxTextf(float x, float y, const char* format, ...);

void GfxDrawFps();

float GetDpiScale();

struct Color {
  float r;
  float g;
  float b;
  float a;

  Color() {}
  Color(float r, float g, float b) : r(r), g(g), b(b), a(1.f) {}
  Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
  Color(uint32_t rgb, float a)
      : r(((rgb & 0xff0000) >> 16) / 255.f),
        g(((rgb & 0xff00) >> 8) / 255.f),
        b((rgb & 0xff) / 255.f),
        a(a) {}
};

// Drawing helpers.
void DrawSolidRect(const Rect& rect, const Color& color);
void DrawSolidRoundedRect(const Rect& rect, const Color& color, float radius);
void DrawOutlineRoundedRect(const Rect& rect,
                            const Color& color,
                            float radius,
                            float width);
void DrawVerticalLine(const Color& color, float x, float y0, float y1);
void DrawHorizontalLine(const Color& color, float x0, float x1, float y);
void DrawTextInRect(const Rect& rect,
                    const char* text,
                    const core::Color& color,
                    float x_padding = 0.f);

struct ScopedRenderOffset {
  ScopedRenderOffset(Widget* parent, Widget* child, bool scissor) {
    (void)parent;
    (void)child;
    (void)scissor;
  }
  ScopedRenderOffset(const Rect& rect, bool scissor) {
    (void)rect;
    (void)scissor;
  }
  ScopedRenderOffset(float dx, float dy) {
    (void)dx;
    (void)dy;
  }
  /*
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
#if 0
    NVGcolor random_color =
        nvgRGBA(rand() % 255, rand() % 255, rand() % 255, 64);
    GfxSolidRect(Rect(0, 0, rect.w, rect.h), random_color);
#endif
  }

  ScopedRenderOffset(float dx, float dy) {
    nvgSave(core::VG);
    nvgTranslate(core::VG, dx, dy);
  }

  virtual ~ScopedRenderOffset() { nvgRestore(core::VG); }
  */
};

struct ScopedTextSetup {
  ScopedTextSetup(const char* name, float size) {
    (void)name;
    (void)size;
#if 0
    nvgSave(core::VG);
    nvgFontSize(core::VG, size);
    nvgFontFace(core::VG, name);
#endif
  }
  virtual ~ScopedTextSetup() {
#if 0
    nvgRestore(core::VG);
#endif
  }
};

struct ScopedMonoSetup : public ScopedTextSetup {
  ScopedMonoSetup() : ScopedTextSetup("mono", 14.f) {}
};

struct ScopedSansSetup : public ScopedTextSetup {
  ScopedSansSetup() : ScopedTextSetup("sans", 16.f) {}
};

struct ScopedIconsSetup : public ScopedTextSetup {
  ScopedIconsSetup() : ScopedTextSetup("icons", 28.f) {}
};


// TODO: Not very core.
void DrawWindow(const char* title,
                bool active,
                double x,
                double y,
                double w,
                double h);

}  // namespace core

#endif  // CORE_GFX_H_
