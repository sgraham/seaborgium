// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/tree_grid.h"

#include "core/gfx.h"
#include "nanovg.h"
#include "ui/skin.h"

TreeGrid::TreeGrid() {
}

TreeGrid::~TreeGrid() {
}

void TreeGrid::Render() {
  nvgSave(core::VG);
  nvgBeginPath(core::VG);
  const Rect& rect = GetClientRect();
  //const ColorScheme& cs = Skin::current().GetColorScheme();
  nvgRect(core::VG,
          static_cast<float>(rect.x),
          static_cast<float>(rect.y),
          static_cast<float>(rect.w),
          static_cast<float>(rect.h));
  nvgFillColor(core::VG, nvgRGBA(255, 255, 255, 255));
  nvgFill(core::VG);
  nvgRestore(core::VG);
}
