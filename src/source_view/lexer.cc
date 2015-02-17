// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "source_view/lexer.h"

#include "core.h"
#include "source_view/lexer_state.h"

namespace {

// Get the result of a parse from a initial and final StringPiece representing
// the current parse locations.
std::string GetResult(const re2::StringPiece& initial,
                      const re2::StringPiece final) {
  size_t length = final.data() - initial.data();
  return std::string(initial.data(), length);
}

size_t GetOffset(const re2::StringPiece& before, const std::string& base) {
  return before.data() - base.data();
}

}  // namespace

LexerState* Lexer::Push;
LexerState* Lexer::Pop;

#ifdef _DEBUG
// static
void Lexer::TidyUpGlobals() {
  delete Lexer::Push;
  delete Lexer::Pop;
  Lexer::Push = NULL;
  Lexer::Pop = NULL;
}
#endif

Lexer::Lexer(const std::string& name) : name_(name) {
  if (!Push) {
    Push = new LexerState("!<push>");
    Pop = new LexerState("!<pop>");
  }
}

Lexer::~Lexer() {
  for (std::map<std::string, LexerState*>::iterator i(states_.begin());
       i != states_.end();
       ++i) {
    delete i->second;
  }
}

LexerState* Lexer::AddState(const std::string& name) {
  LexerState* lexer_state = new LexerState(name);
  states_[name] = lexer_state;
  return lexer_state;
}

void Lexer::GetTokensUnprocessed(const std::string& text,
                                 std::vector<Token>* output_tokens) {
  std::vector<LexerState*> state_stack;
  CHECK(states_.find("root") != states_.end(), "expected root");
  state_stack.push_back(states_["root"]);

  re2::StringPiece input(text);
  for (;;) {
    LexerState* current_state = state_stack.back();
    size_t token_defs_count;
    const LexerState::TokenDef* token_defs =
        current_state->GetTokenDefs(&token_defs_count);
    size_t i;
    for (i = 0; i < token_defs_count; ++i) {
      re2::StringPiece from = input;
      const LexerState::TokenDef* token_def = &token_defs[i];
      if (RE2::Consume(&input, *token_def->regex)) {
        output_tokens->push_back(Token(
            GetOffset(from, text), token_def->action, GetResult(from, input)));
        if (token_def->new_state) {
          if (token_def->new_state == Push) {
            state_stack.push_back(current_state);
          } else if (token_def->new_state == Pop) {
            state_stack.pop_back();
          } else {
            // TODO(scottmg): state tuple, if needed.
            state_stack.push_back(token_def->new_state);
          }
        }
        break;
      }
    }
    if (i == token_defs_count) {
      if (input.empty())
        break;
      // No match, if at EOL, reset to root state.
      if (input[0] == '\n') {
        CHECK(false, "todo; untested");
        state_stack.clear();
        state_stack.push_back(states_["root"]);
        output_tokens->push_back(Token(GetOffset(input, text), Text, "\n"));
        input = re2::StringPiece(input.data() + 1, input.size() - 1);
      } else {
        CHECK(false, "todo; untested, should add Error token");
      }
    }
  }
}
