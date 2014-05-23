// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/core.h"
#include "core/entry.h"
#include "core/gfx.h"
#include "ui/skin.h"

#include "nanovg.h"

#if 0
#include "clang-c/Index.h"
#endif

void drawWindow(struct NVGcontext* vg,
                const char* title,
                float x,
                float y,
                float w,
                float h) {
  float cornerRadius = 3.0f;
  struct NVGpaint headerPaint;

  nvgSave(vg);
  //	nvgClearState(vg);

  // Window
  nvgBeginPath(vg);
  nvgRoundedRect(vg, x, y, w, h, cornerRadius);
  nvgFillColor(vg, nvgRGBA(28, 30, 34, 192));
  //	nvgFillColor(vg, nvgRGBA(0,0,0,128));
  nvgFill(vg);

  // Drop shadow
  struct NVGpaint shadowPaint = nvgBoxGradient(vg,
                               x,
                               y + 2,
                               w,
                               h,
                               cornerRadius * 2,
                               10,
                               nvgRGBA(0, 0, 0, 128),
                               nvgRGBA(0, 0, 0, 0));
  nvgBeginPath(vg);
  nvgRect(vg, x - 10, y - 10, w + 20, h + 30);
  nvgRoundedRect(vg, x, y, w, h, cornerRadius);
  nvgPathWinding(vg, NVG_HOLE);
  nvgFillPaint(vg, shadowPaint);
  nvgFill(vg);

  // Header
  headerPaint = nvgLinearGradient(
      vg, x, y, x, y + 15, nvgRGBA(255, 255, 255, 8), nvgRGBA(0, 0, 0, 16));
  nvgBeginPath(vg);
  nvgRoundedRect(vg, x + 1, y + 1, w - 2, 30, cornerRadius - 1);
  nvgFillPaint(vg, headerPaint);
  nvgFill(vg);
  nvgBeginPath(vg);
  nvgMoveTo(vg, x + 0.5f, y + 0.5f + 30);
  nvgLineTo(vg, x + 0.5f + w - 1, y + 0.5f + 30);
  nvgStrokeColor(vg, nvgRGBA(0, 0, 0, 32));
  nvgStroke(vg);

  nvgFontSize(vg, 15.0f);
  nvgFontFace(vg, "sans-bold");
  nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);

  nvgFontBlur(vg, 2);
  nvgFillColor(vg, nvgRGBA(0, 0, 0, 128));
  nvgText(vg, x + w / 2, y + 16 + 1, title, NULL);

  nvgFontBlur(vg, 0);
  nvgFillColor(vg, nvgRGBA(220, 220, 220, 160));
  nvgText(vg, x + w / 2, y + 16, title, NULL);

  nvgRestore(vg);
}

int Main(int argc, char** argv) {
  core::GfxInit();
  Skin::LoadData();

  CORE_UNUSED(argc);
  CORE_UNUSED(argv);

  /*
  uint32_t test_texture_data[4] = {
    0xff0000ff, 0xff00ffff,
    0xff00ff00, 0xffff0000,
  };
  */

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
    nvgBeginFrame(core::VG, 1024, 768, 1024.f / 768.f, NVG_STRAIGHT_ALPHA);

    drawWindow(core::VG, "Breakpoints", 300, 100, 200, 400);
    nvgFontSize(core::VG, 15.0f);
    nvgFontFace(core::VG, "sans");
    nvgFillColor(core::VG, nvgRGBA(255, 255, 255, 192));
    nvgText(core::VG, 100, 100, "This is some stuff!", NULL);

    nvgFontSize(core::VG, 16.0f);
    nvgFontFace(core::VG, "mono");
    nvgFillColor(core::VG, nvgRGBA(255, 255, 255, 192));
    nvgText(core::VG, 100, 200, "int main(int argc, char** argv) {", NULL);

    nvgEndFrame(core::VG);
    core::GfxFrame();
  }

  core::GfxShutdown();

  return 0;
}
