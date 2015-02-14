// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/gfx.h"

#include <windows.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>

#include <unordered_map>

#include "core/entry.h"

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

namespace core {

static HWND g_hwnd;
static uint32_t g_width;
static uint32_t g_height;
static float g_dpi_scale = 1.0;

static ID2D1Factory* g_direct2d_factory;
static ID2D1HwndRenderTarget* g_render_target;
static IDWriteFactory* g_dwrite_factory;
static IDWriteTextFormat* g_text_format_mono;
struct ColorHash {
  size_t operator()(const Color& c) const {
    return (static_cast<size_t>(c.a * 255.f) << 24) +
           (static_cast<size_t>(c.r * 255.f) << 16) +
           (static_cast<size_t>(c.g * 255.f) << 8) +
           static_cast<size_t>(c.b * 255.f);
  }
};
static std::unordered_map<Color, ID2D1SolidColorBrush*, ColorHash>
    g_brush_for_color;

ID2D1SolidColorBrush* SolidBrushForColor(const Color& color) {
  auto it = g_brush_for_color.find(color);
  if (it != g_brush_for_color.end())
    return it->second;
  ID2D1SolidColorBrush* brush;
  CORE_CHECK(SUCCEEDED(g_render_target->CreateSolidColorBrush(
                 D2D1::ColorF(color.r, color.g, color.b, color.a), &brush)),
             "CreateSolidColorBrush");
  g_brush_for_color[color] = brush;
  return brush;
}

void WinGfxSetHwnd(HWND hwnd) {
  g_hwnd = hwnd;
}

template <class T>
static void SafeRelease(T** ppT) {
  if (*ppT) {
    (*ppT)->Release();
    *ppT = NULL;
  }
}

void WinGfxCreateDeviceIndependentResources() {
  HRESULT hr =
      D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_direct2d_factory);
  CORE_CHECK(SUCCEEDED(hr), "D2D1CreateFactory");

  FLOAT dpi_x, dpi_y;
  g_direct2d_factory->GetDesktopDpi(&dpi_x, &dpi_y);
  CORE_CHECK(dpi_x == dpi_y, "non-uniform dpi");
  g_dpi_scale = dpi_x;

  CORE_CHECK(SUCCEEDED(DWriteCreateFactory(
                 DWRITE_FACTORY_TYPE_SHARED,
                 __uuidof(IDWriteFactory),
                 reinterpret_cast<IUnknown**>(&g_dwrite_factory))),
             "DWriteCreateFactory");

  CORE_CHECK(SUCCEEDED(g_dwrite_factory->CreateTextFormat(
                 L"Consolas",  // Font family name.
                 nullptr,
                 DWRITE_FONT_WEIGHT_REGULAR,
                 DWRITE_FONT_STYLE_NORMAL,
                 DWRITE_FONT_STRETCH_NORMAL,
                 13.0f,
                 L"en-us",
                 &g_text_format_mono)),
             "CreateTextFormat");
}

void CreateDeviceResources() {
  RECT rc;
  GetClientRect(g_hwnd, &rc);

  D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

  CORE_CHECK(SUCCEEDED(g_direct2d_factory->CreateHwndRenderTarget(
                 D2D1::RenderTargetProperties(),
                 D2D1::HwndRenderTargetProperties(
                     g_hwnd, size, D2D1_PRESENT_OPTIONS_NONE),
                 &g_render_target)),
             "CreateHwndRenderTarget");
}

void DiscardDeviceResources() {
  SafeRelease(&g_render_target);

  for (auto& brush : g_brush_for_color)
    SafeRelease(&brush.second);
  g_brush_for_color.clear();
}

void GfxInit() {
  CreateDeviceResources();

  // We use GfxFrame as "swap", so clear and get ready here.
  g_render_target->BeginDraw();
  g_render_target->SetTransform(D2D1::Matrix3x2F::Identity());
  g_render_target->Clear(D2D1::ColorF(D2D1::ColorF::White));
}

void GfxResize(uint32_t width, uint32_t height) {
  g_width = width;
  g_height = height;
  printf("%d, %d\n", width, height);
  if (g_render_target) {
    g_render_target->Resize(D2D1::SizeU(width, height));
    DiscardDeviceResources();
    CreateDeviceResources();
  }
}

void GfxFrame() {
  HRESULT hr = g_render_target->EndDraw();
  if (hr == D2DERR_RECREATE_TARGET) {
    DiscardDeviceResources();
  }

  if (!g_render_target)
    CreateDeviceResources();

  g_render_target->BeginDraw();
  g_render_target->SetTransform(D2D1::Matrix3x2F::Identity());
  g_render_target->Clear(D2D1::ColorF(D2D1::ColorF::White));
}

