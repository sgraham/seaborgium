// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "docking_resizer.h"

#include <algorithm>

#include "core.h"
#include "docking_split_container.h"

DockingResizer::DockingResizer(DockingSplitContainer* resizing)
    : resizing_(resizing) {
  // Only one of these is relevant, so we handle both vertical and horizontal
  // by making the point on the splitter be the same fraction.
  float parent_size = ParentSize();
  initial_location_ =
      Point(parent_size * resizing_->fraction(),
            parent_size * resizing_->fraction());
}

DockingResizer::~DockingResizer() {
}

void DockingResizer::Drag(const Point& screen_point) {
  Point point_in_parent = resizing_->ScreenToClient(screen_point);
  float parent_size = ParentSize();
  float clamped = std::min(
      std::max(ComponentForDirection(point_in_parent), 0.f), parent_size);
  resizing_->SetFraction(clamped / parent_size);
}

void DockingResizer::CancelDrag() {
  float parent_size = ParentSize();
  resizing_->SetFraction(initial_location_.x / parent_size);
}

void DockingResizer::Render() {
  // Nothing to do, we apply directly.
}

float DockingResizer::ParentSize() {
  const Rect& parent_rect = resizing_->GetScreenRect();
  float splitter_width = DockingSplitContainer::GetSplitterWidth();
  CORE_CHECK(resizing_->direction() == kSplitVertical ||
             resizing_->direction() == kSplitHorizontal, "no splitter?");
  if (resizing_->direction() == kSplitVertical)
    return parent_rect.w - splitter_width;
  else
    return parent_rect.h - splitter_width;
}

float DockingResizer::ComponentForDirection(const Point& point) {
  CORE_CHECK(resizing_->direction() == kSplitVertical ||
                 resizing_->direction() == kSplitHorizontal,
             "no splitter?");
  if (resizing_->direction() == kSplitVertical)
    return point.x;
  else
    return point.y;
}
