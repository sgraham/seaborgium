// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GFX_H_
#define GFX_H_

#include <inttypes.h>

#include <memory>
#include <vector>

#include "geometric_types.h"
#include "string_piece.h"

class Widget;

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

enum class Icon : int {
  kDockLeft,
  kDockRight,
  kDockTop,
  kDockBottom,
  kTreeCollapsed,
  kTreeExpanded,
  kIndicatorPC,
  kIndicatorBreakpoint,

  Count,
};

Color Lerp(const Color& x, const Color& y, float frac);

struct TextMeasurements {
  float width;
  float height;
  float line_height;
  void* data_;

  TextMeasurements(float width, float height, float line_height)
      : width(width),
        height(height),
        line_height(line_height),
        data_(nullptr) {}
  TextMeasurements(const TextMeasurements& rhs);
  void operator=(const TextMeasurements& rhs) = delete;
  ~TextMeasurements();

  void GetCaretPosition(int index, bool trailing, float* x, float* y) const;
};

struct RangeAndColor {
  RangeAndColor() {}
  RangeAndColor(int start, int end, const Color& color)
      : start(start), end(end), color(color) {}

  int start;
  int end;
  Color color;
};

void GfxText(Font font,
             const Color& color,
             float x,
             float y,
             StringPiece string);
void GfxText(Font font,
             const Color& color,
             const Rect& rect,
             const char* string);
void GfxColoredText(Font font,
                    const Color& default_color,
                    float x,
                    float y,
                    StringPiece str,
                    const std::vector<RangeAndColor> colors);
void GfxDrawIcon(Icon icon, const Rect& rect, float alpha);
void GfxIconSize(Icon icon, float* width, float* height);


TextMeasurements GfxMeasureText(Font font, StringPiece str);

void GfxDrawFps();

float GetDpiScale();

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
                    StringPiece str,
                    const Color& color,
                    float x_padding = 0.f);

struct ScopedRenderOffset {
  ScopedRenderOffset(const Rect& rect, bool scissor);
  ScopedRenderOffset(float dx, float dy);

  ~ScopedRenderOffset();

  class Data;
  std::unique_ptr<Data> data_;
  bool scissor_;
};

void DrawWindow(const char* title,
                bool active,
                float x,
                float y,
                float w,
                float h);

#endif  // GFX_H_
