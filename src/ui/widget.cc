// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/widget.h"

#include "core/core.h"
#include "ui/docking_split_container.h"
// #include "sg/workspace.h"

Widget::Widget() : parent_(NULL) {
}

Widget::~Widget() {
}

DockingSplitContainer* Widget::AsDockingSplitContainer() {
  CORE_CHECK(IsDockingSplitContainer(), "Expected DockingSplitContainer");
  return reinterpret_cast<DockingSplitContainer*>(this);
}

void Widget::SetScreenRect(const Rect& rect) {
  rect_ = rect;
}

const Rect& Widget::GetScreenRect() const {
  return rect_;
}

Rect Widget::ClientToScreen(const Rect& rect) {
  CORE_CHECK(parent(), "can't convert unparented to screen");
  return Rect(rect.x + parent()->GetScreenRect().x,
              rect.y + parent()->GetScreenRect().y,
              rect.w,
              rect.h);
}

void Widget::Invalidate() {
#pragma message("todo!")
  // Workspace::Invalidate();
}

Widget* Widget::FindTopMostUnderPoint(const Point& point) {
  if (!rect_.Contains(point))
    return NULL;
  return this;
}
