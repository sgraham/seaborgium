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

DockingToolWindow::DockingToolWindow(Widget* contents, const std::string& title)
    : contents_(contents), title_(title) {
  contents->set_parent(this);
}

DockingToolWindow::~DockingToolWindow() {
}

Rect DockingToolWindow::RectForTitleBar() {
  return Rect(0, 0, Width(), Skin::current().title_bar_size());
}

void DockingToolWindow::Render() {
  bool focused = GetFocusedContents() == contents_;
  DrawWindow(title_.c_str(), focused, 0, 0, Width(), Height());

  ScopedRenderOffset offset(this, contents_, true);
  contents_->Render();
}

void DockingToolWindow::SetScreenRect(const Rect& rect) {
  Widget::SetScreenRect(rect);
  Rect contents_rect = rect;
  const Skin& skin = Skin::current();
  contents_rect.x += 0;
  contents_rect.y += skin.title_bar_size();
  contents_rect.h -= skin.title_bar_size();
  contents_->SetScreenRect(contents_rect);
}

bool DockingToolWindow::CouldStartDrag(DragSetup* drag_setup) {
  if (RectForTitleBar().Contains(ScreenToClient(drag_setup->screen_position))) {
    drag_setup->drag_direction = kDragDirectionAll;
    if (drag_setup->draggable)
      drag_setup->draggable->reset(new ToolWindowDragger(this, drag_setup));
    return true;
  }
  return contents_->CouldStartDrag(drag_setup);
}

Widget* DockingToolWindow::FindTopMostUnderPoint(const Point& point) {
  // We never want to return ourselves.
  return contents_->FindTopMostUnderPoint(point);
}
