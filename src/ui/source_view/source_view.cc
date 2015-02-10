// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/source_view/source_view.h"

#include "ui/drawing_common.h"
#include "ui/skin.h"

SourceView::SourceView() : scroll_(this, Skin::current().text_line_height()) {
}

SourceView::~SourceView() {
}

void SourceView::SetFilePath(const std::string& path) {
  CORE_UNUSED(path);
}

bool SourceView::NotifyMouseWheel(int x,
                                  int y,
                                  float delta,
                                  uint8_t modifiers) {
  CORE_UNUSED(x);
  CORE_UNUSED(y);
  bool invalidate = false;
  bool handled = false;
  scroll_.CommonMouseWheel(delta, modifiers, &invalidate, &handled);
  return handled;
}

bool SourceView::NotifyKey(core::Key::Enum key, bool down, uint8_t modifiers) {
  bool invalidate = false;
  bool handled = false;
  scroll_.CommonNotifyKey(key, down, modifiers, &invalidate, &handled);
  return handled;
}

void SourceView::Render() {
  scroll_.Update();
  const Skin& skin = Skin::current();
  const ColorScheme& cs = skin.GetColorScheme();
  DrawSolidRect(GetClientRect(), cs.background());

  {
    ScopedRenderOffset scroll_offset(0,
                                     static_cast<float>(scroll_.GetOffset()));
    nvgFontSize(core::VG, 14.0f);
    nvgFontFace(core::VG, "mono");
    nvgFillColor(core::VG, Skin::current().GetColorScheme().text());
    nvgText(core::VG, 100, 200, "int main(int argc, char** argv) {", NULL);
  }

  scroll_.RenderScrollIndicators();
}

int SourceView::GetContentSize() {
  return 2000;
}

const Rect& SourceView::GetScreenRect() const {
  return Widget::GetScreenRect();
}
