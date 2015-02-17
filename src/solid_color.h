// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SOLID_COLOR_H_
#define SOLID_COLOR_H_

#include "gfx.h"
#include "widget.h"

class SolidColor : public Widget {
 public:
  explicit SolidColor(const Color& color) : color_(color) {}
  virtual ~SolidColor() {}

  void Render() override { DrawSolidRect(GetClientRect(), color_); }

 private:
  Color color_;

  DISALLOW_COPY_AND_ASSIGN(SolidColor);
};

#endif  // SOLID_COLOR_H_
