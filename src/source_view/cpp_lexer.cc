// // Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Originally generated by generate_cpp_lexer_data.py, but hand-modified
// to be slightly worse, but work with RE2's regex style.

#include "source_view/lexer.h"
#include "source_view/lexer_state.h"

Lexer* MakeCppLexer() {
  Lexer* lexer = new Lexer("C++");

  // States
  LexerState* classname = lexer->AddState("classname");
  LexerState* if0 = lexer->AddState("if0");
  LexerState* macro = lexer->AddState("macro");
  LexerState* root = lexer->AddState("root");
  LexerState* string = lexer->AddState("string");

  // Transitions for classname
  TokenDefinitions classname_defs;
  classname_defs.AddWithTransition(
      "[a-zA-Z_][a-zA-Z0-9_]*", Lexer::NameClass, Lexer::Pop);  // NOLINT
  // TODO(lexer) ?=> : classname_defs.AddWithTransition("\\s*(?=>)",
  // Lexer::Text, Lexer::Pop); // NOLINT
  classname->SetTokenDefinitions(classname_defs);

  // Transitions for if0
  TokenDefinitions if0_defs;
  if0_defs.AddWithTransition(
      "^\\s*#if.*?\\n", Lexer::CommentPreproc, Lexer::Push);  // NOLINT
  if0_defs.AddWithTransition(
      "^\\s*#endif.*?\\n", Lexer::CommentPreproc, Lexer::Pop);  // NOLINT
  if0_defs.Add(".*?\\n", Lexer::Comment);
  if0->SetTokenDefinitions(if0_defs);

  // Transitions for macro
  TokenDefinitions macro_defs;
  macro_defs.Add("[^/\\n]+", Lexer::CommentPreproc);
  macro_defs.Add("/[*](.|\\n)*?[*]/", Lexer::CommentMultiline);
  macro_defs.AddWithTransition("//.*?\\n", Lexer::CommentSingle, Lexer::Pop);
  macro_defs.Add("/", Lexer::CommentPreproc);
  // TODO(lexer) ?<= : macro_defs.Add("(?<=\\\\)\\n", Lexer::CommentPreproc);
  macro_defs.AddWithTransition("\\n", Lexer::CommentPreproc, Lexer::Pop);
  macro->SetTokenDefinitions(macro_defs);

  // Transitions for root
  TokenDefinitions root_defs;
  root_defs.AddWithTransition("^#if\\s+0\\s*", Lexer::CommentPreproc, if0);
  root_defs.AddWithTransition("^#", Lexer::CommentPreproc, macro);
  // TODO(<type 'function'>)
  // root_defs.AddWithTransition("^((?:\\s|//.*?\\n|/[*].*?[*]/)+)(#if\\s+0)",
  // <function callback at 0x000000000237AD68>, if0); // NOLINT
  // TODO(<type 'function'>)
  // root_defs.AddWithTransition("^((?:\\s|//.*?\\n|/[*].*?[*]/)+)(#)",
  // <function callback at 0x000000000237AE48>, macro); // NOLINT
  root_defs.Add("\\n", Lexer::Text);
  root_defs.Add("\\s+", Lexer::Text);
  root_defs.Add("\\\\\\n", Lexer::Text);
  root_defs.Add("/(\\\\\\n)?/(\\n|(.|\\n)*?[^\\\\]\\n)", Lexer::CommentSingle);
  root_defs.Add("/(\\\\\\n)?[*](.|\\n)*?[*](\\\\\\n)?/",
                Lexer::CommentMultiline);  // NOLINT
  root_defs.Add("[{}]", Lexer::Punctuation);
  root_defs.AddWithTransition("L?\"", Lexer::LiteralString, string);
  root_defs.Add(
      "L?\'(\\\\.|\\\\[0-7]{1,3}|\\\\x[a-fA-F0-9]{1,2}|[^\\\\\\\'\\n])\'",
      Lexer::LiteralStringChar);  // NOLINT
  root_defs.Add("(\\d+\\.\\d*|\\.\\d+|\\d+)[eE][+-]?\\d+[LlUu]*",
                Lexer::LiteralNumberFloat);  // NOLINT
  root_defs.Add("(\\d+\\.\\d*|\\.\\d+|\\d+[fF])[fF]?",
                Lexer::LiteralNumberFloat);  // NOLINT
  root_defs.Add("0x[0-9a-fA-F]+[LlUu]*", Lexer::LiteralNumberHex);
  root_defs.Add("0[0-7]+[LlUu]*", Lexer::LiteralNumberOct);
  root_defs.Add("\\d+[LlUu]*", Lexer::LiteralNumberInteger);
  root_defs.Add("\\*/", Lexer::Error);
  root_defs.Add("[~!%^&*+=|?:<>/-]", Lexer::Operator);
  root_defs.Add("[()\\[\\],.;]", Lexer::Punctuation);
  root_defs.Add(
      "(asm|auto|break|case|catch|const|const_cast|continue|default|delete|do|"
      "dynamic_cast|else|enum|explicit|export|extern|for|friend|goto|if|"
      "mutable|namespace|new|operator|private|protected|public|register|"
      "reinterpret_cast|return|restrict|sizeof|static|static_cast|struct|"
      "switch|template|this|throw|throws|try|typedef|typeid|typename|union|"
      "using|volatile|virtual|while)\\b",
      Lexer::Keyword);  // NOLINT
  // TODO(<type 'function'>)   root_defs.AddWithTransition("(class)(\\s+)",
  // <function callback at 0x000000000237AEB8>, classname); // NOLINT
  root_defs.Add(
      "(bool|int|long|float|short|double|char|unsigned|signed|void|wchar_t)\\b",
      Lexer::KeywordType);  // NOLINT
  root_defs.Add("(_{0,2}inline|naked|thread)\\b", Lexer::KeywordReserved);
  root_defs.Add(
      "__(asm|int8|based|except|int16|stdcall|cdecl|fastcall|int32|declspec|"
      "finally|int64|try|leave|wchar_t|w64|virtual_inheritance|uuidof|"
      "unaligned|super|single_inheritance|raise|noop|multiple_inheritance|"
      "m128i|m128d|m128|m64|interface|identifier|forceinline|event|assume)\\b",
      Lexer::KeywordReserved);  // NOLINT
  root_defs.Add("(__offload|__blockingoffload|__outer)\\b",
                Lexer::KeywordPseudo);  // NOLINT
  root_defs.Add("(true|false)\\b", Lexer::KeywordConstant);
  root_defs.Add("NULL\\b", Lexer::NameBuiltin);
  // TODO: ?!: : root_defs.Add("[a-zA-Z_][a-zA-Z0-9_]*:(?!:)",
  // Lexer::NameLabel); // NOLINT
  root_defs.Add("[a-zA-Z_][a-zA-Z0-9_]*", Lexer::Name);
  root->SetTokenDefinitions(root_defs);

  // Transitions for string
  TokenDefinitions string_defs;
  string_defs.AddWithTransition("\"", Lexer::LiteralString, Lexer::Pop);
  string_defs.Add("\\\\([\\\\abfnrtv\"\\\']|x[a-fA-F0-9]{2,4}|[0-7]{1,3})",
                  Lexer::LiteralStringEscape);  // NOLINT
  string_defs.Add("[^\\\\\"\\n]+", Lexer::LiteralString);
  string_defs.Add("\\\\\\n", Lexer::LiteralString);
  string_defs.Add("\\\\", Lexer::LiteralString);
  string->SetTokenDefinitions(string_defs);

  return lexer;
}
