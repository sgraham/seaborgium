// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DRAGGABLE_H_
#define DRAGGABLE_H_

#include "geometric_types.h"

class Draggable {
 public:
  virtual ~Draggable() {}

  virtual void Drag(const Point& screen_position) = 0;
  virtual void CancelDrag() = 0;
  virtual void Render() = 0;
};

#endif  // DRAGGABLE_H_
