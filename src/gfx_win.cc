// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gfx.h"

#include <windows.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <stdio.h>
#include <wincodec.h>

#include <algorithm>
#include <limits>
#include <unordered_map>

#include "entry.h"
#include "resource.h"
#include "skin.h"

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "windowscodecs.lib")

extern "C" IMAGE_DOS_HEADER __ImageBase;

static HWND g_hwnd;
static uint32_t g_width;
static uint32_t g_height;
static float g_dpi_scale = 1.0;

static IWICImagingFactory* g_wic_factory;
static ID2D1Factory* g_direct2d_factory;
static ID2D1HwndRenderTarget* g_render_target;
static IDWriteFactory* g_dwrite_factory;
static IDWriteTextFormat* g_text_format_mono;
static IDWriteTextFormat* g_text_format_ui;
static IDWriteTextFormat* g_text_format_title;
static ID2D1LinearGradientBrush* g_title_bar_active_gradient_brush;
static ID2D1LinearGradientBrush* g_title_bar_inactive_gradient_brush;
int operator+(Icon val) {
  return static_cast<int>(val);
}
static ID2D1Bitmap* g_icons[static_cast<int>(Icon::Count)];
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
  CHECK(SUCCEEDED(g_render_target->CreateSolidColorBrush(
      D2D1::ColorF(color.r, color.g, color.b, color.a), &brush)));
  g_brush_for_color[color] = brush;
  return brush;
}

D2D1_COLOR_F ColorToD2DColorF(const Color& color) {
  return D2D1::ColorF(color.r, color.g, color.b, color.a);
}

Color Lerp(const Color& x, const Color& y, float frac) {
  frac = std::max(0.f, std::min(frac, 1.f));
  float one_minus_frac = 1.f - frac;
  return Color(x.r * one_minus_frac + y.r * frac,
               x.g * one_minus_frac + y.g * frac,
               x.b * one_minus_frac + y.b * frac,
               x.a * one_minus_frac + y.a * frac);
}

void WinGfxSetHwnd(HWND hwnd) {
  g_hwnd = hwnd;
}

template <class T>
static void SafeRelease(T** ppT) {
  if (*ppT) {
    (*ppT)->Release();
    *ppT = nullptr;
  }
}

ID2D1Bitmap* LoadBitmapFromResource(int res) {
  HINSTANCE self_hinst = reinterpret_cast<HINSTANCE>(&__ImageBase);
  HRSRC image_res_handle =
      FindResource(self_hinst, MAKEINTRESOURCE(res), "IMAGE");
  CHECK(image_res_handle, "FindResource");
  HGLOBAL image_res_data_handle = LoadResource(self_hinst, image_res_handle);
  CHECK(image_res_data_handle, "LoadResource");
  void* image_file = LockResource(image_res_data_handle);
  CHECK(image_file, "LockResource");
  DWORD image_file_size = SizeofResource(self_hinst, image_res_handle);
  CHECK(image_file_size, "SizeofResource");

  IWICStream* stream;
  CHECK(SUCCEEDED(g_wic_factory->CreateStream(&stream)));
  CHECK(SUCCEEDED(stream->InitializeFromMemory(
      reinterpret_cast<BYTE*>(image_file), image_file_size)));

  IWICBitmapDecoder* decoder;
  CHECK(SUCCEEDED(g_wic_factory->CreateDecoderFromStream(
      stream, nullptr, WICDecodeMetadataCacheOnLoad, &decoder)));

  IWICBitmapFrameDecode* source;
  CHECK(SUCCEEDED(decoder->GetFrame(0, &source)));

  IWICFormatConverter* converter;
  CHECK(SUCCEEDED(g_wic_factory->CreateFormatConverter(&converter)));
  CHECK(SUCCEEDED(converter->Initialize(source,
                                        GUID_WICPixelFormat32bppPBGRA,
                                        WICBitmapDitherTypeNone,
                                        nullptr,
                                        0.f,
                                        WICBitmapPaletteTypeMedianCut)));

  ID2D1Bitmap* bitmap;
  CHECK(SUCCEEDED(
      g_render_target->CreateBitmapFromWicBitmap(converter, nullptr, &bitmap)));

  SafeRelease(&decoder);
  SafeRelease(&source);
  SafeRelease(&stream);
  SafeRelease(&converter);

  return bitmap;
}