void GfxShutdown() {
  DiscardDeviceResources();
  SafeRelease(&g_direct2d_factory);
  SafeRelease(&g_dwrite_factory);
  SafeRelease(&g_text_format_mono);
}

float GfxTextf(Font font,
               const Color& color,
               float x,
               float y,
               const char* format,
               ...) {
  va_list arg_list;
  va_start(arg_list, format);

  char temp[1024];
  char* out = temp;
  int32_t len = core::vsnprintf(out, sizeof(temp), format, arg_list);
  if ((int32_t)sizeof(temp) < len) {
    out = reinterpret_cast<char*>(_alloca(len + 1));
    len = core::vsnprintf(out, len, format, arg_list);
  }
  out[len] = '\0';
  va_end(arg_list);

  int wide_len = MultiByteToWideChar(CP_UTF8, 0, out, -1, NULL, 0);
  wchar_t* wide =
      reinterpret_cast<wchar_t*>(_alloca(sizeof(wchar_t) * wide_len));
  if (MultiByteToWideChar(CP_UTF8, 0, out, -1, wide, wide_len) != wide_len)
    return 0.f;

  D2D1_RECT_F layout_rect = D2D1::RectF(
      x, y, static_cast<float>(g_width), static_cast<float>(g_height));
  (void)font;
  g_render_target->DrawTextA(wide,
                             wide_len,
                             g_text_format_mono,
                             layout_rect,
                             SolidBrushForColor(color));
  return 0.f;
}

void GfxDrawFps() {
  int64_t now = core::GetHPCounter();
  static int64_t last = now;
  int64_t frame_time = now - last;
  last = now;
  static int64_t min = frame_time;
  static int64_t max = frame_time;
  min = min > frame_time ? frame_time : min;
  max = max < frame_time ? frame_time : max;

  double freq = static_cast<double>(core::GetHPFrequency());
  double to_ms = 1000.0 / freq;
  float pos = 1;

    // TODO !
    /*
    nvgFontSize(VG, 13.f);
    nvgFontFace(VG, "mono");
    nvgFillColor(VG, nvgRGBA(0x00, 0xa0, 0x00, 0x60));
    //GfxTextf(10, s_height - (16 * pos++), " GL_VERSION: %s", glGetString(GL_VERSION));
    //GfxTextf(10, s_height - (16 * pos++), "GL_RENDERER: %s", glGetString(GL_RENDERER));
    //GfxTextf(10, s_height - (16 * pos++), "  GL_VENDOR: %s", glGetString(GL_VENDOR));
    */
  GfxTextf(Font::kMono,
           Color(0, 0, 0),
           10,
           16 * pos++,
           // utf-8 sequences are UPWARDS ARROW and DOWNWARDS ARROW.
           "Frame: %7.3f, % 7.3f \xe2\x86\x91, % 7.3f \xe2\x86\x93 [ms] / % "
           "6.2f FPS ",
           static_cast<double>(frame_time) * to_ms,
           static_cast<double>(min) * to_ms,
           static_cast<double>(max) * to_ms,
           freq / frame_time);
}

float GetDpiScale() {
  return g_dpi_scale;
}

void DrawSolidRect(const Rect& rect, const Color& color) {
  D2D1_RECT_F d2d1rect = { rect.x, rect.y, rect.x + rect.w, rect.y + rect.h };
  g_render_target->FillRectangle(d2d1rect, SolidBrushForColor(color));
}

void DrawSolidRoundedRect(const Rect& rect, const Color& color, float radius) {
  (void)rect;
  (void)color;
  (void)radius;
}

void DrawOutlineRoundedRect(const Rect& rect,
                            const Color& color,
                            float radius,
                            float width) {
  (void)rect;
  (void)color;
  (void)radius;
  (void)width;
}

void DrawVerticalLine(const Color& color, float x, float y0, float y1) {
  (void)color;
  (void)x;
  (void)y0;
  (void)y1;
}

void DrawHorizontalLine(const Color& color, float x0, float x1, float y) {
  (void)color;
  (void)x0;
  (void)x1;
  (void)y;
}

void DrawTextInRect(Font font,
                    const Rect& rect,
                    const char* text,
                    const core::Color& color,
                    float x_padding) {
  (void)font;
  (void)rect;
  (void)text;
  (void)color;
  (void)x_padding;
}

void DrawWindow(const char* title,
                bool active,
                double x,
                double y,
                double w,
                double h) {
  (void)title;
  (void)active;
  (void)x;
  (void)y;
  (void)w;
  (void)h;
}

}  // namespace core
