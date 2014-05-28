// Copyright 2012 The Chromium Authors.

#ifndef UI_SOLID_COLOR_H_
#define UI_SOLID_COLOR_H_

#include "core/gfx.h"
#include "nanovg.h"
#include "ui/dockable.h"

class SolidColor : public Dockable {
 public:
  explicit SolidColor(const NVGcolor& color)
      : color_(color) {
  }
  virtual ~SolidColor() {}

  virtual void Render() override {
    nvgBeginPath(core::VG);
    const Rect& rect = GetClientRect();
    nvgRect(core::VG,
            static_cast<float>(rect.x),
            static_cast<float>(rect.y),
            static_cast<float>(rect.w),
            static_cast<float>(rect.h));
    nvgFillColor(core::VG, color_);
    nvgFill(core::VG);
  }

 private:
  NVGcolor color_;

  CORE_DISALLOW_COPY_AND_ASSIGN(SolidColor);
};

#endif  // UI_SOLID_COLOR_H_