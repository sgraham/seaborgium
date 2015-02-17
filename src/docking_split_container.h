// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DOCKING_SPLIT_CONTAINER_H_
#define DOCKING_SPLIT_CONTAINER_H_

#include <memory>
#include "widget.h"

enum DockingSplitDirection {
  kSplitNoneRoot,
  kSplitVertical,
  kSplitHorizontal,
};

class DockingSplitContainer : public Widget {
 public:
  DockingSplitContainer(DockingSplitDirection direction,
                        Widget* left, Widget* right);
  virtual ~DockingSplitContainer();

  static void SetSplitterWidth(float width);
  static float GetSplitterWidth();

  virtual bool IsDockingSplitContainer() const override { return true; }

  // Finds left or right in its children, and replaces that child with a new
  // container split in |direction| containing |left| and |right| in that
  // order.
  void SplitChild(DockingSplitDirection direction,
                  Widget* left,
                  Widget* right);

  // Both of these remove |child| and replace |this| with the sibling of
  // |child|. Delete cleans the child up, Release gives ownership to the
  // caller.
  void DeleteChild(Widget* child);
  Widget* ReleaseChild(Widget* child);

  // Replaces either left or right child matching |target|, with |with|.
  void Replace(Widget* target, Widget* with);

  Widget* GetSiblingOf(Widget* child);

  void SetScreenRect(const Rect& rect) override;
  void Render() override;
  bool CouldStartDrag(DragSetup* drag_setup) override;

  DockingSplitDirection direction() const { return direction_; }
  float fraction() const { return fraction_; }
  Widget* left() { return left_.get(); }
  Widget* right() { return right_.get(); }

  void SetFraction(float fraction);

  // Hokey method only used for DockingWorkspace that uses left as root.
  void ReplaceLeft(Widget* left);

  virtual Widget* FindTopMostUnderPoint(const Point& point) override;

 private:
  Rect GetRectForSplitter();

  DockingSplitDirection direction_;
  float fraction_;

  // Named left/right for simplicity, but "left" is also "top" if the split is
  // horizontal rather than vertical.
  std::unique_ptr<Widget> left_;
  std::unique_ptr<Widget> right_;
};

#endif  // DOCKING_SPLIT_CONTAINER_H_