void WinGfxCreateDeviceIndependentResources() {
  HRESULT hr =
      D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_direct2d_factory);
  CHECK(SUCCEEDED(hr), "D2D1CreateFactory");

  FLOAT dpi_x, dpi_y;
  g_direct2d_factory->GetDesktopDpi(&dpi_x, &dpi_y);
  CHECK(dpi_x == dpi_y, "non-uniform dpi");
  g_dpi_scale = dpi_x / 96.f;

  CHECK(SUCCEEDED(
      DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
                          __uuidof(IDWriteFactory),
                          reinterpret_cast<IUnknown**>(&g_dwrite_factory))));

  CHECK(SUCCEEDED(
      g_dwrite_factory->CreateTextFormat(L"Consolas",  // Font family name.
                                         nullptr,
                                         DWRITE_FONT_WEIGHT_REGULAR,
                                         DWRITE_FONT_STYLE_NORMAL,
                                         DWRITE_FONT_STRETCH_NORMAL,
                                         12.0f,
                                         L"en-us",
                                         &g_text_format_mono)));

  CHECK(SUCCEEDED(
      g_dwrite_factory->CreateTextFormat(L"Segoe UI",  // Font family name.
                                         nullptr,
                                         DWRITE_FONT_WEIGHT_REGULAR,
                                         DWRITE_FONT_STYLE_NORMAL,
                                         DWRITE_FONT_STRETCH_NORMAL,
                                         13.0f,
                                         L"en-us",
                                         &g_text_format_ui)));

  CHECK(SUCCEEDED(
      g_dwrite_factory->CreateTextFormat(L"Segoe UI",  // Font family name.
                                         nullptr,
                                         DWRITE_FONT_WEIGHT_BOLD,
                                         DWRITE_FONT_STYLE_NORMAL,
                                         DWRITE_FONT_STRETCH_NORMAL,
                                         15.0f,
                                         L"en-us",
                                         &g_text_format_title)));
  g_text_format_title->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
  g_text_format_title->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
}

void CreateDeviceResources() {
  RECT rc;
  GetClientRect(g_hwnd, &rc);

  D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

  CHECK(SUCCEEDED(CoInitialize(nullptr)));
  CHECK(SUCCEEDED(CoCreateInstance(CLSID_WICImagingFactory,
                                   nullptr,
                                   CLSCTX_INPROC_SERVER,
                                   IID_IWICImagingFactory,
                                   reinterpret_cast<void**>(&g_wic_factory))));

  if (FAILED(g_direct2d_factory->CreateHwndRenderTarget(
          D2D1::RenderTargetProperties(),
          D2D1::HwndRenderTargetProperties(
              g_hwnd, size, D2D1_PRESENT_OPTIONS_NONE),
          &g_render_target)))
    return;

  const Skin& sk = Skin::current();
  const ColorScheme& cs = sk.GetColorScheme();

  ID2D1GradientStopCollection* gradient_stop_collection;
  D2D1_GRADIENT_STOP gradient_stops[2];
  gradient_stops[0].color = ColorToD2DColorF(cs.title_bar_active_inner());
  gradient_stops[0].position = 0.0f;
  gradient_stops[1].color = ColorToD2DColorF(cs.title_bar_active_outer());
  gradient_stops[1].position = 1.f;
  CHECK(SUCCEEDED(g_render_target->CreateGradientStopCollection(
            gradient_stops,
            2,
            D2D1_GAMMA_2_2,
            D2D1_EXTEND_MODE_CLAMP,
            &gradient_stop_collection)),
        "CreateGradientStopCollection active");
  CHECK(
      SUCCEEDED(g_render_target->CreateLinearGradientBrush(
          D2D1::LinearGradientBrushProperties(
              D2D1::Point2F(0.0, 0.0), D2D1::Point2F(0.0, sk.title_bar_size())),
          gradient_stop_collection,
          &g_title_bar_active_gradient_brush)),
      "CreateLinearGradientBrush active");
  SafeRelease(&gradient_stop_collection);

  gradient_stops[0].color = ColorToD2DColorF(cs.title_bar_inactive_inner());
  gradient_stops[1].color = ColorToD2DColorF(cs.title_bar_inactive_outer());
  CHECK(SUCCEEDED(g_render_target->CreateGradientStopCollection(
            gradient_stops,
            2,
            D2D1_GAMMA_2_2,
            D2D1_EXTEND_MODE_CLAMP,
            &gradient_stop_collection)),
        "CreateGradientStopCollection inactive");
  CHECK(
      SUCCEEDED(g_render_target->CreateLinearGradientBrush(
          D2D1::LinearGradientBrushProperties(
              D2D1::Point2F(0.0, 0.0), D2D1::Point2F(0.0, sk.title_bar_size())),
          gradient_stop_collection,
          &g_title_bar_inactive_gradient_brush)),
      "CreateLinearGradientBrush inactive");
  SafeRelease(&gradient_stop_collection);

  g_icons[+Icon::kDockLeft] = LoadBitmapFromResource(RES_DOCK_LEFT);
  g_icons[+Icon::kDockRight] = LoadBitmapFromResource(RES_DOCK_RIGHT);
  g_icons[+Icon::kDockTop] = LoadBitmapFromResource(RES_DOCK_TOP);
  g_icons[+Icon::kDockBottom] = LoadBitmapFromResource(RES_DOCK_BOTTOM);
  g_icons[+Icon::kTreeCollapsed] = LoadBitmapFromResource(RES_TREE_COLLAPSED);
  g_icons[+Icon::kTreeExpanded] = LoadBitmapFromResource(RES_TREE_EXPANDED);
  g_icons[+Icon::kIndicatorPC] = LoadBitmapFromResource(RES_INDICATOR_PC);
  g_icons[+Icon::kIndicatorBreakpoint] =
      LoadBitmapFromResource(RES_INDICATOR_BREAKPOINT);
}

