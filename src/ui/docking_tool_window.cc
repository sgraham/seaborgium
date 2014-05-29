// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/docking_tool_window.h"

#include "ui/draggable.h"
#include "ui/drawing_common.h"
#include "ui/focus.h"
#include "ui/skin.h"
#include "ui/tool_window_dragger.h"

namespace {

const int kTitleOffset = 3;

}  // namespace

DockingToolWindow::DockingToolWindow(Dockable* contents,
                                     const std::string& title)
    : contents_(contents), title_(title) {}

DockingToolWindow::~DockingToolWindow() {
}

Rect DockingToolWindow::RectForTitleBar() {
  return Rect(
      0, 0, Width(), static_cast<int>(Skin::current().title_bar_size()));
}

void DockingToolWindow::Render() {
  bool focused = GetFocusedContents() == contents_;
  UiDrawWindow(title_.c_str(), focused, 0, 0, Width(), Height());

  ScopedRenderOffset offset(this, contents_, true);
  contents_->Render();
}

void DockingToolWindow::SetScreenRect(const Rect& rect) {
  Dockable::SetScreenRect(rect);
  Rect contents_rect = rect;
  const Skin& skin = Skin::current();
  contents_rect.x += static_cast<int>(skin.border_size());
  contents_rect.y +=
      static_cast<int>(skin.title_bar_size() + skin.border_size());
  contents_rect.w -= static_cast<int>(skin.border_size() * 2);
  contents_rect.h -=
      static_cast<int>(skin.title_bar_size() + skin.border_size() * 2);
  contents_->SetScreenRect(contents_rect);
}

bool DockingToolWindow::CouldStartDrag(DragSetup* drag_setup) {
  if (RectForTitleBar().Contains(ToClient(drag_setup->screen_position))) {
    drag_setup->drag_direction = kDragDirectionAll;
    if (drag_setup->draggable)
      drag_setup->draggable->reset(new ToolWindowDragger(this, drag_setup));
    return true;
  }
  return false;
}

Dockable* DockingToolWindow::FindTopMostUnderPoint(const Point& point) {
  // We never want to return ourselves.
  return contents_->FindTopMostUnderPoint(point);
}
