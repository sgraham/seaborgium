// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TOOL_WINDOW_DRAGGER_H_
#define TOOL_WINDOW_DRAGGER_H_

#include <memory>
#include <vector>

#include "core.h"
#include "docking_split_container.h"
#include "draggable.h"
#include "gfx.h"

class Widget;
class DockingToolWindow;
class DockingWorkspace;
class DragSetup;
class Texture;
class Workspace;

struct DropTargetIndicator {
  Widget* dockable;
  Icon icon;
  Rect rect;
  DockingSplitDirection direction;
  bool this_dockable_first;
};

class ToolWindowDragger : public Draggable {
 public:
  ToolWindowDragger(
      DockingToolWindow* dragging,
      DragSetup* drag_setup);
  virtual ~ToolWindowDragger();

  void Drag(const Point& screen_point) override;
  void CancelDrag() override;
  void Render() override;

 private:
  void RefreshTargets();

  std::unique_ptr<DockingToolWindow> dragging_;
  Point pick_up_offset_;
  Point current_position_;
  Rect initial_screen_rect_;
  DropTargetIndicator* on_drop_target_;
  DockingWorkspace* docking_workspace_;
  std::vector<DropTargetIndicator> targets_;

  Widget* cancel_sibling_;
  DockingSplitDirection cancel_direction_;
  float cancel_fraction_;
  bool cancel_was_primary_;
};

#endif  // TOOL_WINDOW_DRAGGER_H_
