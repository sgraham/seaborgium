// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SG_UI_DOCKING_SPLIT_CONTAINER_H_
#define SG_UI_DOCKING_SPLIT_CONTAINER_H_

#include <memory>
#include "ui/dockable.h"

enum DockingSplitDirection {
  kSplitNoneRoot,
  kSplitVertical,
  kSplitHorizontal,
};

class DockingSplitContainer : public Dockable {
 public:
  DockingSplitContainer(DockingSplitDirection direction,
                        Dockable* left, Dockable* right);
  virtual ~DockingSplitContainer();

  static void SetSplitterWidth(int width);
  static int GetSplitterWidth();

  virtual bool IsContainer() const { return true; }

  // Finds left or right in its children, and replaces that child with a new
  // container split in |direction| containing |left| and |right| in that
  // order.
  void SplitChild(DockingSplitDirection direction,
                  Dockable* left,
                  Dockable* right);

  // Both of these remove |child| and replace |this| with the sibling of
  // |child|. Delete cleans the child up, Release gives ownership to the
  // caller.
  void DeleteChild(Dockable* child);
  Dockable* ReleaseChild(Dockable* child);

  // Replaces either left or right child matching |target|, with |with|.
  void Replace(Dockable* target, Dockable* with);

  Dockable* GetSiblingOf(Dockable* child);

  void SetScreenRect(const Rect& rect) override;
  void Render(Renderer* renderer) override;
  bool CouldStartDrag(DragSetup* drag_setup) override;

  DockingSplitDirection direction() const { return direction_; }
  double fraction() const { return fraction_; }
  Dockable* left() { return left_.get(); }
  Dockable* right() { return right_.get(); }

  void SetFraction(double fraction);

  // Hokey method only used for DockingWorkspace that uses left as root.
  void ReplaceLeft(Dockable* left);

  virtual Dockable* FindTopMostUnderPoint(const Point& point) override;

 private:
  Rect GetRectForSplitter();

  DockingSplitDirection direction_;
  double fraction_;

  // Named left/right for simplicity, but "left" is also "top" if the split is
  // horizontal rather than vertical.
  std::unique_ptr<Dockable> left_;
  std::unique_ptr<Dockable> right_;
};

#endif  // SG_UI_DOCKING_SPLIT_CONTAINER_H_
