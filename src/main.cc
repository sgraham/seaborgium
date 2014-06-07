// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/core.h"
#include "core/entry.h"
#include "core/gfx.h"
#include "ui/drawing_common.h"
#include "ui/skin.h"
#include "ui/solid_color.h"
#include "ui/docking_split_container.h"
#include "ui/docking_tool_window.h"
#include "ui/docking_workspace.h"
#include "ui/focus.h"

#include "nanovg.h"

#if 0
#include "clang-c/Index.h"
#endif

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
    // fprintf(stderr, "%s: %s\n", clang_getCString(text), kind_str);
    clang_disposeString(text);
  }

  clang_disposeTokens(tu, tokens, num_tokens);
  clang_disposeTranslationUnit(tu);
  clang_disposeIndex(index);
#endif

  DockingWorkspace main_area;
  const Skin& skin = Skin::current();
  const ColorScheme& cs = skin.GetColorScheme();
  SolidColor* source_view = new SolidColor(cs.background());

  DockingToolWindow* stack =
      new DockingToolWindow(new SolidColor(cs.background()), "Stack");

  DockingToolWindow* watch =
      new DockingToolWindow(new SolidColor(cs.background()), "Watch");

  DockingToolWindow* breakpoints =
      new DockingToolWindow(new SolidColor(cs.background()), "Breakpoints");

  SolidColor* command_contents = new SolidColor(cs.background());
  DockingToolWindow* command =
      new DockingToolWindow(command_contents, "Command");
  SetFocusedContents(command_contents);

  main_area.SetRoot(source_view);
  source_view->parent()->SplitChild(kSplitHorizontal, source_view, command);
  source_view->parent()->SetFraction(0.7);

  source_view->parent()->SplitChild(kSplitVertical, source_view, stack);
  source_view->parent()->SetFraction(0.4);

  stack->parent()->SplitChild(kSplitVertical, stack, watch);
  stack->parent()->SetFraction(0.5);

  stack->parent()->SplitChild(kSplitHorizontal, stack, breakpoints);
  stack->parent()->SetFraction(0.7);

  uint32_t prev_width = 0, prev_height = 0;
  uint32_t width, height;
  while (!core::ProcessEvents(&width, &height, &main_area)) {
    if (prev_width != width || prev_height != height) {
      main_area.SetScreenRect(
          Rect(static_cast<int>(skin.border_size() / core::GetDpiScale()),
              static_cast<int>(skin.border_size() / core::GetDpiScale()),
              static_cast<int>((width - skin.border_size() * 2) /
                                core::GetDpiScale()),
              static_cast<int>((height - skin.border_size() * 2) /
                                core::GetDpiScale())));
      core::GfxResize(width, height);
      prev_width = width;
      prev_height = height;
    }

    nvgBeginFrame(core::VG,
                  static_cast<int>(width / core::GetDpiScale()),
                  static_cast<int>(height / core::GetDpiScale()),
                  core::GetDpiScale(),
                  NVG_STRAIGHT_ALPHA);

    main_area.Render();

    nvgFontSize(core::VG, 13.0f);
    nvgFontFace(core::VG, "mono");
    nvgFillColor(core::VG, Skin::current().GetColorScheme().text());
    nvgText(core::VG,
            100,
            200,
            "int main(int argc, char** argv) {",
            NULL);

    core::GfxDrawFps();

    nvgEndFrame(core::VG);
    core::GfxFrame();
  }

  core::GfxShutdown();

  return 0;
}
