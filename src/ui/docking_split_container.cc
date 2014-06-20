// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/docking_split_container.h"

#include "core/core.h"
#include "core/gfx.h"
#include "ui/drawing_common.h"
#include "ui/docking_resizer.h"
// #include "ui/skin.h"

namespace {

float gSplitterWidth = 4;

}  // namespace

void DockingSplitContainer::SetSplitterWidth(float width) {
  gSplitterWidth = width;
}

float DockingSplitContainer::GetSplitterWidth() {
  return gSplitterWidth;
}

DockingSplitContainer::DockingSplitContainer(
    DockingSplitDirection direction, Widget* left, Widget* right)
    : direction_(direction),
      fraction_(0.5),
      left_(left),
      right_(right) {
}

DockingSplitContainer::~DockingSplitContainer() {
}

void DockingSplitContainer::SplitChild(
    DockingSplitDirection direction, Widget* left, Widget* right) {
  std::unique_ptr<Widget>* to_replace;
  if (left_.get() == left || left_.get() == right)
    to_replace = &left_;
  else
    to_replace = &right_;
  DockingSplitContainer* previous_parent =
      (*to_replace)->parent()->AsDockingSplitContainer();
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

void DockingSplitContainer::DeleteChild(Widget* child) {
  if (left_.get() == child)
    parent()->AsDockingSplitContainer()->Replace(this, right_.release());
  else if (right_.get() == child)
    parent()->AsDockingSplitContainer()->Replace(this, left_.release());
}

Widget* DockingSplitContainer::ReleaseChild(Widget* child) {
  Widget* result = NULL;
  if (left_.get() == child) {
    result = left_.release();
    parent()->AsDockingSplitContainer()->Replace(this, right_.release());
    // |this| has been deleted here.
  } else if (right_.get() == child) {
    result = right_.release();
    parent()->AsDockingSplitContainer()->Replace(this, left_.release());
    // |this| has been deleted here.
  }
  if (result)
    result->set_parent(NULL);
  return result;
}

void DockingSplitContainer::Replace(Widget* target, Widget* with) {
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

Widget* DockingSplitContainer::GetSiblingOf(Widget* child) {
  if (left_.get() == child)
    return right_.get();
  else if (right_.get() == child)
    return left_.get();
  else
    CORE_NOTREACHED();
  return NULL;
}

void DockingSplitContainer::SetScreenRect(const Rect& rect) {
  Widget::SetScreenRect(rect);
  if (direction_ == kSplitVertical) {
    float width = GetScreenRect().w - gSplitterWidth;
    float width_for_left = width * fraction_;
    float width_for_right = width - width_for_left;
    left_->SetScreenRect(Rect(rect.x, rect.y, width_for_left, rect.h));
    right_->SetScreenRect(
        Rect(rect.x + width_for_left + gSplitterWidth, rect.y,
             width_for_right, rect.h));
  } else if (direction_ == kSplitHorizontal) {
    float height = GetScreenRect().h - gSplitterWidth;
    float height_for_left = height * fraction_;
    float height_for_right = height - height_for_left;
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
    float width = GetScreenRect().w - gSplitterWidth;
    float width_for_left = width * fraction_;
    return Rect(width_for_left, 0, gSplitterWidth, GetScreenRect().h);
  } else if (direction_ == kSplitHorizontal) {
    float height = GetScreenRect().h - gSplitterWidth;
    float height_for_top = height * fraction_;
    return Rect(0, height_for_top, GetScreenRect().w, gSplitterWidth);
  }
  return Rect();
}

void DockingSplitContainer::Render() {
  {
    ScopedRenderOffset left_offset(this, left_.get(), true);
    left_->Render();
  }

  if (right_.get()) {
    ScopedRenderOffset right_offset(this, right_.get(), true);
    right_->Render();
  } else {
    CORE_DCHECK(direction_ == kSplitNoneRoot, "split direction error");
  }

  if (direction_ == kSplitVertical || direction_ == kSplitHorizontal) {
#if 0
    renderer->SetDrawColor(Skin::current().GetColorScheme().border());
    renderer->DrawFilledRect(GetRectForSplitter());
#endif
  }
}

bool DockingSplitContainer::CouldStartDrag(DragSetup* drag_setup) {
  Point client_position = ScreenToClient(drag_setup->screen_position);
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
}

void DockingSplitContainer::SetFraction(float fraction) {
  fraction_ = fraction;
  // Propagate fraction changes to children.
  SetScreenRect(GetScreenRect());
}

void DockingSplitContainer::ReplaceLeft(Widget* left) {
  CORE_CHECK(direction_ == kSplitNoneRoot && !right_.get(),
             "Can only use on root");
  left_.reset(left);
  left->SetScreenRect(Widget::GetScreenRect());
}

Widget* DockingSplitContainer::FindTopMostUnderPoint(const Point& point) {
  if (!GetScreenRect().Contains(point))
    return NULL;
  Widget* left_contains = left_->FindTopMostUnderPoint(point);
  if (left_contains)
    return left_contains;
  Widget* right_contains = right_->FindTopMostUnderPoint(point);
  if (right_contains)
    return right_contains;
  CORE_NOTREACHED();
  return NULL;
}
