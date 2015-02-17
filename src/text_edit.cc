// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "text_edit.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <algorithm>

#include "entry.h"
#include "focus.h"
#include "gfx.h"
#include "skin.h"
#include "string_piece.h"

#define STB_TEXTEDIT_CHARTYPE char
#define STB_TEXTEDIT_STRING TextControl

#include "../third_party/stb/stb_textedit.h"

struct TextControl {
  char* string;
  int string_len;
  STB_TexteditState state;
};

float CursorXFromIndex(const core::TextMeasurements& tm, int count, int index) {
  CORE_CHECK(index <= count, "index out of range");
  if (index == 0)
    return 0.f;
  bool trailing = index == count;
  float x, y;
  tm.GetCaretPosition(index, trailing, &x, &y);
  return x;
}

void LayoutFunc(StbTexteditRow* row, STB_TEXTEDIT_STRING* str, int start_i) {
  int remaining_chars = str->string_len - start_i;
  // Always single line.
  row->num_chars = remaining_chars;
  core::TextMeasurements tm = core::GfxMeasureText(
      core::Font::kMono, StringPiece(str->string, str->string_len));
  row->x0 = 0.f;  // TODO(scottmg): This seems suspect.
  row->x1 = tm.width;
  row->baseline_y_delta = tm.line_height;
  row->ymin = 0;
  row->ymax = tm.height;
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

float GetWidth(STB_TEXTEDIT_STRING* str, int n, int i) {
  CORE_UNUSED(n);  // Single line only.
  (void)str;
  (void)n;
  (void)i;
  // XXX !
  return 10.f;
#if 0
  std::unique_ptr<NVGglyphPosition[]> positions(
      new NVGglyphPosition[str->string_len]);
  nvgTextGlyphPositions(core::VG,
                        0,
                        0,
                        str->string,
                        str->string + str->string_len,
                        positions.get(),
                        str->string_len);
  CORE_CHECK(i < str->string_len, "out of bounds request");
  return CursorXFromIndex(positions.get(), str->string_len, i + 1) -
         CursorXFromIndex(positions.get(), str->string_len, i);
#endif
}


// Define all the #defines needed for stb_textedit's implementation.

#define KEYDOWN_BIT 0x40000000
#define STB_TEXTEDIT_STRINGLEN(tc) ((tc)->string_len)
#define STB_TEXTEDIT_LAYOUTROW LayoutFunc
#define STB_TEXTEDIT_GETWIDTH(tc, n, i) GetWidth(tc, n, i)
#define STB_TEXTEDIT_KEYTOTEXT(key) (((key) & KEYDOWN_BIT) ? 0 : (key))
#define STB_TEXTEDIT_GETCHAR(tc, i) ((tc)->string[i])
#define STB_TEXTEDIT_NEWLINE '\n'
#define STB_TEXTEDIT_IS_SPACE(ch) isspace(ch)
#define STB_TEXTEDIT_DELETECHARS DeleteChars
#define STB_TEXTEDIT_INSERTCHARS InsertChars

#define STB_TEXTEDIT_K_SHIFT 0x20000000
#define STB_TEXTEDIT_K_CONTROL 0x10000000
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
#include "../third_party/stb/stb_textedit.h"  // NOLINT(build/include)

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#define LOCAL_state() \
  STB_TexteditState* state = &static_cast<STB_TEXTEDIT_STRING*>(impl_)->state
#define LOCAL_control() \
  STB_TEXTEDIT_STRING* control = static_cast<STB_TEXTEDIT_STRING*>(impl_);

// Reset the cursor to fully visible, but only if it moves during the scope.
struct ScopedCursorAlphaReset {
  ScopedCursorAlphaReset(TextEdit* parent) : parent_(parent) {
    STB_TexteditState* state =
        &static_cast<STB_TEXTEDIT_STRING*>(parent_->impl_)->state;
    cursor_orig_ = state->cursor;
  }

  ~ScopedCursorAlphaReset() {
    STB_TexteditState* state =
        &static_cast<STB_TEXTEDIT_STRING*>(parent_->impl_)->state;
    if (cursor_orig_ != state->cursor) {
      parent_->cursor_color_.a = 1.f;
      parent_->cursor_color_target_.a = 0.f;
    }
  }

  TextEdit* parent_;
  int cursor_orig_;
};

TextEdit::TextEdit()
    : mouse_x_(-1.f), mouse_y_(-1.f), left_mouse_is_down_(false) {
  const ColorScheme& cs = Skin::current().GetColorScheme();
  cursor_color_ = cursor_color_target_ = cs.cursor();
  cursor_color_.a = 255.f;
  cursor_color_target_.a = 0.f;
  impl_ = calloc(1, sizeof(STB_TEXTEDIT_STRING));
  LOCAL_state();
  LOCAL_control();
  control->string = static_cast<char*>(malloc(0));
  stb_textedit_initialize_state(state, 1 /*is_single_line*/);

  line_height_ = GfxMeasureText(core::Font::kMono, "X").line_height;
}

TextEdit::~TextEdit() {
  free(impl_);
}

bool TextEdit::NotifyMouseMoved(int x, int y, uint8_t modifiers) {
  mouse_x_ = static_cast<float>(x);
  mouse_y_ = static_cast<float>(y);
  CORE_UNUSED(modifiers);
  LOCAL_state();
  LOCAL_control();
  if (GetScreenRect().Contains(Point(mouse_x_, mouse_y_)))
    core::SetMouseCursor(core::MouseCursor::IBeam);
  if (left_mouse_is_down_)
    stb_textedit_drag(control, state, mouse_x_ - X(), mouse_y_ - Y());
  return true;
}

bool TextEdit::NotifyMouseWheel(int x, int y, float delta, uint8_t modifiers) {
  CORE_UNUSED(x);
  CORE_UNUSED(y);
  CORE_UNUSED(delta);
  CORE_UNUSED(modifiers);
  return true;
}

bool TextEdit::NotifyMouseButton(int x,
                                 int y,
                                 core::MouseButton::Enum button,
                                 bool down,
                                 uint8_t modifiers) {
  CORE_UNUSED(x);
  CORE_UNUSED(y);
  ScopedCursorAlphaReset reset(this);
  LOCAL_state();
  LOCAL_control();
  if (button == core::MouseButton::Left && down && modifiers == 0)
    stb_textedit_click(control, state, mouse_x_ - X(), mouse_y_ - Y());
  if (button == core::MouseButton::Left)
    left_mouse_is_down_ = down;
  return true;
}

bool TextEdit::NotifyKey(core::Key::Enum key, bool down, uint8_t modifiers) {
  // We use NotifyChar for regular characters to attempt to get some semblance
  // of support for VK->character mapping from the host OS.
  if (key == core::Key::None || key > core::Key::LAST_NON_PRINTABLE || !down)
    return false;
  ScopedCursorAlphaReset reset(this);
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
  // TODO(scottmg): Catch others in subclass (up/down for history, etc.)
  stb_textedit_key(control, state, stb_key);
  return true;
}

bool TextEdit::NotifyChar(int character) {
  ScopedCursorAlphaReset reset(this);
  if (!isprint(character))
    return false;
  LOCAL_state();
  LOCAL_control();
  stb_textedit_key(control, state, character);
  return true;
}

void TextEdit::SetText(const std::string& value) {
  LOCAL_state();
  LOCAL_control();
  control->string = static_cast<char*>(realloc(control->string, value.size()));
  memcpy(control->string, value.data(), value.size());
  control->string_len = static_cast<int>(value.size());
  state->cursor = 0;
  state->select_start = 0;
  state->select_end = 0;
}

void TextEdit::Render() {
  const ColorScheme& cs = Skin::current().GetColorScheme();
  const Rect& rect = GetClientRect();
  DrawSolidRect(rect, cs.background());

  LOCAL_state();
  LOCAL_control();

  StringPiece str(control->string, control->string_len);
  core::GfxText(core::Font::kMono, cs.text(), rect.x, rect.y, str);

  core::TextMeasurements tm = GfxMeasureText(core::Font::kMono, str);

  // Caret.
  if (GetFocusedContents() == this) {
    // TODO(scottmg): Frame rate.
    cursor_color_ = Lerp(cursor_color_, cursor_color_target_, 0.3f);
    if (fabsf(cursor_color_.a - cursor_color_target_.a) < 0.0001f) {
      if (cursor_color_target_.a == 0.f)
        cursor_color_target_.a = 1.f;
      else
        cursor_color_target_.a = 0.f;
    }
    float cursor_x = CursorXFromIndex(tm, control->string_len, state->cursor);

    DrawSolidRect(Rect(cursor_x, rect.y, 1.5f, line_height_),
                  cursor_color_);
  }

  // Selection.
  if (state->select_start != state->select_end) {
    int start = std::min(state->select_start, state->select_end);
    int end = std::max(state->select_start, state->select_end);
    float select_x = CursorXFromIndex(tm, control->string_len, start);
    float select_w = CursorXFromIndex(tm, control->string_len, end) - select_x;
    DrawSolidRoundedRect(
        Rect(select_x, rect.y, select_w, rect.y + line_height_),
        cs.text_selection(),
        3.f);
  }

}
