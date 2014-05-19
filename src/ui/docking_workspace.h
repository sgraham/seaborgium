// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_DOCKING_WORKSPACE_H_
#define UI_DOCKING_WORKSPACE_H_


#include "ui/dockable.h"
#include "ui/drag_direction.h"

#include <memory>
#include <vector>

class DockingSplitContainer;

// Top level container holding a tree of |Dockable|s.
class DockingWorkspace {
 public:
  DockingWorkspace();
  virtual ~DockingWorkspace();

  void Render(Renderer* renderer);
  bool CouldStartDrag(DragSetup* drag_setup);

  // Takes ownership.
  void SetRoot(Dockable* root);
  Dockable* GetRoot();

  void SetScreenRect(const Rect& rect);
  const Rect& GetScreenRect() const;

  // All non-container children that have an immediate parent of a
  // DockingSplitContainer. I think.
  std::vector<Dockable*> GetAllDockTargets();

  Dockable* FindTopMostUnderPoint(const Point& point);

 private:
  void GetDockTargets(Dockable* root, std::vector<Dockable*>* into);

  std::unique_ptr<DockingSplitContainer> root_;
};

#endif  // UI_DOCKING_WORKSPACE_H_
