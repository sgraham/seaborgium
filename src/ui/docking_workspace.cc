// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/docking_workspace.h"

#include <algorithm>

#include "ui/drawing_common.h"
#include "ui/docking_split_container.h"
#include "ui/focus.h"

// TODO(scottmg):
// This whole file sucks. Maybe it should just be a Dockable/Container too.

DockingWorkspace::DockingWorkspace() {
  root_.reset(new DockingSplitContainer(kSplitNoneRoot, NULL, NULL));
}

DockingWorkspace::~DockingWorkspace() {
}

void DockingWorkspace::Render() {
  if (root_->left()) {
    const Rect& rect = GetScreenRect();
    ScopedRenderOffset offset(rect.x, rect.y);
    root_->left()->Render();
  }
  if (draggable_.get())
    draggable_->Render();
}

void DockingWorkspace::SetRoot(Dockable* root) {
  root_->ReplaceLeft(root);
  root->set_parent(root_.get());
}

Dockable* DockingWorkspace::GetRoot() {
  return root_->left();
}

void DockingWorkspace::SetScreenRect(const Rect& rect) {
  root_->SetScreenRect(rect);
}

const Rect& DockingWorkspace::GetScreenRect() const {
  return root_->GetScreenRect();
}

bool DockingWorkspace::CouldStartDrag(DragSetup* drag_setup) {
  return root_->CouldStartDrag(drag_setup);
}

std::vector<Dockable*> DockingWorkspace::GetAllDockTargets() {
  std::vector<Dockable*> into;
  GetDockTargets(root_.get(), &into);
  std::sort(into.begin(), into.end());
  std::unique(into.begin(), into.end());
  return into;
}

Dockable* DockingWorkspace::FindTopMostUnderPoint(const Point& point) {
  return root_->left()->FindTopMostUnderPoint(point);
}

bool DockingWorkspace::NotifyMouseMoved(int x, int y, uint8_t modifiers) {
  mouse_position_.x = x;
  mouse_position_.y = y;
  if (draggable_.get()) {
    draggable_->Drag(mouse_position_);
    // TODO(scottmg): InvalidateImpl();
    return true;
  }
  UpdateCursorForLocation();
  Dockable* focused = GetFocusedContents();
  if (!focused || !focused->WantMouseEvents())
    return false;
  return focused->NotifyMouseMoved(x, y, modifiers);
}

bool DockingWorkspace::NotifyMouseWheel(int x,
                                        int y,
                                        float delta,
                                        uint8_t modifiers) {
  mouse_position_.x = x;
  mouse_position_.y = y;
  Dockable* focused = GetFocusedContents();
  if (!focused || !focused->WantMouseEvents())
    return false;
  return focused->NotifyMouseWheel(x, y, delta, modifiers);
}

bool DockingWorkspace::NotifyMouseButton(core::MouseButton::Enum button,
                                         bool down,
                                         uint8_t modifiers) {
  DragSetup drag_setup(mouse_position_, this);
  drag_setup.draggable = &draggable_;
  if (draggable_.get() && button == core::MouseButton::Left && !down) {
    draggable_.reset();
    UpdateCursorForLocation();
    // TODO(scottmg): Invalidate();
    return true;
  } else if (button == core::MouseButton::Left && down &&
             root_->left()->CouldStartDrag(&drag_setup)) {
    // TODO(scottmg): Invalidate();
    return true;
  } else if (button == core::MouseButton::Left && down) {
    Dockable* target = root_->left()->FindTopMostUnderPoint(mouse_position_);
    if (target) {
      SetFocusedContents(target);
      target->NotifyMouseMoved(mouse_position_.x, mouse_position_.y, modifiers);
      target->NotifyMouseButton(button, down, modifiers);
      // TODO(scottmg): Invalidate();
    }
  }
  return false;
}

bool DockingWorkspace::NotifyKey(core::Key::Enum key,
                                 bool down,
                                 uint8_t modifiers) {
  Dockable* focused = GetFocusedContents();
  if (!focused)
    return false;
  if (focused->WantKeyEvents() && focused->NotifyKey(key, down, modifiers))
    return true;
  // TODO(scottmg): Global keys.
  // return debug_presenter_notify_->NotifyKey(key, down, modifiers);
  return false;
}


void DockingWorkspace::GetDockTargets(
    Dockable* root, std::vector<Dockable*>* into) {
  // TODO(scottmg): This is crappy. DockingToolWindow claims that it's not
  // IsContainer, which happens to make this work, but is false obviously.
  // See also AsDockingSplitContainer, which is kind of what IsContainer
  // means, so it's not so bad. Sort of.
  if (!root)
    return;
  if (!root->IsContainer()) {
    into->push_back(root);
  } else {
    GetDockTargets(root->AsDockingSplitContainer()->left(), into);
    GetDockTargets(root->AsDockingSplitContainer()->right(), into);
  }
}

void DockingWorkspace::UpdateCursorForLocation() {
  DragSetup drag_setup(mouse_position_, this);
  if (root_->left()->CouldStartDrag(&drag_setup)) {
    if (drag_setup.drag_direction == kDragDirectionLeftRight)
      core::SetMouseCursor(core::MouseCursor::DragLeftRight);
    else if (drag_setup.drag_direction == kDragDirectionUpDown)
      core::SetMouseCursor(core::MouseCursor::DragUpDown);
    else if (drag_setup.drag_direction == kDragDirectionAll)
      core::SetMouseCursor(core::MouseCursor::DragAll);
  } else {
    core::SetMouseCursor(core::MouseCursor::Default);
  }
}
