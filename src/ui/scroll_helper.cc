// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/scroll_helper.h"

#include <algorithm>

#include "core/gfx.h"
#include "ui/drawing_common.h"
#include "ui/skin.h"

namespace {

const int kFadeOutAfterTicks = 90;
const int kFadeOutOverTicks = 30;

}  // namespace

ScrollHelper::ScrollHelper(ScrollHelperDataProvider* data_provider,
                           float num_pixels_in_line)
    : y_pixel_scroll_(0),
      y_pixel_scroll_target_(0),
      // Start hidden.
      ticks_since_stopped_moving_(kFadeOutAfterTicks + kFadeOutOverTicks),
      num_pixels_in_line_(static_cast<int>(num_pixels_in_line)),
      data_provider_(data_provider) {
}

ScrollHelper::~ScrollHelper() {
}

bool ScrollHelper::Update() {
  float delta = (y_pixel_scroll_target_ - y_pixel_scroll_) * 0.2f;
  int before = y_pixel_scroll_;
  y_pixel_scroll_ += static_cast<int>(delta);
  if (before == y_pixel_scroll_) {
    y_pixel_scroll_ = y_pixel_scroll_target_;
    bool invalidate =
        ticks_since_stopped_moving_ < (kFadeOutAfterTicks + kFadeOutOverTicks);
    if (invalidate)
      ticks_since_stopped_moving_++;
    return invalidate;
  }
  ticks_since_stopped_moving_ = 0;
  return true;
}

void ScrollHelper::RenderScrollIndicators() {
  int scrollable_height = data_provider_->GetContentSize() +
                          static_cast<int>(data_provider_->GetScreenRect().h) -
                          num_pixels_in_line_;
  Rect screen_rect = data_provider_->GetScreenRect();
  int visible_height = static_cast<int>(screen_rect.h);
  double fraction_visible = static_cast<double>(visible_height) /
                            static_cast<double>(scrollable_height);
  float scrollbar_height =
      static_cast<float>(visible_height * fraction_visible);

  double offset_fraction = static_cast<double>(y_pixel_scroll_) /
                           static_cast<double>(scrollable_height);
  float scrollbar_offset = static_cast<float>(visible_height * offset_fraction);

  double alpha = 1.0;
  if (ticks_since_stopped_moving_ >= kFadeOutAfterTicks) {
    alpha = 1.0 - static_cast<double>(
        ticks_since_stopped_moving_ - kFadeOutAfterTicks) / kFadeOutOverTicks;
  }

  DrawOutlineRoundedRect(
      Rect(screen_rect.w - 10.f, scrollbar_offset, 8.f, scrollbar_height),
      nvgRGBA(80, 80, 80, static_cast<unsigned char>(alpha * 255)),
      4.f,
      1.f);
}

bool ScrollHelper::ClampScrollTarget() {
  y_pixel_scroll_target_ = std::max(0, y_pixel_scroll_target_);
  int largest_possible =
      data_provider_->GetContentSize() - num_pixels_in_line_;
  y_pixel_scroll_target_ = std::min(largest_possible, y_pixel_scroll_target_);
  // Not this, if we want the scrollbar to re-appear if, e.g. you press up
  // while at the top of the document.
  // return y_pixel_scroll_ != y_pixel_scroll_target_;
  ticks_since_stopped_moving_ = 0;
  return true;
}

bool ScrollHelper::ScrollPixels(int delta) {
  y_pixel_scroll_target_ += delta;
  return ClampScrollTarget();
}

bool ScrollHelper::ScrollLines(int delta) {
  y_pixel_scroll_target_ += delta * num_pixels_in_line_;
  return ClampScrollTarget();
}

bool ScrollHelper::ScrollPages(int delta) {
  float screen_height = data_provider_->GetScreenRect().h;
  y_pixel_scroll_target_ +=
      static_cast<int>(delta * screen_height - num_pixels_in_line_);
  return ClampScrollTarget();
}

bool ScrollHelper::ScrollToBeginning() {
  y_pixel_scroll_target_ = 0;
  return ClampScrollTarget();
}

bool ScrollHelper::ScrollToEnd() {
  y_pixel_scroll_target_ =
      data_provider_->GetContentSize() - num_pixels_in_line_;
  return ClampScrollTarget();
}

void ScrollHelper::CommonNotifyKey(core::Key::Enum key,
                                   bool down,
                                   uint8_t modifiers,
                                   bool* invalidate,
                                   bool* handled) {
  *invalidate = false;
  *handled = false;
  if (down) {
    if (key == core::Key::Down) {
      *invalidate = ScrollLines(1);
      *handled = true;
    } else if (key == core::Key::Up) {
      *invalidate = ScrollLines(-1);
      *handled = true;
    } else if (key == core::Key::PageUp ||
               (key == core::Key::Space &&
                (modifiers & (core::Modifier::LeftShift |
                              core::Modifier::RightShift)) != 0)) {
      *invalidate = ScrollPages(-1);
      *handled = true;
    } else if (key == core::Key::PageDown ||
               (key == core::Key::Space &&
                (modifiers & (core::Modifier::LeftShift |
                              core::Modifier::RightShift)) == 0)) {
      *invalidate = ScrollPages(1);
      *handled = true;
    } else if (key == core::Key::Home) {
      *invalidate = ScrollToBeginning();
      *handled = true;
    } else if (key == core::Key::End) {
      *invalidate = ScrollToEnd();
      *handled = true;
    }
  }
}

void ScrollHelper::CommonMouseWheel(float delta,
                                    uint8_t modifiers,
                                    bool* invalidate,
                                    bool* handled) {
  CORE_UNUSED(modifiers);
  ScrollPixels(static_cast<int>(delta * 3));
  *invalidate = true;
  *handled = true;
}

