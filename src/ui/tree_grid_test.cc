// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/tree_grid.h"

#include <gtest/gtest.h>

TEST(TreeGridTest, ColumnLayout) {
  TreeGrid tg;

  // Basics.
  TreeGridColumn* c0 = new TreeGridColumn(&tg, "col0");
  EXPECT_EQ(c0->GetCaption(), "col0");
  tg.Columns()->push_back(c0);
  c0->SetWidthPercentage(0.5f);
  std::vector<float> width_one_col = tg.GetColumnWidths(100);
  EXPECT_EQ(100.f, width_one_col[0]);

  TreeGridColumn* c1 = new TreeGridColumn(&tg, "col1");
  tg.Columns()->push_back(c1);
  c1->SetWidthPercentage(0.5f);
  std::vector<float> width_two_col = tg.GetColumnWidths(100);
  EXPECT_EQ(50.f, width_two_col[0]);
  EXPECT_EQ(50.f, width_two_col[1]);

  // Over 1.0 sized.
  TreeGridColumn* c2 = new TreeGridColumn(&tg, "col2");
  tg.Columns()->push_back(c2);
  c2->SetWidthPercentage(1.f);
  std::vector<float> width_three_col = tg.GetColumnWidths(100);
  EXPECT_EQ(25.f, width_three_col[0]);
  EXPECT_EQ(25.f, width_three_col[1]);
  EXPECT_EQ(50.f, width_three_col[2]);
}

TEST(TreeGridTest, ColumnResizeWidth) {
  TreeGrid tg;

  TreeGridColumn* c0 = new TreeGridColumn(&tg, "col0");
  tg.Columns()->push_back(c0);
  c0->SetWidthPercentage(0.5f);
  TreeGridColumn* c1 = new TreeGridColumn(&tg, "col1");
  tg.Columns()->push_back(c1);
  c1->SetWidthPercentage(0.5f);
  std::vector<float> width_two_col = tg.GetColumnWidths(100);
  EXPECT_EQ(50.f, width_two_col[0]);
  EXPECT_EQ(50.f, width_two_col[1]);

  // Resize as if it was being dragged.
  c0->SetPercentageToMatchWidth(40.f, 100.f);

  EXPECT_FLOAT_EQ(0.4f, c0->WidthPercentage());

  width_two_col = tg.GetColumnWidths(100);
  EXPECT_FLOAT_EQ(40.f, width_two_col[0]);
  EXPECT_FLOAT_EQ(60.f, width_two_col[1]);
}

TEST(TreeGridTest, ColumnResizePosition) {
  TreeGrid tg;

  TreeGridColumn* c0 = new TreeGridColumn(&tg, "col0");
  tg.Columns()->push_back(c0);
  c0->SetWidthPercentage(0.25f);
  TreeGridColumn* c1 = new TreeGridColumn(&tg, "col1");
  tg.Columns()->push_back(c1);
  c1->SetWidthPercentage(0.25f);
  TreeGridColumn* c2 = new TreeGridColumn(&tg, "col2");
  tg.Columns()->push_back(c2);
  c2->SetWidthPercentage(0.5f);
  std::vector<float> widths = tg.GetColumnWidths(100);
  EXPECT_EQ(25.f, widths[0]);
  EXPECT_EQ(25.f, widths[1]);
  EXPECT_EQ(50.f, widths[2]);

  // Resize as if it was being dragged.
  c1->SetPercentageToMatchPosition(40.f, 100.f);

  widths = tg.GetColumnWidths(100);
  EXPECT_FLOAT_EQ(25.f, widths[0]);
  EXPECT_FLOAT_EQ(15.f, widths[1]);
  EXPECT_FLOAT_EQ(60.f, widths[2]);
}

TEST(TreeGridTest, ColumnResizePositionInvalid) {
  TreeGrid tg;

  TreeGridColumn* c0 = new TreeGridColumn(&tg, "col0");
  tg.Columns()->push_back(c0);
  c0->SetWidthPercentage(0.25f);
  TreeGridColumn* c1 = new TreeGridColumn(&tg, "col1");
  tg.Columns()->push_back(c1);
  c1->SetWidthPercentage(0.25f);
  TreeGridColumn* c2 = new TreeGridColumn(&tg, "col2");
  tg.Columns()->push_back(c2);
  c2->SetWidthPercentage(0.5f);
  std::vector<float> widths = tg.GetColumnWidths(100);
  EXPECT_EQ(25.f, widths[0]);
  EXPECT_EQ(25.f, widths[1]);
  EXPECT_EQ(50.f, widths[2]);

  // If too small, clamp.
  c1->SetPercentageToMatchPosition(20.f, 100.f);

  // TODO(scottmg): 3 is the standard border width in the skin, but shouldn't
  // be duplicated here.
  const float kBorderWidth = 3.f;

  widths = tg.GetColumnWidths(100);
  EXPECT_FLOAT_EQ(25.f, widths[0]);
  EXPECT_FLOAT_EQ(kBorderWidth, widths[1]);
  EXPECT_FLOAT_EQ(75.f - kBorderWidth, widths[2]);

  // If too large, clamp.
  c1->SetPercentageToMatchPosition(110.f, 100.f);

  widths = tg.GetColumnWidths(100);
  EXPECT_FLOAT_EQ(25.f, widths[0]);
  EXPECT_FLOAT_EQ(100.f - 25.f - kBorderWidth, widths[1]);
  EXPECT_FLOAT_EQ(kBorderWidth, widths[2]);
}

