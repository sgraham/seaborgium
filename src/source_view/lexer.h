// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SOURCE_VIEW_LEXER_H_
#define SOURCE_VIEW_LEXER_H_

#include <limits>
#include <map>
#include <string>
#include <vector>

#include "core.h"

class LexerState;
class Token;

// This module (regex, input, parsed tokens) works entirely in utf8, even on
// Windows, because that's what RE2 processes.
//
// Probably this whole thing should be something more like re2c, but this is
// OK for now. re2c doesn't have syntax for the more complex syntax, so it'd
// be a fair amount of reworking.

class Lexer {
 public:
  explicit Lexer(const std::string& name);
  ~Lexer();
  LexerState* AddState(const std::string& name);
  void GetTokensUnprocessed(const std::string& text,
                            std::vector<Token>* output_tokens);

  enum TokenType {
    Comment,
    CommentMultiline,
    CommentPreproc,
    CommentSingle,
    Error,
    Keyword,
    KeywordConstant,
    KeywordPseudo,
    KeywordReserved,
    KeywordType,
    LiteralNumberFloat,
    LiteralNumberHex,
    LiteralNumberInteger,
    LiteralNumberOct,
    LiteralString,
    LiteralStringChar,
    LiteralStringEscape,
    Name,
    NameBuiltin,
    NameClass,
    NameLabel,
    Operator,
    Punctuation,
    Text,

    Invalid,
  };

  static LexerState* Push;
  static LexerState* Pop;
#ifdef _DEBUG
  static void TidyUpGlobals();
#endif

 private:
  std::string name_;
  std::map<std::string, LexerState*> states_;

  DISALLOW_COPY_AND_ASSIGN(Lexer);
};

class Token {
 public:
  Token() : index(std::numeric_limits<size_t>::max()), token(Lexer::Invalid) {}

  Token(size_t index, Lexer::TokenType token, const std::string& value)
      : index(index), token(token), value(value) {}

  size_t index;
  Lexer::TokenType token;
  std::string value;
};

#endif  // SOURCE_VIEW_LEXER_H_
