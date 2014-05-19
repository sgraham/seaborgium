// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_DRAGGABLE_H_
#define UI_DRAGGABLE_H_

class Renderer;

class Draggable {
 public:
  virtual ~Draggable() {}

  virtual void Drag(const Point& screen_position) = 0;
  virtual void CancelDrag() = 0;
  virtual void Render(Renderer* renderer) = 0;
};

#endif  // UI_DRAGGABLE_H_
