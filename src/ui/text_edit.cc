// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/text_edit.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "core/entry.h"
#include "core/gfx.h"
#include "nanovg.h"
#include "ui/skin.h"

#define STB_TEXTEDIT_CHARTYPE char
#define STB_TEXTEDIT_STRING TextControl

#include "../third_party/stb/stb_textedit.h"

struct TextControl {
  char* string;
  int string_len;
  STB_TexteditState state;
};

void LayoutFunc(StbTexteditRow* row, STB_TEXTEDIT_STRING* str, int start_i) {
  int remaining_chars = str->string_len - start_i;
  // Always single line.
  row->num_chars = remaining_chars;
  row->x0 = 0;
  row->x1 = static_cast<float>(remaining_chars);
  row->baseline_y_delta = 1.25;
  row->ymin = -1;
  row->ymax = 0;
}

int DeleteChars(STB_TEXTEDIT_STRING* str, int pos, int num) {
  memmove(&str->string[pos],
          &str->string[pos + num],
          str->string_len - pos - num);
  str->string_len -= num;
  return 1;
}

int InsertChars(STB_TEXTEDIT_STRING* str,
                int pos,
                STB_TEXTEDIT_CHARTYPE* newtext,
                int num) {
  str->string = static_cast<char*>(realloc(str->string, str->string_len + num));
  memmove(&str->string[pos + num], &str->string[pos], str->string_len - pos);
  memcpy(&str->string[pos], newtext, num);
  str->string_len += num;
  return 1;
}

// Define all the #defines needed for stb_textedit's implementation.

#define KEYDOWN_BIT 0x80000000
#define STB_TEXTEDIT_STRINGLEN(tc) ((tc)->string_len)
#define STB_TEXTEDIT_LAYOUTROW LayoutFunc
#define STB_TEXTEDIT_GETWIDTH(tc, n, i) (1)
#define STB_TEXTEDIT_KEYTOTEXT(key) (((key) & KEYDOWN_BIT) ? 0 : (key))
#define STB_TEXTEDIT_GETCHAR(tc, i) ((tc)->string[i])
#define STB_TEXTEDIT_NEWLINE '\n'
#define STB_TEXTEDIT_IS_SPACE(ch) isspace(ch)
#define STB_TEXTEDIT_DELETECHARS DeleteChars
#define STB_TEXTEDIT_INSERTCHARS InsertChars

#define STB_TEXTEDIT_K_SHIFT 0x40000000
#define STB_TEXTEDIT_K_CONTROL 0x20000000
#define STB_TEXTEDIT_K_LEFT (KEYDOWN_BIT | core::Key::Left)
#define STB_TEXTEDIT_K_RIGHT (KEYDOWN_BIT | core::Key::Right)
#define STB_TEXTEDIT_K_UP (KEYDOWN_BIT | core::Key::Up)
#define STB_TEXTEDIT_K_DOWN (KEYDOWN_BIT | core::Key::Down)
#define STB_TEXTEDIT_K_LINESTART (KEYDOWN_BIT | core::Key::Home)
#define STB_TEXTEDIT_K_LINEEND (KEYDOWN_BIT | core::Key::End)
#define STB_TEXTEDIT_K_TEXTSTART \
  (STB_TEXTEDIT_K_LINESTART | STB_TEXTEDIT_K_CONTROL)
#define STB_TEXTEDIT_K_TEXTEND (STB_TEXTEDIT_K_LINEEND | STB_TEXTEDIT_K_CONTROL)
#define STB_TEXTEDIT_K_DELETE (KEYDOWN_BIT | core::Key::Delete)
#define STB_TEXTEDIT_K_BACKSPACE (KEYDOWN_BIT | core::Key::Backspace)
#define STB_TEXTEDIT_K_UNDO \
  (KEYDOWN_BIT | STB_TEXTEDIT_K_CONTROL | core::Key::KeyZ)
#define STB_TEXTEDIT_K_REDO \
  (KEYDOWN_BIT | STB_TEXTEDIT_K_CONTROL | core::Key::KeyY)
