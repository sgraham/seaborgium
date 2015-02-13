// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/drawing_common.h"

#include "core/gfx.h"
#include "ui/skin.h"
#if 0
void DrawWindow(const char* title,
                bool active,
                float x,
                float y,
                float w,
                float h) {
  const Skin& sk = Skin::current();
  const ColorScheme& cs = sk.GetColorScheme();
  const float corner_radius = 3.0f;

  nvgSave(core::VG);

  // Window
  nvgBeginPath(core::VG);
  nvgRoundedRect(core::VG, x, y, w, h, corner_radius);
  // Want round top, but square content area.
  nvgRect(core::VG, x, y + sk.title_bar_size(), w, h - sk.title_bar_size());
  nvgFillColor(core::VG, cs.background());
  nvgFill(core::VG);

  // Drop shadow
  NVGpaint shadow_paint = nvgBoxGradient(core::VG,
                                         x,
                                         y + 2,
                                         w,
                                         h,
                                         corner_radius * 2,
                                         10,
                                         cs.drop_shadow_inner(),
                                         cs.drop_shadow_outer());
  nvgBeginPath(core::VG);
  nvgRect(core::VG, x - 10, y - 10, w + 20, h + 30);
  nvgRoundedRect(core::VG, x, y, w, h, corner_radius);
  nvgPathWinding(core::VG, NVG_HOLE);
  nvgFillPaint(core::VG, shadow_paint);
  nvgFill(core::VG);

  // Header
  NVGpaint header_paint = nvgLinearGradient(
      core::VG,
      x,
      y,
      x,
      y + sk.title_bar_size() / 2.f,
      active ? cs.title_bar_active_inner() : cs.title_bar_inactive_inner(),
      active ? cs.title_bar_active_outer() : cs.title_bar_inactive_outer());
  nvgBeginPath(core::VG);
  nvgRoundedRect(
      core::VG, x + 1, y + 1, w - 2, sk.title_bar_size(), corner_radius - 1);
  nvgFillPaint(core::VG, header_paint);
  nvgFill(core::VG);
  nvgBeginPath(core::VG);
  nvgMoveTo(core::VG, x + 0.5f, y + sk.title_bar_size() - 1);
  nvgLineTo(core::VG, x + 0.5f + w - 1, y + sk.title_bar_size() - 1);
  nvgStrokeColor(core::VG, cs.border());
  nvgStroke(core::VG);

  nvgFontSize(core::VG, sk.title_bar_text_size());
  nvgFontFace(core::VG, "sans-bold");
  nvgTextAlign(core::VG, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);

  const float kFudgeText = -1.f;  // Because of the 1 pixel divider.
  nvgFontBlur(core::VG, 2);
  nvgFillColor(core::VG, cs.title_bar_text_drop_shadow());
  nvgText(core::VG,
          x + w / 2,
          y + sk.title_bar_size() / 2 + kFudgeText + 1,
          title,
          NULL);

  nvgFontBlur(core::VG, 0);
  nvgFillColor(
      core::VG,
      active ? cs.title_bar_text_active() : cs.title_bar_text_inactive());
  nvgText(core::VG,
          x + w / 2,
          y + sk.title_bar_size() / 2 + kFudgeText,
          title,
          NULL);

  nvgRestore(core::VG);
}

void DrawSolidRect(const Rect& rect, const NVGcolor& color) {
  nvgSave(core::VG);
  nvgBeginPath(core::VG);
  nvgRect(core::VG, rect.x, rect.y, rect.w, rect.h);
  nvgFillColor(core::VG, color);
  nvgFill(core::VG);
  nvgRestore(core::VG);
}

void DrawSolidRoundedRect(const Rect& rect,
                          const NVGcolor& color,
                          float radius) {
  nvgSave(core::VG);
  nvgBeginPath(core::VG);
  nvgRoundedRect(core::VG, rect.x, rect.y, rect.w, rect.h, radius);
  nvgFillColor(core::VG, color);
  nvgFill(core::VG);
  nvgRestore(core::VG);
}

void DrawOutlineRoundedRect(const Rect& rect,
                            const NVGcolor& color,
                            float radius,
                            float width) {
  nvgSave(core::VG);
  nvgBeginPath(core::VG);
  nvgRoundedRect(core::VG, rect.x, rect.y, rect.w, rect.h, radius);
  nvgStrokeColor(core::VG, color);
  nvgStrokeWidth(core::VG, width);
  nvgStroke(core::VG);
  nvgRestore(core::VG);
}

void DrawVerticalLine(const NVGcolor& color, float x, float y0, float y1) {
  nvgStrokeColor(core::VG, color);
  nvgBeginPath(core::VG);
  nvgMoveTo(core::VG, x, y0);
  nvgLineTo(core::VG, x, y1);
  nvgStroke(core::VG);
}

void DrawHorizontalLine(const NVGcolor& color, float x0, float x1, float y) {
  nvgStrokeColor(core::VG, color);
  nvgBeginPath(core::VG);
  nvgMoveTo(core::VG, x0, y);
  nvgLineTo(core::VG, x1, y);
  nvgStroke(core::VG);
}

void DrawTextInRect(const Rect& rect,
                    const std::string& text,
                    const NVGcolor& color,
                    float x_padding) {
  ScopedRenderOffset offset(rect, true);
  float line_height;
  nvgTextMetrics(core::VG, NULL, NULL, &line_height);
  nvgFillColor(core::VG, color);
  static bool snap = true;
  if (snap) {
    nvgSave(core::VG);
    float current_transform[6];
    nvgCurrentTransform(core::VG, current_transform);
    CORE_DCHECK(current_transform[0] == 1.f && current_transform[1] == 0.f &&
                    current_transform[2] == 0.f && current_transform[3] == 1.f,
                "expecting no scale skew");
    nvgResetTransform(core::VG);
    nvgTransform(
        core::VG,
        1,
        0,
        0,
        1,
        std::round((current_transform[4] + x_padding) / core::GetDpiScale()) *
            core::GetDpiScale(),
        std::round((current_transform[5] + line_height) / core::GetDpiScale()) *
            core::GetDpiScale());
    nvgText(core::VG,
            0,
            0,
            &text.data()[0],
            &text.data()[text.size()]);
    nvgRestore(core::VG);
  } else {
    nvgText(core::VG,
            x_padding,
            line_height,
            &text.data()[0],
            &text.data()[text.size()]);
  }
}
#endif