void DiscardDeviceResources() {
  SafeRelease(&g_render_target);
  SafeRelease(&g_title_bar_active_gradient_brush);
  SafeRelease(&g_title_bar_inactive_gradient_brush);

  for (auto& brush : g_brush_for_color)
    SafeRelease(&brush.second);
  g_brush_for_color.clear();

  for (auto& icon : g_icons)
    SafeRelease(&icon);
}

void BeginFrame() {
  g_render_target->BeginDraw();
  g_render_target->SetTransform(D2D1::Matrix3x2F::Identity());
  g_render_target->Clear(D2D1::ColorF(D2D1::ColorF::DarkSlateGray));
}

void GfxInit() {
  CreateDeviceResources();
  BeginFrame();
}

void GfxResize(uint32_t width, uint32_t height) {
  g_width = width;
  g_height = height;
  if (g_render_target) {
    DiscardDeviceResources();
    CreateDeviceResources();
  }
}

void GfxFrame() {
  HRESULT hr = g_render_target->EndDraw();
  if (hr == D2DERR_RECREATE_TARGET)
    DiscardDeviceResources();

  if (!g_render_target)
    CreateDeviceResources();

  if (!g_render_target) {
    // If still failed, can't continue.
    return;
  }

  BeginFrame();
}

void GfxShutdown() {
  DiscardDeviceResources();
  SafeRelease(&g_direct2d_factory);
  SafeRelease(&g_dwrite_factory);
  SafeRelease(&g_text_format_mono);
  SafeRelease(&g_text_format_ui);
  SafeRelease(&g_text_format_title);
}

IDWriteTextFormat* TextFormatForFont(Font font) {
  switch (font) {
    case Font::kMono:
      return g_text_format_mono;
    case Font::kUI:
      return g_text_format_ui;
    case Font::kTitle:
      return g_text_format_title;
    default:
      CHECK(false, "unexpected font");
      return nullptr;
  }
}

std::wstring UTF8ToUTF16(StringPiece utf8) {
  int wide_len =
      MultiByteToWideChar(CP_UTF8, 0, utf8.data(), utf8.size(), nullptr, 0);
  wchar_t* wide =
      reinterpret_cast<wchar_t*>(_alloca(sizeof(wchar_t) * wide_len));
  if (MultiByteToWideChar(
          CP_UTF8, 0, utf8.data(), utf8.size(), wide, wide_len) != wide_len) {
    return std::wstring();
  }
  return std::wstring(wide, wide_len);
}

void GfxText(Font font,
             const Color& color,
             float x,
             float y,
             StringPiece string) {
  std::wstring wide = UTF8ToUTF16(string);
  D2D1_RECT_F layout_rect = D2D1::RectF(
      x, y, static_cast<float>(g_width), static_cast<float>(g_height));
  g_render_target->DrawTextA(&wide[0],
                             wide.size(),
                             TextFormatForFont(font),
                             layout_rect,
                             SolidBrushForColor(color));
}

void GfxText(Font font,
             const Color& color,
             const Rect& rect,
             const char* string) {
  std::wstring wide = UTF8ToUTF16(string);
  D2D1_RECT_F layout_rect =
      D2D1::RectF(rect.x, rect.y, rect.x + rect.w, rect.x + rect.h);
  g_render_target->DrawTextA(&wide[0],
                             wide.size(),
                             TextFormatForFont(font),
                             layout_rect,
                             SolidBrushForColor(color));
}

