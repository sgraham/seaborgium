// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DOCKING_TOOL_WINDOW_H_
#define DOCKING_TOOL_WINDOW_H_

#include <string>

#include "core.h"
#include "widget.h"

// Renders window decoration, handles drag/re-dock interaction.
class DockingToolWindow : public Widget {
 public:
  DockingToolWindow(Widget* dockable, const std::string& title);
  virtual ~DockingToolWindow();

  virtual void Render() override;
  virtual void SetScreenRect(const Rect& rect) override;
  virtual bool CouldStartDrag(DragSetup* drag_setup) override;
  virtual Widget* FindTopMostUnderPoint(const Point& point) override;

 private:
  Rect RectForTitleBar();

  Widget* contents_;
  std::string title_;
};

#endif  // DOCKING_TOOL_WINDOW_H_
