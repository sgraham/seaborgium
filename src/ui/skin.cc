// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/skin.h"

#include "core/core.h"
#include "core/gfx.h"
#include "nanovg.h"

// Temp manual config; Solarized Dark
namespace {

NVGcolor kBase03 = nvgRGB(0, 43, 54);
NVGcolor kBase02 = nvgRGB(7, 54, 66);
NVGcolor kBase01 = nvgRGB(88, 110, 117);
NVGcolor kBase00 = nvgRGB(101, 123, 131);
NVGcolor kBase0 = nvgRGB(131, 148, 150);
NVGcolor kBase1 = nvgRGB(147, 161, 161);
NVGcolor kBase2 = nvgRGB(238, 232, 213);
NVGcolor kBase3 = nvgRGB(253, 246, 227);
NVGcolor kYellow = nvgRGB(181, 137, 0);
NVGcolor kOrange = nvgRGB(203, 75, 22);
NVGcolor kRed = nvgRGB(220, 50, 47);
NVGcolor kMagenta = nvgRGB(211, 54, 130);
NVGcolor kViolet = nvgRGB(108, 113, 196);
NVGcolor kBlue = nvgRGB(38, 139, 210);
NVGcolor kCyan = nvgRGB(42, 161, 152);
NVGcolor kGreen = nvgRGB(133, 153, 0);

Skin g_skin;

}  // namespace

ColorScheme::ColorScheme()
  : border_(nvgRGB(0, 12, 16)),
    background_(kBase03),
    text_(kBase0),
    title_bar_active_(kBase00),
    title_bar_text_active_(kBase3),
    title_bar_inactive_(kBase02),
    title_bar_text_inactive_(kBase1),
    comment_(kBase01),
    comment_preprocessor_(kOrange),
    error_(kRed),
    keyword_(kGreen),
    keyword_type_(kYellow),
    literal_number_(kCyan),
    literal_string_(kViolet),
    klass_(kBlue),
    op_(kGreen),
    margin_(kBase02),
    margin_text_(kBase0),
    pc_indicator_(kYellow) {
}

Skin::Skin()
    : title_bar_size_(19),
      border_size_(3),
      status_bar_size_(26),
      text_line_height_(17) {
}

void Skin::Init() {
  CORE_CHECK(nvgCreateFont(core::VG, "sans", "art/DejaVuSans.ttf") != -1,
             "font load failed");
  CORE_CHECK(
      nvgCreateFont(core::VG, "sans-bold", "art/DejaVuSans-Bold.ttf") != -1,
      "font load failed");
  CORE_CHECK(nvgCreateFont(core::VG, "mono", "art/DejaVuSansMono.ttf") != -1,
             "font load failed");
  CORE_CHECK(nvgCreateFont(core::VG, "icons", "art/entypo.ttf") != -1,
             "font load failed");

#if 0
  pc_indicator_texture_.name = L"art/pc-location.png";
  breakpoint_texture_.name = L"art/breakpoint.png";
  dock_top_texture_.name = L"art/dock-indicator-top.png";
  dock_left_texture_.name = L"art/dock-indicator-left.png";
  dock_right_texture_.name = L"art/dock-indicator-right.png";
  dock_bottom_texture_.name = L"art/dock-indicator-bottom.png";
  vscrollbar_top_texture_.name = L"art/scrollbar-top.png";
  vscrollbar_middle_texture_.name = L"art/scrollbar-middle.png";
  vscrollbar_bottom_texture_.name = L"art/scrollbar-bottom.png";
  tree_collapsed_texture_.name = L"art/tree-collapsed.png";
  tree_expanded_texture_.name = L"art/tree-expanded.png";
#endif
}

// static
const Skin& Skin::current() {
  return g_skin;
}

// static
void Skin::LoadData() { g_skin.Init(); }
