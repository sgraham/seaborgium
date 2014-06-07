// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Derived from bgfx/examples/common/entry:
// Copyright 2011-2014 Branimir Karadzic. All rights reserved.
// License: http://www.opensource.org/licenses/BSD-2-Clause

#ifndef CORE_ENTRY_H_
#define CORE_ENTRY_H_

#include "core/core.h"

namespace core {

struct MouseButton {
  enum Enum { None, Left, Middle, Right, Count };
};

struct Modifier {
  enum Enum {
    None = 0,
    LeftAlt = 0x01,
    RightAlt = 0x02,
    LeftCtrl = 0x04,
    RightCtrl = 0x08,
    LeftShift = 0x10,
    RightShift = 0x20,
    LeftMeta = 0x40,
    RightMeta = 0x80,
  };
};

struct Key {
  enum Enum {
    None = 0,
    Esc,
    Return,
    Tab,
    Space,
    Backspace,
    Up,
    Down,
    Left,
    Right,
    PageUp,
    PageDown,
    Home,
    End,
    Insert,
    Delete,
    Print,
    Plus,
    Minus,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    NumPad0,
    NumPad1,
    NumPad2,
    NumPad3,
    NumPad4,
    NumPad5,
    NumPad6,
    NumPad7,
    NumPad8,
    NumPad9,
    Key0,
    Key1,
    Key2,
    Key3,
    Key4,
    Key5,
    Key6,
    Key7,
    Key8,
    Key9,
    KeyA,
    KeyB,
    KeyC,
    KeyD,
    KeyE,
    KeyF,
    KeyG,
    KeyH,
    KeyI,
    KeyJ,
    KeyK,
    KeyL,
    KeyM,
    KeyN,
    KeyO,
    KeyP,
    KeyQ,
    KeyR,
    KeyS,
    KeyT,
    KeyU,
    KeyV,
    KeyW,
    KeyX,
    KeyY,
    KeyZ,
  };
};

class InputHandler {
 public:
  virtual ~InputHandler() {}

  virtual bool WantMouseEvents() = 0;
  virtual bool WantKeyEvents() = 0;

  virtual bool NotifyMouseMoved(int x,
                                int y,
                                uint8_t modifiers) = 0;
  virtual bool NotifyMouseWheel(int x,
                                int y,
                                float delta,
                                uint8_t modifiers) = 0;
  virtual bool NotifyMouseButton(MouseButton::Enum button,
                                 bool down,
                                 uint8_t modifiers) = 0;
  virtual bool NotifyKey(Key::Enum key, bool down, uint8_t modifiers) = 0;
};

bool ProcessEvents(uint32_t* width,
                   uint32_t* height,
                   InputHandler* input_handler);

float GetDpiScale();

void SetWindowSize(uint32_t width, uint32_t height);

struct MouseCursor {
  enum Enum {
    Default,
    DragLeftRight,
    DragUpDown,
    DragAll,
    Pointer,
  };
};

void SetMouseCursor(MouseCursor::Enum cursor);

}  // namespace core

#endif  // CORE_ENTRY_H_
