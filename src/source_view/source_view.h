// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SOURCE_VIEW_SOURCE_VIEW_H_
#define SOURCE_VIEW_SOURCE_VIEW_H_

#include <string>

#include "core.h"
#include "gfx.h"
#include "scroll_helper.h"
#include "source_view/lexer.h"
#include "widget.h"

struct ColoredText {
  Lexer::TokenType type;
  std::string text;
};
typedef std::vector<ColoredText> Line;

class SourceView : public Widget, public ScrollHelperDataProvider {
 public:
  SourceView();
  ~SourceView() override;

  void SetFilePath(const std::string& path);

  // Implementation of InputHandler:
  bool WantMouseEvents() override { return true; }
  bool WantKeyEvents() override { return true; }
  bool NotifyMouseWheel(int x, int y, float delta, uint8_t modifiers) override;
  bool NotifyKey(Key::Enum key, bool down, uint8_t modifiers) override;

  // Widget:
  void Render() override;

  // ScrollHelperDataProvider:
  int GetContentSize() override;
  const Rect& GetScreenRect() const override;

 private:
  int GetFirstLineInView();
  bool LineInView(int line_number);
  const Color& ColorForTokenType(const Skin& skin, Lexer::TokenType type);

  ScrollHelper scroll_;
  std::vector<Line> lines_;

  DISALLOW_COPY_AND_ASSIGN(SourceView);
};

#endif  // SOURCE_VIEW_SOURCE_VIEW_H_
