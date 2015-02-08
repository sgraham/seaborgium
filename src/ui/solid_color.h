// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_SOLID_COLOR_H_
#define UI_SOLID_COLOR_H_

#include "core/gfx.h"
#include "ui/drawing_common.h"
#include "ui/widget.h"

class SolidColor : public Widget {
 public:
  explicit SolidColor(const NVGcolor& color)
      : color_(color) {
  }
  virtual ~SolidColor() {}

  virtual void Render() override {
    DrawSolidRect(GetClientRect(), color_);
  }

 private:
  NVGcolor color_;

  CORE_DISALLOW_COPY_AND_ASSIGN(SolidColor);
};

#endif  // UI_SOLID_COLOR_H_
