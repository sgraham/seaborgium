// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/dockable.h"

#include "core/core.h"
#include "ui/docking_split_container.h"
#pragma message("todo!")
//#include "sg/workspace.h"

Dockable::Dockable() : parent_(NULL) {
}

Dockable::~Dockable() {
}

DockingSplitContainer* Dockable::AsDockingSplitContainer() {
  CORE_CHECK(IsContainer(), "Expected container");
  return reinterpret_cast<DockingSplitContainer*>(this);
}

void Dockable::SetScreenRect(const Rect& rect) {
  rect_ = rect;
}

const Rect& Dockable::GetScreenRect() const {
  return rect_;
}

void Dockable::Invalidate() {
#pragma message("todo!")
  //Workspace::Invalidate();
}

Dockable* Dockable::FindTopMostUnderPoint(const Point& point) {
  if (!rect_.Contains(point))
    return NULL;
  return this;
}
