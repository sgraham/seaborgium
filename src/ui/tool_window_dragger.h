// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_TOOL_WINDOW_DRAGGER_H_
#define UI_TOOL_WINDOW_DRAGGER_H_

#include <memory>
#include <vector>

#include "core/core.h"
#include "ui/docking_split_container.h"
#include "ui/draggable.h"

class Dockable;
class DockingToolWindow;
class DockingWorkspace;
class DragSetup;
class Texture;
class Workspace;

struct DropTargetIndicator {
  Dockable* dockable;
  const Texture* texture;
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

  virtual void Drag(const Point& screen_point) override;
  virtual void CancelDrag() override;
  virtual void Render() override;

 private:
  void RefreshTargets();

  std::unique_ptr<DockingToolWindow> dragging_;
  Point pick_up_offset_;
  Point current_position_;
  Rect initial_screen_rect_;
  DropTargetIndicator* on_drop_target_;
  DockingWorkspace* docking_workspace_;
  std::vector<DropTargetIndicator> targets_;

  Dockable* cancel_sibling_;
  DockingSplitDirection cancel_direction_;
  double cancel_fraction_;
  bool cancel_was_primary_;
};

#endif  // UI_TOOL_WINDOW_DRAGGER_H_
