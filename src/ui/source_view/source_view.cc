// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/source_view/source_view.h"

#include "ui/drawing_common.h"
#include "ui/source_view/cpp_lexer.h"
#include "ui/skin.h"

SourceView::SourceView() : scroll_(this, Skin::current().text_line_height()) {
}

SourceView::~SourceView() {
}

// TODO(scottmg): Losing last line if doesn't end in \n.
void SyntaxHighlight(const std::string& input, std::vector<Line>* lines) {
  std::unique_ptr<Lexer> lexer(MakeCppLexer());
  std::vector<Token> tokens;
  lexer->GetTokensUnprocessed(input, &tokens);
  Line current_line;
  for (size_t i = 0; i < tokens.size(); ++i) {
    const Token& token = tokens[i];
    if (token.value == "\n") {
      lines->push_back(current_line);
      current_line.clear();
    } else {
      ColoredText fragment;
      fragment.type = token.token;
      fragment.text = token.value;
      current_line.push_back(fragment);
      if (token.token == Lexer::CommentSingle) {
        // Includes \n in its value so we don't otherwise see it.
        lines->push_back(current_line);
        current_line.clear();
      }
    }
  }
}

void SourceView::SetFilePath(const std::string& path) {
  // TODO(scottmg): On background thread.
  FILE* f = fopen(path.c_str(), "rb");
  if (!f) return;
  fseek(f, 0, SEEK_END);
  long len = ftell(f);
  std::unique_ptr<char[]> file_contents(new char[len]);
  fseek(f, 0, SEEK_SET);
  fread(file_contents.get(), 1, len, f);
  fclose(f);
  SyntaxHighlight(std::string(file_contents.get(), len), &lines_);
}

bool SourceView::NotifyMouseWheel(int x,
                                  int y,
                                  float delta,
                                  uint8_t modifiers) {
  CORE_UNUSED(x);
  CORE_UNUSED(y);
  bool invalidate = false;
  bool handled = false;
  scroll_.CommonMouseWheel(delta, modifiers, &invalidate, &handled);
  return handled;
}

bool SourceView::NotifyKey(core::Key::Enum key, bool down, uint8_t modifiers) {
  bool invalidate = false;
  bool handled = false;
  scroll_.CommonNotifyKey(key, down, modifiers, &invalidate, &handled);
  return handled;
}

void SourceView::Render() {
  scroll_.Update();
  const Skin& skin = Skin::current();
  const ColorScheme& cs = skin.GetColorScheme();
  DrawSolidRect(GetClientRect(), cs.background());

#if 0
  nvgFontSize(core::VG, 14.0f);
  nvgFontFace(core::VG, "mono");
#if 0
  {
    ScopedRenderOffset scroll_offset(0,
                                     static_cast<float>(scroll_.GetOffset()));
    nvgFillColor(core::VG, Skin::current().GetColorScheme().text());
    nvgText(core::VG, 100, 200, "int main(int argc, char** argv) {", NULL);
  }
#endif

  int line_height = static_cast<int>(skin.text_line_height());
  int start_line = GetFirstLineInView();

#if 0  // TODO(scottmg): Margin.
  // Not quite right, but probably close enough.
  int largest_numbers_width = renderer->MeasureText(
      skin.mono_font(),
      base::IntToString16(lines_.size()).c_str()).x;
  static const int left_margin = 5;
  static const int right_margin = 10;
  static const int indicator_width = line_height;
  static const int indicator_height = line_height;
  static const int indicator_and_margin = indicator_width + 5;
  renderer->SetDrawColor(skin.GetColorScheme().margin());
  renderer->DrawFilledRect(Rect(
      0, 0,
      left_margin + largest_numbers_width + right_margin + indicator_and_margin,
      Height()));
#endif

  int y_pixel_scroll = scroll_.GetOffset() - line_height;

  for (size_t i = start_line; i < lines_.size(); ++i) {
    // Extra |line_height| added to height so that a full line is drawn at
    // the bottom when partial-line pixel scrolled.
    if (!LineInView(i))
      break;

    // Line numbers.
#if 0
    renderer->SetDrawColor(skin.GetColorScheme().margin_text());
    renderer->RenderText(
        skin.mono_font(),
        Point(left_margin, i * line_height - y_pixel_scroll),
        base::IntToString16(i + 1).c_str());
    size_t x = left_margin + largest_numbers_width + right_margin +
               indicator_and_margin;
#endif
    size_t x = 5;

    // Source.
    for (size_t j = 0; j < lines_[i].size(); ++j) {
      nvgFillColor(core::VG, ColorForTokenType(skin, lines_[i][j].type));
      x = static_cast<size_t>(
          nvgText(core::VG,
                  static_cast<float>(x),
                  static_cast<float>(i * line_height - y_pixel_scroll),
                  lines_[i][j].text.c_str(),
                  nullptr));
    }
  }

#if 0
  if (LineInView(program_counter_line_)) {
    int y = program_counter_line_ * line_height - y_pixel_scroll;
    renderer->SetDrawColor(skin.GetColorScheme().pc_indicator());
    renderer->DrawTexturedRect(
        skin.pc_indicator_texture(),
        Rect(left_margin + largest_numbers_width + right_margin, y,
                   indicator_width, indicator_height),
        0, 0, 1, 1);
  }
#endif
#endif

  scroll_.RenderScrollIndicators();
}

int SourceView::GetContentSize() {
  return static_cast<int>(Skin::current().text_line_height() * lines_.size());
}

const Rect& SourceView::GetScreenRect() const {
  return Widget::GetScreenRect();
}

int SourceView::GetFirstLineInView() {
  return static_cast<int>(scroll_.GetOffset() /
                          Skin::current().text_line_height());
}

bool SourceView::LineInView(int line_number) {
  int start_line = GetFirstLineInView();
  int line_height = static_cast<int>(Skin::current().text_line_height());
  if (line_number < start_line)
    return false;
  if ((line_number - start_line) * line_height > Height() + line_height)
    return false;
  return true;
}

const core::Color& SourceView::ColorForTokenType(const Skin& skin,
                                                 Lexer::TokenType type) {
  // TODO(config): More customizability with fallbacks.
  const ColorScheme& cs = skin.GetColorScheme();
  switch (type) {
    case Lexer::Comment:
    case Lexer::CommentMultiline:
    case Lexer::CommentSingle:
      return cs.comment();
    case Lexer::CommentPreproc:
      return cs.comment_preprocessor();
    case Lexer::Error:
      return cs.error();
    case Lexer::Keyword:
    case Lexer::KeywordConstant:
    case Lexer::KeywordPseudo:
    case Lexer::KeywordReserved:
      return cs.keyword();
    case Lexer::KeywordType:
      return cs.keyword_type();
    case Lexer::LiteralNumberFloat:
    case Lexer::LiteralNumberHex:
    case Lexer::LiteralNumberInteger:
    case Lexer::LiteralNumberOct:
      return cs.literal_number();
    case Lexer::LiteralString:
    case Lexer::LiteralStringChar:
    case Lexer::LiteralStringEscape:
      return cs.literal_string();
    case Lexer::Name:
    case Lexer::NameBuiltin:
    case Lexer::NameLabel:
      break;
    case Lexer::NameClass:
      return cs.klass();
    case Lexer::Operator:
      return cs.op();
    case Lexer::Punctuation:
    case Lexer::Text:
    default:
      break;
  }
  return cs.text();
}