#define STB_TEXTEDIT_K_INSERT (KEYDOWN_BIT | core::Key::Insert)
#define STB_TEXTEDIT_K_WORDLEFT (STB_TEXTEDIT_K_LEFT | STB_TEXTEDIT_K_CONTROL)
#define STB_TEXTEDIT_K_WORDRIGHT (STB_TEXTEDIT_K_RIGHT | STB_TEXTEDIT_K_CONTROL)
#define STB_TEXTEDIT_K_PGUP (KEYDOWN_BIT | core::Key::PageUp)
#define STB_TEXTEDIT_K_PGDOWN (KEYDOWN_BIT | core::Key::PageDown)

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100)  // unreferenced formal parameter
#endif

#define STB_TEXTEDIT_IMPLEMENTATION
#include "../third_party/stb/stb_textedit.h"

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#define LOCAL_state() \
  STB_TexteditState* state = &static_cast<STB_TEXTEDIT_STRING*>(impl_)->state
#define LOCAL_control() \
  STB_TEXTEDIT_STRING* control = static_cast<STB_TEXTEDIT_STRING*>(impl_);

TextEdit::TextEdit() : mouse_x_(-1.f), mouse_y_(-1.f) {
  impl_ = calloc(1, sizeof(STB_TEXTEDIT_STRING));
  LOCAL_state();
  LOCAL_control();
  control->string = static_cast<char*>(malloc(0));
  stb_textedit_initialize_state(state, 1 /*is_single_line*/);
}

TextEdit::~TextEdit() {
  free(impl_);
}

bool TextEdit::NotifyMouseMoved(int x, int y, uint8_t modifiers) {
  mouse_x_ = static_cast<float>(x);
  mouse_y_ = static_cast<float>(y);
  CORE_UNUSED(modifiers);
  // TODO(scottmg): Drag selection.
  CORE_UNUSED(&stb_textedit_drag);
  return true;
}

bool TextEdit::NotifyMouseWheel(int x, int y, float delta, uint8_t modifiers) {
  CORE_UNUSED(x);
  CORE_UNUSED(y);
  CORE_UNUSED(delta);
  CORE_UNUSED(modifiers);
  return true;
}

bool TextEdit::NotifyMouseButton(core::MouseButton::Enum button,
                                 bool down,
                                 uint8_t modifiers) {
  LOCAL_state();
  LOCAL_control();
  if (button == core::MouseButton::Left && down && modifiers == 0)
    stb_textedit_click(control, state, mouse_x_, mouse_y_);
  return true;
}

bool TextEdit::NotifyKey(core::Key::Enum key, bool down, uint8_t modifiers) {
  LOCAL_state();
  LOCAL_control();
  int stb_key = key;
  if (modifiers & core::Modifier::LeftCtrl)
    stb_key |= STB_TEXTEDIT_K_CONTROL;
  if (modifiers & core::Modifier::RightCtrl)
    stb_key |= STB_TEXTEDIT_K_CONTROL;
  if (modifiers & core::Modifier::LeftShift)
    stb_key |= STB_TEXTEDIT_K_SHIFT;
  if (modifiers & core::Modifier::RightShift)
    stb_key |= STB_TEXTEDIT_K_SHIFT;
  if (down)
    stb_key |= KEYDOWN_BIT;
  // TODO(scottmg): Cut, copy, paste shortcuts.
  CORE_UNUSED(&stb_textedit_cut);
  CORE_UNUSED(&stb_textedit_paste);
  // TODO(scottmg): Catch up/down for history in subclass.
  stb_textedit_key(control, state, stb_key);
  return true;
}

void TextEdit::Render() {
  nvgSave(core::VG);
  nvgBeginPath(core::VG);
  const Rect& rect = GetClientRect();
  const ColorScheme& cs = Skin::current().GetColorScheme();
  nvgRect(core::VG,
          static_cast<float>(rect.x),
          static_cast<float>(rect.y),
          static_cast<float>(rect.w),
          static_cast<float>(rect.h));
  nvgFillColor(core::VG, cs.background());
  nvgFill(core::VG);

  LOCAL_control();
  nvgFontSize(core::VG, 13.f);  // TODO
  nvgFontFace(core::VG, "mono");
  nvgFillColor(core::VG, cs.text());
  nvgText(core::VG,
          static_cast<float>(rect.x),
          static_cast<float>(rect.y + 16.f),  // TODO
          control->string,
          control->string + control->string_len);
  nvgRestore(core::VG);
}
