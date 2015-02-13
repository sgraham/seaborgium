// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/gfx.h"

#include <windows.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>

#include "core/entry.h"

#pragma comment(lib, "d2d1.lib")

namespace core {

static HWND g_hwnd;
static uint32_t g_width;
static uint32_t g_height;
static float g_dpi_scale = 1.0;

static ID2D1Factory* g_direct2d_factory;
static ID2D1HwndRenderTarget* g_render_target;
static ID2D1SolidColorBrush* g_light_slate_gray_brush;
static ID2D1SolidColorBrush* g_cornflower_blue_brush;

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
}

void CreateDeviceResources() {
  RECT rc;
  GetClientRect(g_hwnd, &rc);

  D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

  CORE_CHECK(SUCCEEDED(g_direct2d_factory->CreateHwndRenderTarget(
                 D2D1::RenderTargetProperties(),
                 D2D1::HwndRenderTargetProperties(g_hwnd, size),
                 &g_render_target)),
             "CreateHwndRenderTarget");

  CORE_CHECK(SUCCEEDED(g_render_target->CreateSolidColorBrush(
                 D2D1::ColorF(D2D1::ColorF::LightSlateGray),
                 &g_light_slate_gray_brush)),
             "CreateSolidColorBrush");

  CORE_CHECK(SUCCEEDED(g_render_target->CreateSolidColorBrush(
                 D2D1::ColorF(D2D1::ColorF::CornflowerBlue),
                 &g_cornflower_blue_brush)),
             "CreateSolidColorBrush");
}

void DiscardDeviceResources() {
  SafeRelease(&g_render_target);
  SafeRelease(&g_light_slate_gray_brush);
  SafeRelease(&g_cornflower_blue_brush);
}

void GfxInit() {
  CreateDeviceResources();

  // We use GfxFrame as "swap", so clear and get ready here.
  g_render_target->BeginDraw();
  g_render_target->SetTransform(D2D1::Matrix3x2F::Identity());
  g_render_target->Clear(D2D1::ColorF(D2D1::ColorF::Blue));
}

void GfxResize(uint32_t width, uint32_t height) {
  g_width = width;
  g_height = height;
  if (g_render_target)
    g_render_target->Resize(D2D1::SizeU(width, height));
}

void GfxFrame() {
  HRESULT hr = g_render_target->EndDraw();
  if (hr == D2DERR_RECREATE_TARGET) {
    DiscardDeviceResources();
  }



  g_render_target->BeginDraw();
  g_render_target->SetTransform(D2D1::Matrix3x2F::Identity());
  g_render_target->Clear(D2D1::ColorF(D2D1::ColorF::White));
  D2D1_SIZE_F size = g_render_target->GetSize();

  // XXX Debug stuff, grid background.
  int width = static_cast<int>(size.width);
  int height = static_cast<int>(size.height);

  for (int x = 0; x < width; x += 10) {
    g_render_target->DrawLine(D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
                              D2D1::Point2F(static_cast<FLOAT>(x), size.height),
                              g_light_slate_gray_brush,
                              0.5f);
  }

  for (int y = 0; y < height; y += 10) {
    g_render_target->DrawLine(D2D1::Point2F(0.0f, static_cast<FLOAT>(y)),
                              D2D1::Point2F(size.width, static_cast<FLOAT>(y)),
                              g_light_slate_gray_brush,
                              0.5f);
  }

  // Draw two rectangles.
  D2D1_RECT_F rectangle1 = D2D1::RectF(size.width / 2 - 50.0f,
                                       size.height / 2 - 50.0f,
                                       size.width / 2 + 50.0f,
                                       size.height / 2 + 50.0f);

  D2D1_RECT_F rectangle2 = D2D1::RectF(size.width / 2 - 100.0f,
                                       size.height / 2 - 100.0f,
                                       size.width / 2 + 100.0f,
                                       size.height / 2 + 100.0f);

  g_render_target->FillRectangle(&rectangle1, g_light_slate_gray_brush);
  g_render_target->DrawRectangle(&rectangle2, g_cornflower_blue_brush);

  CORE_CHECK(SUCCEEDED(g_render_target->EndDraw()), "EndDraw");
}

void GfxShutdown() {
  DiscardDeviceResources();
  SafeRelease(&g_direct2d_factory);
}

float GfxTextf(float x, float y, const char* format, ...) {
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

  // TODO !
  (void)x;
  (void)y;
  // return nvgText(core::VG, x, y, out, out + len);
  return 1.f; 
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

  (void)pos;
  (void)to_ms;

  // TODO !
  /*
  nvgFontSize(VG, 13.f);
  nvgFontFace(VG, "mono");
  nvgFillColor(VG, nvgRGBA(0x00, 0xa0, 0x00, 0x60));
  //GfxTextf(10, s_height - (16 * pos++), " GL_VERSION: %s", glGetString(GL_VERSION));
  //GfxTextf(10, s_height - (16 * pos++), "GL_RENDERER: %s", glGetString(GL_RENDERER));
  //GfxTextf(10, s_height - (16 * pos++), "  GL_VENDOR: %s", glGetString(GL_VENDOR));
  GfxTextf(10,
           s_height - (16 * pos++),
           // utf-8 sequences are UPWARDS ARROW and DOWNWARDS ARROW.
           "      Frame: %7.3f, % 7.3f \xe2\x86\x91, % 7.3f \xe2\x86\x93 [ms] "
           "/ % 6.2f FPS ",
           static_cast<double>(frame_time) * to_ms,
           static_cast<double>(min) * to_ms,
           static_cast<double>(max) * to_ms,
           freq / frame_time);
           */
}

float GetDpiScale() {
  return g_dpi_scale;
}

void DrawSolidRect(const Rect& rect, const Color& color) {
  (void)rect;
  (void)color;
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

void DrawTextInRect(const Rect& rect,
                    const char* text,
                    const core::Color& color,
                    float x_padding) {
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
