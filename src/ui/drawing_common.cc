// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/drawing_common.h"

#include "core/gfx.h"
#include "nanovg.h"
#include "ui/skin.h"

void UiDrawWindow(const char* title,
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
  nvgMoveTo(core::VG, x + 0.5f, y + 0.5f + sk.title_bar_size());
  nvgLineTo(core::VG, x + 0.5f + w - 1, y + 0.5f + sk.title_bar_size());
  nvgStrokeColor(core::VG, cs.border());
  nvgStroke(core::VG);

  nvgFontSize(core::VG, sk.title_bar_text_size());
  nvgFontFace(core::VG, "sans-bold");
  nvgTextAlign(core::VG, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);

  const float kFudgeTextLower = 1.f;  // Because of the 1 pixel divider.
  nvgFontBlur(core::VG, 2);
  nvgFillColor(core::VG, cs.title_bar_text_drop_shadow());
  nvgText(core::VG,
          x + w / 2,
          y + sk.title_bar_size() / 2 + kFudgeTextLower + 1,
          title,
          NULL);

  nvgFontBlur(core::VG, 0);
  nvgFillColor(
      core::VG,
      active ? cs.title_bar_text_active() : cs.title_bar_text_inactive());
  nvgText(core::VG,
          x + w / 2,
          y + sk.title_bar_size() / 2 + kFudgeTextLower,
          title,
          NULL);

  nvgRestore(core::VG);
}