void GfxColoredText(Font font,
                    const Color& default_color,
                    float x,
                    float y,
                    StringPiece str,
                    const std::vector<RangeAndColor> colors) {
  IDWriteTextLayout* layout;
  std::wstring wide = UTF8ToUTF16(str);
  CHECK(SUCCEEDED(
      g_dwrite_factory->CreateTextLayout(&wide[0],
                                         wide.size(),
                                         TextFormatForFont(font),
                                         std::numeric_limits<float>::max(),
                                         std::numeric_limits<float>::max(),
                                         &layout)));
  for (const auto& rac : colors) {
    DWRITE_TEXT_RANGE range = {rac.start, rac.end - rac.start};
    layout->SetDrawingEffect(SolidBrushForColor(rac.color), range);
  }

  g_render_target->DrawTextLayout(
      D2D1::Point2F(x, y), layout, SolidBrushForColor(default_color));

  layout->Release();
}

void GfxDrawIcon(Icon icon, const Rect& rect, float alpha) {
  g_render_target->DrawBitmap(
      g_icons[+icon],
      D2D1::RectF(rect.x, rect.y, rect.x + rect.w, rect.y + rect.h),
      alpha);
}

void GfxIconSize(Icon icon, float* width, float* height) {
  D2D1_SIZE_F size = g_icons[+icon]->GetSize();
  *width = size.width;
  *height = size.height;
}

TextMeasurements GfxMeasureText(Font font, StringPiece str) {
  IDWriteTextLayout* layout;
  std::wstring wide = UTF8ToUTF16(str);
  CHECK(SUCCEEDED(
      g_dwrite_factory->CreateTextLayout(&wide[0],
                                         wide.size(),
                                         TextFormatForFont(font),
                                         std::numeric_limits<float>::max(),
                                         std::numeric_limits<float>::max(),
                                         &layout)));
  DWRITE_TEXT_METRICS metrics;
  CHECK(SUCCEEDED(layout->GetMetrics(&metrics)));

  DWRITE_LINE_SPACING_METHOD method;
  float line_spacing, baseline;
  CHECK(SUCCEEDED(layout->GetLineSpacing(&method, &line_spacing, &baseline)));

  auto tm = TextMeasurements(
      metrics.width, metrics.height, metrics.height /* TODO */);
  tm.data_ = reinterpret_cast<void*>(layout);
  return tm;
}

TextMeasurements::TextMeasurements(const TextMeasurements& rhs) {
  width = rhs.width;
  height = rhs.height;
  line_height = rhs.line_height;
  data_ = rhs.data_;
  reinterpret_cast<IDWriteTextLayout*>(data_)->AddRef();
}

TextMeasurements::~TextMeasurements() {
  reinterpret_cast<IDWriteTextLayout*>(data_)->Release();
}

void TextMeasurements::GetCaretPosition(int index,
                                        bool trailing,
                                        float* x,
                                        float* y) const {
  auto layout = reinterpret_cast<IDWriteTextLayout*>(data_);
  DWRITE_HIT_TEST_METRICS metrics;
  CHECK(SUCCEEDED(layout->HitTestTextPosition(index, trailing, x, y, &metrics)),
        "HitTestTextPosition");
}

void GfxDrawFps() {
  int64_t now = GetHPCounter();
  static int64_t last = now;
  int64_t frame_time = now - last;
  last = now;
  static int64_t min = frame_time;
  static int64_t max = frame_time;
  min = min > frame_time ? frame_time : min;
  max = max < frame_time ? frame_time : max;

  double freq = static_cast<double>(GetHPFrequency());
  double to_ms = 1000.0 / freq;
  float pos = 1;

  char buf[256];
  snprintf(buf,
           sizeof(buf),
           // utf-8 sequences are UPWARDS ARROW and DOWNWARDS ARROW.
           "Frame: %7.3f, % 7.3f \xe2\x86\x91, % 7.3f \xe2\x86\x93 [ms] / "
           "%6.2f FPS ",
           static_cast<double>(frame_time) * to_ms,
           static_cast<double>(min) * to_ms,
           static_cast<double>(max) * to_ms,
           freq / frame_time);
  GfxText(Font::kMono, Color(0.f, 0.65f, 0.f, 0.375f), 10, 16 * pos++, buf);
}

