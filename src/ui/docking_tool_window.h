// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_DOCKING_TOOL_WINDOW_H_
#define UI_DOCKING_TOOL_WINDOW_H_

#include <string>

#include "core/core.h"
#include "ui/dockable.h"

// Renders window decoration, handles drag/re-dock interaction.
class DockingToolWindow : public Dockable {
 public:
  DockingToolWindow(Dockable* dockable, const std::string& title);
  virtual ~DockingToolWindow();

  virtual void Render() override;
  virtual void SetScreenRect(const Rect& rect) override;
  virtual bool CouldStartDrag(DragSetup* drag_setup) override;
  virtual Dockable* FindTopMostUnderPoint(const Point& point) override;

 private:
  Rect RectForTitleBar();

  Dockable* contents_;
  std::string title_;
};

#endif  // UI_DOCKING_TOOL_WINDOW_H_
