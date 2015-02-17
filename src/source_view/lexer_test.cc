// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "source_view/lexer.h"

#include <gtest/gtest.h>

#include <memory>

#include "source_view/cpp_lexer.h"
#include "source_view/lexer_state.h"

TEST(Lexer, Basic) {
  Lexer* lexer = new Lexer("test");
  LexerState* root = lexer->AddState("root");

  TokenDefinitions defs;
  defs.Add("a", Lexer::Keyword);
  defs.Add("b", Lexer::KeywordConstant);
  defs.Add("c", Lexer::KeywordPseudo);
  root->SetTokenDefinitions(defs);

  std::vector<Token> tokens;
  lexer->GetTokensUnprocessed("ababc", &tokens);

  EXPECT_EQ(5, tokens.size());

  EXPECT_EQ(0, tokens[0].index);
  EXPECT_EQ(Lexer::Keyword, tokens[0].token);
  EXPECT_EQ("a", tokens[0].value);

  EXPECT_EQ(1, tokens[1].index);
  EXPECT_EQ(Lexer::KeywordConstant, tokens[1].token);
  EXPECT_EQ("b", tokens[1].value);

  EXPECT_EQ(2, tokens[2].index);
  EXPECT_EQ(Lexer::Keyword, tokens[2].token);
  EXPECT_EQ("a", tokens[2].value);

  EXPECT_EQ(3, tokens[3].index);
  EXPECT_EQ(Lexer::KeywordConstant, tokens[3].token);
  EXPECT_EQ("b", tokens[3].value);

  EXPECT_EQ(4, tokens[4].index);
  EXPECT_EQ(Lexer::KeywordPseudo, tokens[4].token);
  EXPECT_EQ("c", tokens[4].value);
}

TEST(Lexer, IniFile) {
  Lexer* lexer = new Lexer("ini-ish");
  LexerState* root = lexer->AddState("root");

  TokenDefinitions defs;
  defs.Add("\\s+", Lexer::Text);
  defs.Add("[;#].*", Lexer::CommentSingle);
  defs.Add("\\[.*?\\]", Lexer::Keyword);

  root->SetTokenDefinitions(defs);

  std::vector<Token> tokens;
  lexer->GetTokensUnprocessed("[wee]\n; stuff\n# things\n    \n", &tokens);

  EXPECT_EQ(6, tokens.size());
  // TODO(scottmg): More detailed expectations.
}

TEST(Lexer, BasicCpp) {
  std::unique_ptr<Lexer> lexer(MakeCppLexer());

  std::vector<Token> tokens;
  lexer->GetTokensUnprocessed("int foo;", &tokens);
  EXPECT_EQ(4, tokens.size());

  EXPECT_EQ(Lexer::KeywordType, tokens[0].token);
  EXPECT_EQ(0, tokens[0].index);

  EXPECT_EQ(Lexer::Text, tokens[1].token);
  EXPECT_EQ(3, tokens[1].index);

  EXPECT_EQ(Lexer::Name, tokens[2].token);
  EXPECT_EQ(4, tokens[2].index);

  EXPECT_EQ(Lexer::Punctuation, tokens[3].token);
  EXPECT_EQ(7, tokens[3].index);
}

TEST(Lexer, CppIf0) {
  std::unique_ptr<Lexer> lexer(MakeCppLexer());

  std::vector<Token> tokens;
  lexer->GetTokensUnprocessed("#if 0\nthis is some stuff\n#endif\n", &tokens);

  EXPECT_EQ(3, tokens.size());
  EXPECT_EQ(Lexer::CommentPreproc, tokens[0].token);
  EXPECT_EQ(Lexer::Comment, tokens[1].token);
  EXPECT_EQ(Lexer::CommentPreproc, tokens[2].token);
}

TEST(Lexer, CppNumbers) {
  std::unique_ptr<Lexer> lexer(MakeCppLexer());

  std::vector<Token> tokens;
  lexer->GetTokensUnprocessed("42 23.42 23. .42 023 0xdeadbeef 23e+42 42e-23",
                              &tokens);

  EXPECT_EQ(15, tokens.size());
  EXPECT_EQ(Lexer::LiteralNumberInteger, tokens[0].token);
  EXPECT_EQ(Lexer::LiteralNumberFloat, tokens[2].token);
  EXPECT_EQ(Lexer::LiteralNumberFloat, tokens[4].token);
  EXPECT_EQ(Lexer::LiteralNumberFloat, tokens[6].token);
  EXPECT_EQ(Lexer::LiteralNumberOct, tokens[8].token);
  EXPECT_EQ(Lexer::LiteralNumberHex, tokens[10].token);
  EXPECT_EQ(Lexer::LiteralNumberFloat, tokens[12].token);
  EXPECT_EQ(Lexer::LiteralNumberFloat, tokens[14].token);
}
