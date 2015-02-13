// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/skin.h"

#include "core/core.h"
#include "core/entry.h"
#include "core/gfx.h"

// Temp manual config; Solarized Dark
namespace {

core::Color kBase03(0, 43, 54);
core::Color kBase02(7, 54, 66);
core::Color kBase01(88, 110, 117);
core::Color kBase00(101, 123, 131);
core::Color kBase0(131, 148, 150);
core::Color kBase1(147, 161, 161);
core::Color kBase2(238, 232, 213);
core::Color kBase3(253, 246, 227);
core::Color kYellow(181, 137, 0);
core::Color kOrange(203, 75, 22);
core::Color kRed(220, 50, 47);
core::Color kMagenta(211, 54, 130);
core::Color kViolet(108, 113, 196);
core::Color kBlue(38, 139, 210);
core::Color kCyan(42, 161, 152);
core::Color kGreen(133, 153, 0);

Skin g_skin;

}  // namespace

ColorScheme::ColorScheme()
  : border_(0, 0, 0, 255),
    background_(kBase03),
    text_(kBase0),
    text_selection_(kYellow),
    cursor_(kBase1),
    title_bar_active_inner_(128, 255, 255, 192),
    title_bar_active_outer_(128, 255, 255, 32),
    title_bar_inactive_inner_(128, 128, 128, 8),
    title_bar_inactive_outer_(0, 0, 0, 16),
    title_bar_text_active_(kBase2),
    title_bar_text_inactive_(kBase00),
    title_bar_text_drop_shadow_(0, 0, 0, 128),
    drop_shadow_inner_(0, 0, 0, 128),
    drop_shadow_outer_(0, 0, 0, 0),
    drop_indicator_(kBase1),
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
  text_selection_.a = .3f;
}

Skin::Skin()
    : title_bar_size_(24.f),
      title_bar_text_size_(14.f),
      border_size_(3.f),
      status_bar_size_(26.f),
      text_line_height_(17.f) {}

void Skin::Init() {
  // TODO !
#if 0
  CORE_CHECK(nvgCreateFont(core::VG, "sans", "art/Roboto-Regular.ttf") != -1,
             "font load failed");
  CORE_CHECK(
      nvgCreateFont(core::VG, "sans-bold", "art/DejaVuSans-Bold.ttf") != -1,
      "font load failed");
  CORE_CHECK(nvgCreateFont(core::VG, "mono", "art/DejaVuSansMono.ttf") != -1,
             "font load failed");
  CORE_CHECK(nvgCreateFont(core::VG, "icons", "art/entypo.ttf") != -1,
             "font load failed");
#endif

  dock_bottom_icon_ = "\xee\x9d\x98";  // circled-down
  dock_left_icon_ = "\xee\x9d\x99";  // circled-left
  dock_right_icon_ = "\xee\x9d\x9a";  // circled-right
  dock_top_icon_ = "\xee\x9d\x9b";  // circled-up
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
