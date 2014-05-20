// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/core.h"
#include "core/entry.h"
#include "core/gfx.h"

#if 0
#include "clang-c/Index.h"
#endif


int Main(int argc, char** argv) {
  core::gfx::Init();
  CORE_UNUSED(argc);
  CORE_UNUSED(argv);

  uint32_t test_texture_data[4] = {
    0xff0000ff, 0xff00ffff,
    0xff00ff00, 0xffff0000,
  };
  core::gfx::TextureId test_texture =
      core::gfx::LoadTexture(test_texture_data, 2, 2);

#if 0  // Works fine, but tokenizing at this level isn't really enough for
       // syntax highlighting.
  const char* args[] = {"-Isrc", "-D_CRT_SECURE_NO_WARNINGS"};
  int numArgs = sizeof(args) / sizeof(*args);
  CXIndex index = clang_createIndex(0, 0);
  CXTranslationUnit tu = clang_parseTranslationUnit(
      index, "src/main.cc", args, numArgs, NULL, 0, CXTranslationUnit_None);

  CXSourceRange range =
      clang_getCursorExtent(clang_getTranslationUnitCursor(tu));
  CXToken* tokens;
  unsigned num_tokens;
  clang_tokenize(tu, range, &tokens, &num_tokens);

  for (unsigned i = 0; i < num_tokens; ++i) {
    CXTokenKind token_kind = clang_getTokenKind(tokens[i]);
    const char* kind_str = "???";
    switch (token_kind) {
      case CXToken_Punctuation:
        kind_str = "Punctuation";
        break;
      case CXToken_Keyword:
        kind_str = "Keyword";
        break;
      case CXToken_Identifier:
        kind_str = "Identifier";
        break;
      case CXToken_Literal:
        kind_str = "Literal";
        break;
      case CXToken_Comment:
        kind_str = "Comment";
        break;
    }
    CXString text = clang_getTokenSpelling(tu, tokens[i]);
    //fprintf(stderr, "%s: %s\n", clang_getCString(text), kind_str);
    clang_disposeString(text);
  }

  clang_disposeTokens(tu, tokens, num_tokens);
  clang_disposeTranslationUnit(tu);
  clang_disposeIndex(index);
#endif

  uint32_t width, height;
  while (!core::ProcessEvents(&width, &height)) {
    core::gfx::DrawRect(5, 5, 100, 100, 0x60000000);
    core::gfx::DrawSprite(150, 150, 0xffffffff, test_texture);
    core::gfx::DrawTexturedRect(200, 200, 200, 200, 0xffffffff, test_texture);
    core::gfx::Frame();
  }

  core::gfx::Shutdown();

  return 0;
}
