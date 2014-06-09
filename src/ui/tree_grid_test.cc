// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/tree_grid.h"

#include <gtest/gtest.h>

TEST(TreeGridTest, ColumnSizing) {
  TreeGrid tg;

  // Basics.
  TreeGridColumn* c0 = new TreeGridColumn(&tg, "col0");
  EXPECT_EQ(c0->GetCaption(), "col0");
  tg.Columns()->push_back(c0);
  c0->SetWidthPercentage(0.5f);
  std::vector<int> width_one_col = tg.GetColumnWidths(100);
  EXPECT_EQ(100, width_one_col[0]);

  TreeGridColumn* c1 = new TreeGridColumn(&tg, "col1");
  tg.Columns()->push_back(c1);
  c1->SetWidthPercentage(0.5f);
  std::vector<int> width_two_col = tg.GetColumnWidths(100);
  EXPECT_EQ(50, width_two_col[0]);
  EXPECT_EQ(50, width_two_col[1]);

  // Fixed width.
  TreeGridColumn* c2 = new TreeGridColumn(&tg, "col2");
  tg.Columns()->push_back(c2);
  c2->SetWidthFixed(40);
  std::vector<int> width_three_col = tg.GetColumnWidths(100);
  EXPECT_EQ(30, width_three_col[0]);
  EXPECT_EQ(30, width_three_col[1]);
  EXPECT_EQ(40, width_three_col[2]);

  // Overflow clamps.
  c2->SetWidthFixed(110);
  width_three_col = tg.GetColumnWidths(100);
  EXPECT_EQ(0, width_three_col[0]);
  EXPECT_EQ(0, width_three_col[1]);
  EXPECT_EQ(100, width_three_col[2]);
}
