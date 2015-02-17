// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DOCKING_RESIZER_H_
#define DOCKING_RESIZER_H_

#include "core.h"
#include "draggable.h"
#include "geometric_types.h"

class DockingSplitContainer;

class DockingResizer : public Draggable {
 public:
  explicit DockingResizer(DockingSplitContainer* resizing);
  ~DockingResizer() override;

  void Drag(const Point& screen_point) override;
  void CancelDrag() override;
  void Render() override;

  const Point& GetInitialLocationForTest() const { return initial_location_; }

 private:
  float ParentSize();
  float ComponentForDirection(const Point& point);

  DockingSplitContainer* resizing_;
  // Relative to parent.
  Point initial_location_;
};

#endif  // DOCKING_RESIZER_H_
