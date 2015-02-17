// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "source_view/lexer_state.h"

void TokenDefinitions::Add(const std::string& regex,
                           Lexer::TokenType token_type) {
  token_data_.push_back(TokenData(regex, token_type, NULL));
}

void TokenDefinitions::AddWithTransition(const std::string& regex,
                                         Lexer::TokenType token_type,
                                         LexerState* new_state) {
  token_data_.push_back(TokenData(regex, token_type, new_state));
}

LexerState::LexerState(const std::string& name)
    : token_defs_(NULL), token_defs_count_(0), name_(name) {
}

LexerState::~LexerState() {
  for (size_t i = 0; i < token_defs_count_; ++i) {
    delete token_defs_[i].regex;
  }
  delete[] token_defs_;
}

void LexerState::SetTokenDefinitions(const TokenDefinitions& tokens) {
  token_defs_count_ = tokens.token_data_.size();
  token_defs_ = new TokenDef[token_defs_count_];
  re2::RE2::Options options;
  for (size_t i = 0; i < token_defs_count_; ++i) {
    token_defs_[i].regex = new re2::RE2(tokens.token_data_[i].regex, options);
    token_defs_[i].action = tokens.token_data_[i].action;
    token_defs_[i].new_state = tokens.token_data_[i].new_state;
  }
}

const LexerState::TokenDef* LexerState::GetTokenDefs(size_t* count) const {
  *count = token_defs_count_;
  return token_defs_;
}