namespace {

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
  name_column->SetWidthPercentage(0.3f);
  value_column->SetWidthPercentage(0.7f);
  type_column->SetWidthPercentage(0.4f);

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

}  // namespace

TEST(TreeGridTest, FocusMovementOnEmptyTree) {
  TreeGrid tg;
  tg.MoveFocusByDirection(TreeGrid::kFocusDown);
  // Mostly that iterator debugging doesn't assert.
  EXPECT_EQ(NULL, tg.GetFocusedNode());
}

TEST(TreeGridTest, FocusMovementNoneThenDown) {
  TreeGrid tg;
  FillWatchWithSampleData(&tg);
  tg.MoveFocusByDirection(TreeGrid::kFocusDown);
  EXPECT_EQ(tg.Nodes()->at(0), tg.GetFocusedNode());
}

TEST(TreeGridTest, FocusMovementNoneThenUp) {
  TreeGrid tg;
  FillWatchWithSampleData(&tg);
  tg.MoveFocusByDirection(TreeGrid::kFocusUp);
  EXPECT_EQ(tg.Nodes()->at(1), tg.GetFocusedNode());
}

TEST(TreeGridTest, FocusMovementDownTwice) {
  TreeGrid tg;
  FillWatchWithSampleData(&tg);
  tg.MoveFocusByDirection(TreeGrid::kFocusDown);
  tg.MoveFocusByDirection(TreeGrid::kFocusDown);
  EXPECT_EQ(tg.Nodes()->at(1), tg.GetFocusedNode());
}

TEST(TreeGridTest, FocusMovementDownPastBottomClamps) {
  TreeGrid tg;
  FillWatchWithSampleData(&tg);
  tg.MoveFocusByDirection(TreeGrid::kFocusDown);
  tg.MoveFocusByDirection(TreeGrid::kFocusDown);
  tg.MoveFocusByDirection(TreeGrid::kFocusDown);
  tg.MoveFocusByDirection(TreeGrid::kFocusDown);
  tg.MoveFocusByDirection(TreeGrid::kFocusDown);
  EXPECT_EQ(tg.Nodes()->at(1), tg.GetFocusedNode());
}

TEST(TreeGridTest, FocusMovementDownWithExpansions) {
  TreeGrid tg;
  FillWatchWithSampleData(&tg);
  tg.Nodes()->at(0)->SetExpanded(true);
  tg.Nodes()->at(0)->Nodes()->at(2)->SetExpanded(true);
  tg.MoveFocusByDirection(TreeGrid::kFocusDown);
  tg.MoveFocusByDirection(TreeGrid::kFocusDown);
  tg.MoveFocusByDirection(TreeGrid::kFocusDown);
  tg.MoveFocusByDirection(TreeGrid::kFocusDown);
  tg.MoveFocusByDirection(TreeGrid::kFocusDown);
  EXPECT_EQ("x", tg.GetFocusedNode()->GetValue(0)->AsString());
}

TEST(TreeGridTest, FocusMovementUpWithExpansions) {
  TreeGrid tg;
  FillWatchWithSampleData(&tg);
  // Down to end.
  tg.MoveFocusByDirection(TreeGrid::kFocusDown);
  tg.MoveFocusByDirection(TreeGrid::kFocusDown);
  // Expand.
  tg.Nodes()->at(0)->SetExpanded(true);
  tg.Nodes()->at(0)->Nodes()->at(2)->SetExpanded(true);
  // Move back up and walk to children.
  tg.MoveFocusByDirection(TreeGrid::kFocusUp);
  EXPECT_EQ("draggable_", tg.GetFocusedNode()->GetValue(0)->AsString());
  tg.MoveFocusByDirection(TreeGrid::kFocusUp);
  EXPECT_EQ("y", tg.GetFocusedNode()->GetValue(0)->AsString());
  tg.MoveFocusByDirection(TreeGrid::kFocusUp);
  EXPECT_EQ("x", tg.GetFocusedNode()->GetValue(0)->AsString());
  tg.MoveFocusByDirection(TreeGrid::kFocusUp);
  EXPECT_EQ("mouse_position_", tg.GetFocusedNode()->GetValue(0)->AsString());
}


// TODO: Leakiness test.
