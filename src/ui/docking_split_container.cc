// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/docking_split_container.h"

#include "core/core.h"
#pragma message("todo!")
// #include "sg/render/renderer.h"
// #include "sg/render/scoped_render_offset.h"
#include "ui/docking_resizer.h"
// #include "ui/skin.h"

namespace {

int gSplitterWidth = 4;

}  // namespace

void DockingSplitContainer::SetSplitterWidth(int width) {
  gSplitterWidth = width;
}

int DockingSplitContainer::GetSplitterWidth() {
  return gSplitterWidth;
}

DockingSplitContainer::DockingSplitContainer(
    DockingSplitDirection direction, Dockable* left, Dockable* right)
    : direction_(direction),
      fraction_(0.5),
      left_(left),
      right_(right) {
}

DockingSplitContainer::~DockingSplitContainer() {
}

void DockingSplitContainer::SplitChild(
    DockingSplitDirection direction, Dockable* left, Dockable* right) {
  std::unique_ptr<Dockable>* to_replace;
  if (left_.get() == left || left_.get() == right)
    to_replace = &left_;
  else
    to_replace = &right_;
  DockingSplitContainer* previous_parent = (*to_replace)->parent();
  Rect previous_rect = (*to_replace)->GetScreenRect();
  to_replace->release();  // We're going re-own this pointer on the next line.
  DockingSplitContainer* replacement =
    new DockingSplitContainer(direction, left, right);
  replacement->set_parent(previous_parent);
  to_replace->reset(replacement);
  left->set_parent(replacement);
  right->set_parent(replacement);
  replacement->SetScreenRect(previous_rect);
}

void DockingSplitContainer::DeleteChild(Dockable* child) {
  if (left_.get() == child)
    parent()->Replace(this, right_.release());
  else if (right_.get() == child)
    parent()->Replace(this, left_.release());
}

Dockable* DockingSplitContainer::ReleaseChild(Dockable* child) {
  Dockable* result = NULL;
  if (left_.get() == child) {
    result = left_.release();
    parent()->Replace(this, right_.release());
    // |this| has been deleted here.
  } else if (right_.get() == child) {
    result = right_.release();
    parent()->Replace(this, left_.release());
    // |this| has been deleted here.
  }
  if (result)
    result->set_parent(NULL);
  return result;
}

void DockingSplitContainer::Replace(Dockable* target, Dockable* with) {
  if (left_.get() == target) {
    left_.reset(with);
    left_->set_parent(this);
  } else if (right_.get() == target) {
    right_.reset(with);
    right_->set_parent(this);
  } else {
    CORE_NOTREACHED();
  }
  SetScreenRect(GetScreenRect());
}

Dockable* DockingSplitContainer::GetSiblingOf(Dockable* child) {
  if (left_.get() == child)
    return right_.get();
  else if (right_.get() == child)
    return left_.get();
  else
    CORE_NOTREACHED();
  return NULL;
}

void DockingSplitContainer::SetScreenRect(const Rect& rect) {
  Dockable::SetScreenRect(rect);
  if (direction_ == kSplitVertical) {
    int width = GetScreenRect().w - gSplitterWidth;
    int width_for_left = static_cast<int>(width * fraction_);
    int width_for_right = static_cast<int>(width - width_for_left);
    left_->SetScreenRect(Rect(rect.x, rect.y, width_for_left, rect.h));
    right_->SetScreenRect(
        Rect(rect.x + width_for_left + gSplitterWidth, rect.y,
             width_for_right, rect.h));
  } else if (direction_ == kSplitHorizontal) {
    int height = GetScreenRect().h - gSplitterWidth;
    int height_for_left = static_cast<int>(height * fraction_);
    int height_for_right = static_cast<int>(height - height_for_left);
    left_->SetScreenRect(Rect(rect.x, rect.y, rect.w, height_for_left));
    right_->SetScreenRect(
        Rect(rect.x, rect.y + height_for_left + gSplitterWidth,
             rect.w, height_for_right));
  } else {
    CORE_DCHECK(direction_ == kSplitNoneRoot, "unexpected case");
    if (left_.get())
      left_->SetScreenRect(GetScreenRect());
  }
}

Rect DockingSplitContainer::GetRectForSplitter() {
  if (direction_ == kSplitVertical) {
    int width = GetScreenRect().w - gSplitterWidth;
    int width_for_left = static_cast<int>(width * fraction_);
    return Rect(width_for_left, 0, gSplitterWidth, GetScreenRect().h);
  } else if (direction_ == kSplitHorizontal) {
    int height = GetScreenRect().h - gSplitterWidth;
    int height_for_top = static_cast<int>(height * fraction_);
    return Rect(0, height_for_top, GetScreenRect().w, gSplitterWidth);
  }
  return Rect();
}

void DockingSplitContainer::Render() {
#if 0
  Point old_offset = renderer->GetRenderOffset();

  {
    ScopedRenderOffset left_offset(renderer, this, left_.get());
    left_->Render(renderer);
  }

  if (right_.get()) {
    ScopedRenderOffset right_offset(renderer, this, right_.get());
    right_->Render(renderer);
  } else {
    CHECK(direction_ == kSplitNoneRoot);
  }

  if (direction_ == kSplitVertical || direction_ == kSplitHorizontal) {
    renderer->SetDrawColor(Skin::current().GetColorScheme().border());
    renderer->DrawFilledRect(GetRectForSplitter());
  }
#endif
}

bool DockingSplitContainer::CouldStartDrag(DragSetup* drag_setup) {
  CORE_UNUSED(drag_setup);
  return false;
#if 0
  Point client_position = ToClient(drag_setup->screen_position);
  if (GetRectForSplitter().Contains(client_position)) {
    if (drag_setup->draggable)
      drag_setup->draggable->reset(new DockingResizer(this));
    if (direction_ == kSplitVertical)
      drag_setup->drag_direction = kDragDirectionLeftRight;
    else if (direction_ == kSplitHorizontal)
      drag_setup->drag_direction = kDragDirectionUpDown;
    return true;
  } else {
    if (left_->GetScreenRect().Contains(drag_setup->screen_position))
      return left_->CouldStartDrag(drag_setup);
    if (right_.get()) {
      if (right_->GetScreenRect().Contains(drag_setup->screen_position))
        return right_->CouldStartDrag(drag_setup);
    }
  }
  return false;
#endif
}

void DockingSplitContainer::SetFraction(double fraction) {
  fraction_ = fraction;
  // Propagate fraction changes to children.
  SetScreenRect(GetScreenRect());
}

void DockingSplitContainer::ReplaceLeft(Dockable* left) {
  CORE_CHECK(direction_ == kSplitNoneRoot && !right_.get(),
             "Can only use on root");
  left_.reset(left);
  left->SetScreenRect(Dockable::GetScreenRect());
}

Dockable* DockingSplitContainer::FindTopMostUnderPoint(const Point& point) {
  if (!GetScreenRect().Contains(point))
    return NULL;
  Dockable* left_contains = left_->FindTopMostUnderPoint(point);
  if (left_contains)
    return left_contains;
  Dockable* right_contains = right_->FindTopMostUnderPoint(point);
  if (right_contains)
    return right_contains;
  CORE_NOTREACHED();
  return NULL;
}
