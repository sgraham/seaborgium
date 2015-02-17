// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SOURCE_VIEW_LEXER_STATE_H_
#define SOURCE_VIEW_LEXER_STATE_H_

#include <string>
#include <vector>

#include "core.h"
#include "re2/re2.h"
#include "source_view/lexer.h"

class TokenDefinitions {
 public:
  TokenDefinitions() {}
  void Add(const std::string& regex, Lexer::TokenType token_type);
  void AddWithTransition(const std::string& regex,
                         Lexer::TokenType token_type,
                         LexerState* new_state);

 private:
  friend class LexerState;
  struct TokenData {
    TokenData() {}
    TokenData(const std::string& regex,
              Lexer::TokenType action,
              LexerState* new_state)
        : regex(regex), action(action), new_state(new_state) {}

    std::string regex;
    Lexer::TokenType action;
    LexerState* new_state;
  };
  std::vector<TokenData> token_data_;

  DISALLOW_COPY_AND_ASSIGN(TokenDefinitions);
};

class LexerState {
 public:
  explicit LexerState(const std::string& name);
  ~LexerState();
  void SetTokenDefinitions(const TokenDefinitions& token_definitions);

 private:
  friend class Lexer;
  struct TokenDef {
    re2::RE2* regex;
    Lexer::TokenType action;
    LexerState* new_state;
  };
  TokenDef* token_defs_;
  size_t token_defs_count_;

  const TokenDef* GetTokenDefs(size_t* count) const;

  std::string name_;

  DISALLOW_COPY_AND_ASSIGN(LexerState);
};

#endif  // SOURCE_VIEW_LEXER_STATE_H_