float GetDpiScale() {
  return g_dpi_scale;
}

void DrawSolidRect(const Rect& rect, const Color& color) {
  g_render_target->FillRectangle(
      D2D1::RectF(rect.x, rect.y, rect.x + rect.w, rect.y + rect.h),
      SolidBrushForColor(color));
}

void DrawSolidRoundedRect(const Rect& rect, const Color& color, float radius) {
  g_render_target->FillRoundedRectangle(
      D2D1::RoundedRect(
          D2D1::RectF(rect.x, rect.y, rect.x + rect.w, rect.y + rect.h),
          radius,
          radius),
      SolidBrushForColor(color));
}

void DrawOutlineRoundedRect(const Rect& rect,
                            const Color& color,
                            float radius,
                            float width) {
  g_render_target->DrawRoundedRectangle(
      D2D1::RoundedRect(
          D2D1::RectF(rect.x, rect.y, rect.x + rect.w, rect.y + rect.h),
          radius,
          radius),
      SolidBrushForColor(color),
      width);
}

void DrawVerticalLine(const Color& color, float x, float y0, float y1) {
  g_render_target->DrawLine(
      D2D1::Point2F(x, y0), D2D1::Point2F(x, y1), SolidBrushForColor(color));
}

void DrawHorizontalLine(const Color& color, float x0, float x1, float y) {
  g_render_target->DrawLine(
      D2D1::Point2F(x0, y), D2D1::Point2F(x1, y), SolidBrushForColor(color));
}

void DrawTextInRect(Font font,
                    const Rect& rect,
                    StringPiece str,
                    const Color& color,
                    float x_padding) {
  ScopedRenderOffset offset(rect, true);
  GfxText(font, color, x_padding, 0.f, str);
}

void DrawWindow(const char* title,
                bool active,
                float x,
                float y,
                float w,
                float h) {
  const Skin& sk = Skin::current();
  const ColorScheme& cs = sk.GetColorScheme();
  const float kCornerRadius = 3.f;

  // Window: round top, but square content area.
  DrawSolidRoundedRect(Rect(x, y, w, h), cs.background(), kCornerRadius);
  DrawSolidRect(Rect(x, y + sk.title_bar_size(), w, h - sk.title_bar_size()),
                cs.background());

  // Drop shadow maybe.

  // Header.
  g_render_target->FillRoundedRectangle(
      D2D1::RoundedRect(D2D1::RectF(x, y, x + w, y + sk.title_bar_size()),
                        kCornerRadius - 1,
                        kCornerRadius - 1),
      active ? g_title_bar_active_gradient_brush
             : g_title_bar_inactive_gradient_brush);
  g_render_target->DrawLine(
      D2D1::Point2F(x + 0.5f, y + sk.title_bar_size() - 1),
      D2D1::Point2F(x + 0.5f + w - 1, y + sk.title_bar_size() - 1),
      SolidBrushForColor(cs.border()));

  // Title.
  const float kTextFudge = -4;
  GfxText(Font::kTitle,
          cs.title_bar_text_drop_shadow(),
          Rect(x + 1, y + 1 + kTextFudge, w, sk.title_bar_size()),
          title);
  GfxText(Font::kTitle,
          active ? cs.title_bar_text_active() : cs.title_bar_text_inactive(),
          Rect(x, y + kTextFudge, w, sk.title_bar_size()),
          title);
}

class ScopedRenderOffset::Data {
 public:
  Data() { g_render_target->GetTransform(&transform_); }
  ~Data() { g_render_target->SetTransform(transform_); }

  D2D1_MATRIX_3X2_F transform_;
};

ScopedRenderOffset::ScopedRenderOffset(const Rect& rect, bool scissor)
    : data_(new Data), scissor_(scissor) {
  g_render_target->SetTransform(data_->transform_ *
                                D2D1::Matrix3x2F::Translation(rect.x, rect.y));
  if (scissor) {
    g_render_target->PushAxisAlignedClip(D2D1::RectF(0, 0, rect.w, rect.h),
                                         D2D1_ANTIALIAS_MODE_ALIASED);
  }
}

ScopedRenderOffset::ScopedRenderOffset(float dx, float dy)
    : data_(new Data), scissor_(false) {
  g_render_target->SetTransform(data_->transform_ *
                                D2D1::Matrix3x2F::Translation(dx, dy));
}

ScopedRenderOffset::~ScopedRenderOffset() {
  if (scissor_)
    g_render_target->PopAxisAlignedClip();
}
