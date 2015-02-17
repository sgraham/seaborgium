// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "skin.h"

#include "core.h"
#include "entry.h"
#include "gfx.h"

// Temp manual config; Solarized Dark
namespace {

Color kBase03(0x002b36, 1.f);
Color kBase02(0x073642, 1.f);
Color kBase01(0x586e75, 1.f);
Color kBase00(0x657b83, 1.f);
Color kBase0(0x839496, 1.f);
Color kBase1(0x93a1a1, 1.f);
Color kBase2(0xeee8d5, 1.f);
Color kBase3(0xfdf6e3, 1.f);
Color kYellow(0xb58900, 1.f);
Color kOrange(0xcb4b16, 1.f);
Color kRed(0xdc322f, 1.f);
Color kMagenta(0xd33682, 1.f);
Color kViolet(0x6c71c4, 1.f);
Color kBlue(0x268bd2, 1.f);
Color kCyan(0x2aa198, 1.f);
Color kGreen(0x859900, 1.f);

Skin g_skin;

}  // namespace

ColorScheme::ColorScheme()
    : border_(0, 0, 0),
      background_(kBase03),
      text_(kBase0),
      text_selection_(kYellow),
      cursor_(kBase1),
      title_bar_active_inner_(0.5f, 1.f, 1.f, .75f),
      title_bar_active_outer_(0.5f, 1.f, 1.f, 0.125f),
      title_bar_inactive_inner_(0.5f, 0.5f, 0.5f, 0.03f),
      title_bar_inactive_outer_(0.f, 0.f, 0.f, 0.06f),
      title_bar_text_active_(kBase2),
      title_bar_text_inactive_(kBase00),
      title_bar_text_drop_shadow_(0, 0, 0, 0.5f),
      drop_shadow_inner_(0, 0, 0, 0.5f),
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
      text_line_height_(17.f) {
}

void Skin::Init() {
}

// static
const Skin& Skin::current() {
  return g_skin;
}

// static
void Skin::LoadData() {
  g_skin.Init();
}
