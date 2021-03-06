// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DRAG_SETUP_H_
#define DRAG_SETUP_H_

#include <memory>

#include "geometric_types.h"
#include "drag_direction.h"

class DockingWorkspace;
class Draggable;

class DragSetup {
 public:
  DragSetup(const Point& screen_position, DockingWorkspace* docking_workspace)
      : screen_position(screen_position),
        docking_workspace(docking_workspace),
        drag_direction(kDragDirectionNone),
        draggable(NULL) {}

  // Provided by query:
  Point screen_position;
  DockingWorkspace* docking_workspace;

  // Filled out by drag initiator:
  DragDirection drag_direction;
  std::unique_ptr<Draggable>* draggable;
};

#endif  // DRAG_SETUP_H_
