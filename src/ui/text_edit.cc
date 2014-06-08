// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/text_edit.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <algorithm>

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

struct ScopedTextSetup {
  ScopedTextSetup() {
    nvgSave(core::VG);
    nvgFontSize(core::VG, 14.f);  // TODO
    nvgFontFace(core::VG, "mono");
  }
  ~ScopedTextSetup() {
    nvgRestore(core::VG);
  }
};

float CursorXFromIndex(NVGglyphPosition* positions, int count, int index) {
  CORE_CHECK(index <= count, "index out of range");
  if (index == 0)
    return 0.f;
  else if (index == count)
    return positions[index - 1].maxx;
  else
    return positions[index].minx;
}

void LayoutFunc(StbTexteditRow* row, STB_TEXTEDIT_STRING* str, int start_i) {
  ScopedTextSetup text_setup;

  int remaining_chars = str->string_len - start_i;
  // Always single line.
  row->num_chars = remaining_chars;
  std::unique_ptr<NVGglyphPosition[]> positions(
      new NVGglyphPosition[str->string_len]);
  nvgTextGlyphPositions(core::VG,
                        0,
                        0,
                        str->string,
                        str->string + str->string_len,
                        positions.get(),
                        str->string_len);
  float ascender, descender, line_height;
  nvgTextMetrics(core::VG, &ascender, &descender, &line_height);
  row->x0 = 0.f;  // TODO(scottmg): This seems suspect.
  row->x1 = positions[str->string_len - 1].maxx;
  row->baseline_y_delta = line_height;
  row->ymin = 0;
  row->ymax = line_height - descender;
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
  ScopedTextSetup text_setup;
  CORE_UNUSED(n);  // Single line only.
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
}


// Define all the #defines needed for stb_textedit's implementation.

#define KEYDOWN_BIT 0x80000000
#define STB_TEXTEDIT_STRINGLEN(tc) ((tc)->string_len)
#define STB_TEXTEDIT_LAYOUTROW LayoutFunc
#define STB_TEXTEDIT_GETWIDTH(tc, n, i) GetWidth(tc, n, i)
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

TextEdit::TextEdit()
    : mouse_x_(-1.f),
      mouse_y_(-1.f) {
  const ColorScheme& cs = Skin::current().GetColorScheme();
  cursor_color_ = cursor_color_target_ = cs.cursor();
  cursor_color_.a = 255.f;
  cursor_color_target_.a = 0.f;
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
  if (GetScreenRect().Contains(Point(x, y)))
    core::SetMouseCursor(core::MouseCursor::IBeam);
  return true;
}

bool TextEdit::NotifyMouseWheel(int x, int y, float delta, uint8_t modifiers) {
  CORE_UNUSED(x);
  CORE_UNUSED(y);
  CORE_UNUSED(delta);
  CORE_UNUSED(modifiers);
  return true;
}

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

bool TextEdit::NotifyMouseButton(core::MouseButton::Enum button,
                                 bool down,
                                 uint8_t modifiers) {
  ScopedCursorAlphaReset reset(this);
  LOCAL_state();
  LOCAL_control();
  if (button == core::MouseButton::Left && down && modifiers == 0)
    stb_textedit_click(control, state, mouse_x_ - X(), mouse_y_ - Y());
  return true;
}

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

void TextEdit::Render() {
  ScopedTextSetup text_setup;

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

  LOCAL_state();
  LOCAL_control();

  float ascender, descender, line_height;
  nvgTextMetrics(core::VG, &ascender, &descender, &line_height);
  nvgFillColor(core::VG, cs.text());
  nvgText(core::VG,
          static_cast<float>(rect.x),
          static_cast<float>(rect.y + line_height),
          control->string,
          control->string + control->string_len);

  nvgBeginPath(core::VG);
  // TODO(scottmg): Frame rate.
  cursor_color_ = nvgLerpRGBA(cursor_color_, cursor_color_target_, 0.3f);
  if (fabsf(cursor_color_.a - cursor_color_target_.a) < 0.0001f) {
    if (cursor_color_target_.a == 0.f)
      cursor_color_target_.a = 1.f;
    else
      cursor_color_target_.a = 0.f;
  }
  nvgFillColor(core::VG, cursor_color_);
  std::unique_ptr<NVGglyphPosition[]> positions(
      new NVGglyphPosition[control->string_len]);
  nvgTextGlyphPositions(core::VG,
                        static_cast<float>(rect.x),
                        static_cast<float>(rect.y),
                        control->string,
                        control->string + control->string_len,
                        positions.get(),
                        control->string_len);

  float cursor_x =
      CursorXFromIndex(positions.get(), control->string_len, state->cursor);

  nvgRect(core::VG,
          cursor_x,
          static_cast<float>(rect.y),
          1.5f,
          line_height - descender);
  nvgFill(core::VG);

  if (state->select_start != state->select_end) {
    nvgBeginPath(core::VG);
    nvgFillColor(core::VG, cs.text_selection());
    int start = std::min(state->select_start, state->select_end);
    int end = std::max(state->select_start, state->select_end);
    float select_x =
        CursorXFromIndex(positions.get(), control->string_len, start);
    float select_w =
        CursorXFromIndex(positions.get(), control->string_len, end) - select_x;
    nvgRoundedRect(core::VG,
        select_x,
        static_cast<float>(rect.y),
        select_w,
        static_cast<float>(rect.y + line_height - descender),
        3.f);
    nvgFill(core::VG);
  }
}
