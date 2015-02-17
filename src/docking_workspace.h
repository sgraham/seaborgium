// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DOCKING_WORKSPACE_H_
#define DOCKING_WORKSPACE_H_

#include <memory>
#include <vector>

#include "drag_direction.h"
#include "draggable.h"
#include "entry.h"
#include "widget.h"

class DockingSplitContainer;

// Top level container holding a tree of |Widget|s.
class DockingWorkspace : public core::InputHandler {
 public:
  DockingWorkspace();
  virtual ~DockingWorkspace();

  void Render();
  bool CouldStartDrag(DragSetup* drag_setup);

  // Takes ownership.
  void SetRoot(Widget* root);
  Widget* GetRoot();

  void SetScreenRect(const Rect& rect);
  const Rect& GetScreenRect() const;

  // All non-container children that have an immediate parent of a
  // DockingSplitContainer. I think.
  std::vector<Widget*> GetAllDockTargets();

  Widget* FindTopMostUnderPoint(const Point& point);

  // Implementation of core::InputHandler:
  virtual bool WantMouseEvents() override { return true; }
  virtual bool WantKeyEvents() override { return true; }
  virtual bool NotifyMouseMoved(int x, int y, uint8_t modifiers) override;
  virtual bool NotifyMouseWheel(int x, int y, float delta, uint8_t modifiers)
      override;
  virtual bool NotifyMouseButton(int x,
                                 int y,
                                 core::MouseButton::Enum button,
                                 bool down,
                                 uint8_t modifiers) override;
  virtual bool NotifyKey(core::Key::Enum key,
                         bool down,
                         uint8_t modifiers) override;
  virtual bool NotifyChar(int character) override;

 private:
  void GetDockTargets(Widget* root, std::vector<Widget*>* into);

  void UpdateCursorForLocation();

  std::unique_ptr<DockingSplitContainer> root_;

  Point mouse_position_;

  std::unique_ptr<Draggable> draggable_;
};

#endif  // DOCKING_WORKSPACE_H_
