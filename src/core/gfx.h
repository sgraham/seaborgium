// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_GFX_H_
#define CORE_GFX_H_

#include <inttypes.h>

#include <memory>

#include "core/geometric_types.h"

class Widget;

namespace core {

void GfxInit();
void GfxResize(uint32_t width, uint32_t height);
void GfxFrame();
void GfxShutdown();

// Perhaps a bit anemic.
enum class Font {
  kMono,
  kUI,
  kTitle,
};

struct Color {
  float r;
  float g;
  float b;
  float a;

  Color() {}
  Color(float r, float g, float b) : Color(r, g, b, 1.f) {}
  Color::Color(uint32_t rgb, float a)
      : Color(((rgb & 0xff0000) >> 16) / 255.f,
              ((rgb & 0xff00) >> 8) / 255.f,
              (rgb & 0xff) / 255.f,
              a) {}
  Color::Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
  bool operator==(const Color& rhs) const {
    return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a;
  }
};

float GfxText(Font font, const Color& color, float x, float y, const char* string);
float GfxTextf(Font font, const Color& color, float x, float y, const char* format, ...);

void GfxDrawFps();

float GetDpiScale();

class ColorInternalData;

// Drawing helpers.
void DrawSolidRect(const Rect& rect, const Color& color);
void DrawSolidRoundedRect(const Rect& rect, const Color& color, float radius);
void DrawOutlineRoundedRect(const Rect& rect,
                            const Color& color,
                            float radius,
                            float width);
void DrawVerticalLine(const Color& color, float x, float y0, float y1);
void DrawHorizontalLine(const Color& color, float x0, float x1, float y);
void DrawTextInRect(Font font,
                    const Rect& rect,
                    const char* text,
                    const core::Color& color,
                    float x_padding = 0.f);

struct ScopedRenderOffset {
  ScopedRenderOffset(const Rect& rect, bool scissor);
  ScopedRenderOffset(float dx, float dy);

  ~ScopedRenderOffset();

  class Data;
  std::unique_ptr<Data> data_;
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
                float x,
                float y,
                float w,
                float h);

}  // namespace core

#endif  // CORE_GFX_H_
