// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SG_UI_INPUT_H_
#define SG_UI_INPUT_H_

enum InputKey {
  kNone = -1,
  kShift,
  kReturn,
  kBackspace,
  kDelete,
  kTab,
  kSpace,
  kHome,
  kEnd,
  kControl,
  kUp,
  kDown,
  kLeft,
  kRight,
  kPageUp,
  kPageDown,
  kF1,
  kF2,
  kF3,
  kF4,
  kF5,
  kF6,
  kF7,
  kF8,
  kF9,
  kF10,
  kF11,
  kF12,
};

class InputModifiers {
 public:
  InputModifiers(bool l_control, bool r_control,
                 bool l_shift, bool r_shift,
                 bool l_alt, bool r_alt)
      : l_control_(l_control),
        r_control_(r_control),
        l_shift_(l_shift),
        r_shift_(r_shift),
        l_alt_(l_alt),
        r_alt_(r_alt) {
  }

  bool ControlPressed() const { return l_control_ || r_control_; }
  bool ShiftPressed() const { return l_shift_ || r_shift_; }
  bool AltPressed() const { return l_alt_ || r_alt_; }
  bool None() const {
    return !l_alt_ && !l_control_ && !l_shift_ &&
           !r_alt_ && !r_control_ && !r_shift_;
  }

 private:
  bool l_control_;
  bool r_control_;
  bool l_shift_;
  bool r_shift_;
  bool l_alt_;
  bool r_alt_;
};

class InputHandler {
 public:
  virtual ~InputHandler() {}

  virtual bool WantMouseEvents() = 0;
  virtual bool WantKeyEvents() = 0;

  virtual bool NotifyMouseMoved(
      int x, int y, int dx, int dy, const InputModifiers& modifiers) = 0;
  virtual bool NotifyMouseWheel(int delta, const InputModifiers& modifiers) = 0;
  virtual bool NotifyMouseButton(
      int index, bool down, const InputModifiers& modifiers) = 0;
  virtual bool NotifyKey(
      InputKey key, bool down, const InputModifiers& modifiers) = 0;
};

#endif  // SG_UI_INPUT_H_
