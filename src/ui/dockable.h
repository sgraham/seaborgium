// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_DOCKABLE_H_
#define UI_DOCKABLE_H_

#include "core/core.h"
#include "core/geometric_types.h"
#include "ui/drag_setup.h"
#pragma message("todo!")
//#include "ui/input.h"

class DockingSplitContainer;
class Draggable;
class Renderer;
class Workspace;

class Dockable /*: public InputHandler*/ {
 public:
  Dockable();
  virtual ~Dockable();

  virtual bool CanUndock() const { return false; }
  virtual bool IsContainer() const { return false; }
  virtual DockingSplitContainer* AsDockingSplitContainer();

  virtual void SetScreenRect(const Rect& rect);
  const Rect& GetScreenRect() const;

  void set_parent(DockingSplitContainer* parent) { parent_ = parent; }
  DockingSplitContainer* parent() { return parent_; }

  virtual void Render(Renderer* renderer) { CORE_UNUSED(renderer); }
  virtual void Invalidate();
  virtual bool CouldStartDrag(DragSetup* drag_setup) {
    CORE_UNUSED(drag_setup);
    return false;
  }

  virtual int X() const { return GetScreenRect().x; }
  virtual int Y() const { return GetScreenRect().y; }
  virtual int Width() const { return GetScreenRect().w; }
  virtual int Height() const { return GetScreenRect().h; }
  Rect GetClientRect() {
    return Rect(0, 0, GetScreenRect().w, GetScreenRect().h);
  }
  Point ToClient(const Point& point) {
    return Point(point.x - GetScreenRect().x, point.y - GetScreenRect().y);
  }

  // Find the Dockable in the tree that's top-most in the stacking order (or
  // equivalently lowest in the tree).
  virtual Dockable* FindTopMostUnderPoint(const Point& point);

  /*
  // Default implementation of InputHandler.
  virtual bool NotifyMouseMoved(
      int x, int y, int dx, int dy, const InputModifiers& modifiers) override {
    return false;
  }
  virtual bool NotifyMouseWheel(
      int delta, const InputModifiers& modifiers) override {
    return false;
  }
  virtual bool NotifyMouseButton(
      int index, bool down, const InputModifiers& modifiers) override {
    return false;
  }
  virtual bool NotifyKey(
      InputKey key, bool down, const InputModifiers& modifiers) override {
    return false;
  }
  virtual bool WantMouseEvents() { return false; }
  virtual bool WantKeyEvents() { return false; }
  */

 private:
  DockingSplitContainer* parent_;
  Rect rect_;

  CORE_DISALLOW_COPY_AND_ASSIGN(Dockable);
};

#endif  // UI_DOCKABLE_H_
