// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_TEXT_EDIT_H_
#define UI_TEXT_EDIT_H_

#include "nanovg.h"
#include "ui/dockable.h"

// Single line text edit control.
class TextEdit : public Dockable {
 public:
  TextEdit();
  virtual ~TextEdit();

  // Implementation of core::InputHandler:
  virtual bool WantMouseEvents() override { return true; }
  virtual bool WantKeyEvents() override { return true; }

  virtual bool NotifyMouseMoved(int x, int y, uint8_t modifiers) override;
  virtual bool NotifyMouseWheel(int x, int y, float delta, uint8_t modifiers)
      override;
  virtual bool NotifyMouseButton(int x,
                                 int y,
                                 core::MouseButton::Enum button,
                                 bool down,
                                 uint8_t modifiers) override;
  virtual bool NotifyKey(core::Key::Enum key,
                         bool down,
                         uint8_t modifiers) override;
  virtual bool NotifyChar(int character) override;

  // Implementation of Dockable:
  void Render() override;

 private:
  friend struct ScopedCursorAlphaReset;

  void* impl_;
  float mouse_x_;
  float mouse_y_;
  NVGcolor cursor_color_;
  NVGcolor cursor_color_target_;
  bool left_mouse_is_down_;

  CORE_DISALLOW_COPY_AND_ASSIGN(TextEdit);
};

#endif  // UI_TEXT_EDIT_H_
