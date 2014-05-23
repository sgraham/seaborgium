// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/docking_workspace.h"

#include <algorithm>

#include "ui/docking_split_container.h"

// TODO(scottmg):
// This whole file sucks. Maybe it should just be a Dockable/Container too.

DockingWorkspace::DockingWorkspace() {
  root_.reset(new DockingSplitContainer(kSplitNoneRoot, NULL, NULL));
}

DockingWorkspace::~DockingWorkspace() {
}

void DockingWorkspace::Render() {
  if (root_->left())
    root_->left()->Render();
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

Dockable* DockingWorkspace::FindTopMostUnderPoint(const Point& point) {
  return root_->FindTopMostUnderPoint(point);
}
