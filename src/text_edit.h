// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TEXT_EDIT_H_
#define TEXT_EDIT_H_

#include <string>

#include "gfx.h"
#include "widget.h"

// Single line text edit control.
class TextEdit : public Widget {
 public:
  TextEdit();
  virtual ~TextEdit();

  // Implementation of InputHandler:
  virtual bool WantMouseEvents() override { return true; }
  virtual bool WantKeyEvents() override { return true; }

  virtual bool NotifyMouseMoved(int x, int y, uint8_t modifiers) override;
  virtual bool NotifyMouseWheel(int x, int y, float delta, uint8_t modifiers)
      override;
  virtual bool NotifyMouseButton(int x,
                                 int y,
                                 MouseButton::Enum button,
                                 bool down,
                                 uint8_t modifiers) override;
  virtual bool NotifyKey(Key::Enum key,
                         bool down,
                         uint8_t modifiers) override;
  virtual bool NotifyChar(int character) override;

  void SetText(const std::string& value);

  // Implementation of Widget:
  void Render() override;

 private:
  friend struct ScopedCursorAlphaReset;

  void* impl_;
  float mouse_x_;
  float mouse_y_;
  Color cursor_color_;
  Color cursor_color_target_;
  bool left_mouse_is_down_;
  float line_height_;

  CORE_DISALLOW_COPY_AND_ASSIGN(TextEdit);
};

#endif  // TEXT_EDIT_H_
