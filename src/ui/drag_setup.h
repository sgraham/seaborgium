// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SG_UI_DRAG_SETUP_H_
#define SG_UI_DRAG_SETUP_H_

#include "core/geometric_types.h"
#include "ui/drag_direction.h"

#include <memory>

class DockingWorkspace;
class Draggable;

class DragSetup {
 public:
  DragSetup(const Point& screen_position, DockingWorkspace* docking_workspace)
      : screen_position(screen_position),
        docking_workspace(docking_workspace),
        drag_direction(kDragDirectionNone),
        draggable(NULL) {
  }

  // Provided by query:
  Point screen_position;
  DockingWorkspace* docking_workspace;

  // Filled out by drag initiator:
  DragDirection drag_direction;
  std::unique_ptr<Draggable>* draggable;
};

#endif  // SG_UI_DRAG_SETUP_H_
