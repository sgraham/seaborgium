// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_DOCKING_RESIZER_H_
#define UI_DOCKING_RESIZER_H_

#include "core/core.h"
#include "core/geometric_types.h"
#include "ui/draggable.h"

class DockingSplitContainer;

class DockingResizer : public Draggable {
 public:
  explicit DockingResizer(DockingSplitContainer* resizing);
  virtual ~DockingResizer();

  virtual void Drag(const Point& screen_point) override;
  virtual void CancelDrag() override;
  virtual void Render() override;

  const Point& GetInitialLocationForTest() const { return initial_location_; }

 private:
  int ParentSize();
  int ComponentForDirection(const Point& point);

  DockingSplitContainer* resizing_;
  // Relative to parent.
  Point initial_location_;
};

#endif  // UI_DOCKING_RESIZER_H_
