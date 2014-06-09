// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_DOCKABLE_H_
#define UI_DOCKABLE_H_

#include "core/core.h"
#include "core/entry.h"
#include "core/geometric_types.h"
#include "ui/drag_setup.h"

class DockingSplitContainer;
class Draggable;
class Renderer;
class Workspace;

class Dockable : public core::InputHandler {
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

  virtual void Render() {}
  virtual void Invalidate();
  virtual bool CouldStartDrag(DragSetup* drag_setup) {
    CORE_UNUSED(drag_setup);
    return false;
  }

  virtual float X() const { return GetScreenRect().x; }
  virtual float Y() const { return GetScreenRect().y; }
  virtual float Width() const { return GetScreenRect().w; }
  virtual float Height() const { return GetScreenRect().h; }
  Rect GetClientRect() {
    return Rect(0, 0, GetScreenRect().w, GetScreenRect().h);
  }
  Point ToClient(const Point& point) {
    return Point(point.x - GetScreenRect().x, point.y - GetScreenRect().y);
  }

  // Find the Dockable in the tree that's top-most in the stacking order (or
  // equivalently lowest in the tree).
  virtual Dockable* FindTopMostUnderPoint(const Point& point);

  // Default implementation of core::InputHandler.
  virtual bool NotifyMouseMoved(int /*x*/,
                                int /*y*/,
                                uint8_t /*modifiers*/) override {
    return false;
  }
  virtual bool NotifyMouseWheel(int /*x*/,
                                int /*y*/,
                                float /*delta*/,
                                uint8_t /*modifiers*/) override {
    return false;
  }
  virtual bool NotifyMouseButton(core::MouseButton::Enum /*button*/,
                                 bool /*down*/,
                                 uint8_t /*modifiers*/) override {
    return false;
  }
  virtual bool NotifyKey(core::Key::Enum /*key*/,
                         bool /*down*/,
                         uint8_t /*modifiers*/) override {
    return false;
  }
  virtual bool NotifyChar(int /*character*/) override {
    return false;
  }
  virtual bool WantMouseEvents() { return false; }
  virtual bool WantKeyEvents() { return false; }

 private:
  DockingSplitContainer* parent_;
  Rect rect_;

  CORE_DISALLOW_COPY_AND_ASSIGN(Dockable);
};

#endif  // UI_DOCKABLE_H_
