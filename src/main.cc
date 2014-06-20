// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/core.h"
#include "core/entry.h"
#include "core/gfx.h"
#include "ui/docking_split_container.h"
#include "ui/docking_tool_window.h"
#include "ui/docking_workspace.h"
#include "ui/drawing_common.h"
#include "ui/focus.h"
#include "ui/skin.h"
#include "ui/solid_color.h"
#include "ui/text_edit.h"
#include "ui/tree_grid.h"

#include "nanovg.h"

#if 0
#include "clang-c/Index.h"
#endif

void FillColumns(TreeGridNode* node,
                 const char* name,
                 const char* value,
                 const char* type) {
  node->SetValue(0, new TreeGridNodeValueString(name));
  node->SetValue(1, new TreeGridNodeValueString(value));
  node->SetValue(2, new TreeGridNodeValueString(type));
}

void FillWatchWithSampleData(TreeGrid* watch) {
  // The TreeGrid owns all these pointers once they're added.

  TreeGridColumn* name_column = new TreeGridColumn(watch, "Name");
  TreeGridColumn* value_column = new TreeGridColumn(watch, "Value");
  TreeGridColumn* type_column = new TreeGridColumn(watch, "Type");
  watch->Columns()->push_back(name_column);
  watch->Columns()->push_back(value_column);
  watch->Columns()->push_back(type_column);
  name_column->SetWidthPercentage(0.25f);
  value_column->SetWidthPercentage(0.5f);
  type_column->SetWidthPercentage(0.25f);

  TreeGridNode* root0 = new TreeGridNode(watch, NULL);
  watch->Nodes()->push_back(root0);
  FillColumns(root0,
              "this",
              "{root_=unique_ptr {direction_=kSplitNoneRoot (0) "
              "fraction_=0.50000000000000000 left_=unique_ptr "
              "{direction_=kSplitHorizontal (2) fraction_=0.69999999999999996 "
              "left_=unique_ptr ...} ...} ...}",
              "DockingWorkspace *");

  TreeGridNode* child0 = new TreeGridNode(watch, root0);
  root0->Nodes()->push_back(child0);
  FillColumns(child0, "core::InputHandler", "{...}", "core::InputHandler");

  TreeGridNode* child1 = new TreeGridNode(watch, root0);
  root0->Nodes()->push_back(child1);
  FillColumns(child1,
              "root_",
              "unique_ptr {direction_=kSplitNoneRoot (0) "
              "fraction_=0.50000000000000000 left_=unique_ptr "
              "{direction_=kSplitHorizontal (2) fraction_=0.69999999999999996 "
              "left_=unique_ptr {...} ...} ...}",
              "std::unique_ptr<DockingSplitContainer,std::default_delete<"
              "DockingSplitContainer> >");

  TreeGridNode* child2 = new TreeGridNode(watch, root0);
  root0->Nodes()->push_back(child2);
  FillColumns(child2, "mouse_position_", "{x=1924 y=440 }", "Point");

  TreeGridNode* child2_0 = new TreeGridNode(watch, child2);
  child2->Nodes()->push_back(child2_0);
  FillColumns(child2_0, "x", "1924", "int");

  TreeGridNode* child2_1 = new TreeGridNode(watch, child2);
  child2->Nodes()->push_back(child2_1);
  FillColumns(child2_1, "y", "440", "int");

  TreeGridNode* child3 = new TreeGridNode(watch, root0);
  root0->Nodes()->push_back(child3);
  FillColumns(child3,
              "draggable_",
              "empty",
              "std::unique_ptr<Draggable,std::default_delete<Draggable> >");

  TreeGridNode* root1 = new TreeGridNode(watch, NULL);
  watch->Nodes()->push_back(root1);
  FillColumns(root1,
              "target",
              "0x040a87b0 {color_={rgba=0x040a87c8 {0.000000000, 0.168627456, "
              "0.211764708, 1.00000000} r=0.000000000 ...} }",
              "Dockable *");
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

  TreeGrid* watch_contents = new TreeGrid();
  DockingToolWindow* watch = new DockingToolWindow(watch_contents, "Watch");

  FillWatchWithSampleData(watch_contents);

  DockingToolWindow* breakpoints =
      new DockingToolWindow(new TreeGrid(), "Breakpoints");

  TextEdit* command_contents = new TextEdit();
  DockingToolWindow* command =
      new DockingToolWindow(command_contents, "Command");
  SetFocusedContents(command_contents);

  main_area.SetRoot(source_view);
  source_view->parent()->SplitChild(kSplitHorizontal, source_view, command);
  source_view->parent()->SetFraction(0.7f);

  source_view->parent()->SplitChild(kSplitVertical, source_view, stack);
  source_view->parent()->SetFraction(0.4f);

  stack->parent()->SplitChild(kSplitVertical, stack, watch);
  stack->parent()->SetFraction(0.5f);

  stack->parent()->SplitChild(kSplitHorizontal, stack, breakpoints);
  stack->parent()->SetFraction(0.7f);

  uint32_t prev_width = 0, prev_height = 0;
  uint32_t width, height;
  while (!core::ProcessEvents(&width, &height, &main_area)) {
    if (prev_width != width || prev_height != height) {
      main_area.SetScreenRect(
          Rect(skin.border_size() / core::GetDpiScale(),
               skin.border_size() / core::GetDpiScale(),
               (width - skin.border_size() * 2) / core::GetDpiScale(),
               (height - skin.border_size() * 2) / core::GetDpiScale()));
      core::GfxResize(width, height);
      prev_width = width;
      prev_height = height;
    }

    nvgBeginFrame(core::VG,
                  static_cast<int>(width / core::GetDpiScale()),
                  static_cast<int>(height / core::GetDpiScale()),
                  core::GetDpiScale());

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
