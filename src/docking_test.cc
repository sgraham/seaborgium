// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "docking_resizer.h"
#include "docking_split_container.h"
#include "docking_workspace.h"
#include "leak_check_test.h"

namespace {

class MainDocument : public Widget {};

class ContentPane : public Widget {};

std::string RectAsString(const Rect& r) {
  char buf[256];
  snprintf(buf, sizeof(buf), "%.0f,%.0f %.0fx%.0f", r.x, r.y, r.w, r.h);
  return buf;
}

Point CalculateDragPoint(const DockingResizer& resizer, int dx, int dy) {
  const Point& location = resizer.GetInitialLocationForTest();
  return Point(location.x + dx, location.y + dy);
}

}  // namespace

class DockingTest : public LeakCheckTest {};

TEST_F(DockingTest, Creation) {
  DockingWorkspace workspace;
  workspace.SetRoot(new MainDocument);
  EXPECT_FALSE(workspace.GetRoot()->IsDockingSplitContainer());
  // Just checking for no leaks.
}

TEST_F(DockingTest, AddVerticalSplit) {
  DockingWorkspace workspace;
  MainDocument* main = new MainDocument;
  workspace.SetRoot(main);
  ContentPane* pane = new ContentPane;
  main->parent()->AsDockingSplitContainer()->SplitChild(
      kSplitVertical, main, pane);
  EXPECT_TRUE(workspace.GetRoot()->IsDockingSplitContainer());
  EXPECT_EQ(main, workspace.GetRoot()->AsDockingSplitContainer()->left());
  EXPECT_EQ(pane, workspace.GetRoot()->AsDockingSplitContainer()->right());
  EXPECT_EQ(0.5, workspace.GetRoot()->AsDockingSplitContainer()->fraction());
  EXPECT_EQ(kSplitVertical,
            workspace.GetRoot()->AsDockingSplitContainer()->direction());
}

TEST_F(DockingTest, AddVerticalSplitOtherOrder) {
  DockingWorkspace workspace;
  MainDocument* main = new MainDocument;
  workspace.SetRoot(main);
  ContentPane* pane = new ContentPane;
  main->parent()->AsDockingSplitContainer()->SplitChild(
      kSplitVertical, pane, main);
  EXPECT_TRUE(workspace.GetRoot()->IsDockingSplitContainer());
  EXPECT_EQ(pane, workspace.GetRoot()->AsDockingSplitContainer()->left());
  EXPECT_EQ(main, workspace.GetRoot()->AsDockingSplitContainer()->right());
  EXPECT_EQ(0.5, workspace.GetRoot()->AsDockingSplitContainer()->fraction());
  EXPECT_EQ(kSplitVertical,
            workspace.GetRoot()->AsDockingSplitContainer()->direction());
}

TEST_F(DockingTest, AddHorizontalSplit) {
  DockingWorkspace workspace;
  MainDocument* main = new MainDocument;
  workspace.SetRoot(main);
  ContentPane* pane = new ContentPane;
  main->parent()->AsDockingSplitContainer()->SplitChild(
      kSplitHorizontal, main, pane);
  EXPECT_TRUE(workspace.GetRoot()->IsDockingSplitContainer());
  EXPECT_EQ(main, workspace.GetRoot()->AsDockingSplitContainer()->left());
  EXPECT_EQ(pane, workspace.GetRoot()->AsDockingSplitContainer()->right());
  EXPECT_EQ(0.5, workspace.GetRoot()->AsDockingSplitContainer()->fraction());
  EXPECT_EQ(kSplitHorizontal,
            workspace.GetRoot()->AsDockingSplitContainer()->direction());
}

TEST_F(DockingTest, SubSplit) {
  DockingWorkspace workspace;
  MainDocument* main = new MainDocument;
  workspace.SetRoot(main);
  ContentPane* pane1 = new ContentPane;
  ContentPane* pane2 = new ContentPane;
  main->parent()->AsDockingSplitContainer()->SplitChild(
      kSplitVertical, main, pane1);
  pane1->parent()->AsDockingSplitContainer()->SplitChild(
      kSplitHorizontal, pane1, pane2);
  EXPECT_TRUE(workspace.GetRoot()->IsDockingSplitContainer());
  DockingSplitContainer* root_as_container =
      workspace.GetRoot()->AsDockingSplitContainer();
  EXPECT_EQ(main, root_as_container->left());
  EXPECT_TRUE(root_as_container->right()->IsDockingSplitContainer());
  DockingSplitContainer* subtree =
      root_as_container->right()->AsDockingSplitContainer();
  EXPECT_EQ(pane1, subtree->left());
  EXPECT_EQ(pane2, subtree->right());
  EXPECT_EQ(kSplitHorizontal, subtree->direction());
}

TEST_F(DockingTest, SetSizes) {
  DockingWorkspace workspace;
  DockingSplitContainer::SetSplitterWidth(4);
  workspace.SetScreenRect(Rect(0, 0, 1000, 1000));
  MainDocument* main = new MainDocument;
  workspace.SetRoot(main);
  ContentPane* pane = new ContentPane;
  main->parent()->AsDockingSplitContainer()->SplitChild(
      kSplitVertical, main, pane);
  DockingSplitContainer* root = workspace.GetRoot()->AsDockingSplitContainer();
  EXPECT_EQ(root->left(), main);
  EXPECT_EQ("0,0 498x1000", RectAsString(root->left()->GetScreenRect()));
  EXPECT_EQ(root->right(), pane);
  EXPECT_EQ("502,0 498x1000", RectAsString(root->right()->GetScreenRect()));
}

TEST_F(DockingTest, DragSplitter) {
  DockingWorkspace workspace;
  DockingSplitContainer::SetSplitterWidth(10);
  workspace.SetScreenRect(Rect(0, 0, 1000, 1000));
  MainDocument* main = new MainDocument;
  workspace.SetRoot(main);
  ContentPane* pane = new ContentPane;
  main->parent()->AsDockingSplitContainer()->SplitChild(
      kSplitVertical, main, pane);
  DockingSplitContainer* root = workspace.GetRoot()->AsDockingSplitContainer();
  DockingResizer resizer(root);
  resizer.Drag(CalculateDragPoint(resizer, -200, 10));
  EXPECT_EQ(root->left(), main);
  EXPECT_EQ("0,0 295x1000", RectAsString(root->left()->GetScreenRect()));
  EXPECT_EQ(root->right(), pane);
  EXPECT_EQ("305,0 695x1000", RectAsString(root->right()->GetScreenRect()));
}

TEST_F(DockingTest, DragLeftAndThenRight) {
  DockingWorkspace workspace;
  DockingSplitContainer::SetSplitterWidth(4);
  workspace.SetScreenRect(Rect(0, 0, 1000, 1000));
  MainDocument* main = new MainDocument;
  workspace.SetRoot(main);
  ContentPane* pane = new ContentPane;
  main->parent()->AsDockingSplitContainer()->SplitChild(
      kSplitVertical, main, pane);
  DockingSplitContainer* root = workspace.GetRoot()->AsDockingSplitContainer();
  DockingResizer resizer(root);
  resizer.Drag(CalculateDragPoint(resizer, -200, 10));
  resizer.Drag(CalculateDragPoint(resizer, -150, 10));
  EXPECT_EQ(root->left(), main);
  EXPECT_EQ("0,0 348x1000", RectAsString(root->left()->GetScreenRect()));
  EXPECT_EQ(root->right(), pane);
  EXPECT_EQ("352,0 648x1000", RectAsString(root->right()->GetScreenRect()));
}

TEST_F(DockingTest, DragCancel) {
  DockingWorkspace workspace;
  DockingSplitContainer::SetSplitterWidth(4);
  workspace.SetScreenRect(Rect(0, 0, 1000, 1000));
  MainDocument* main = new MainDocument;
  workspace.SetRoot(main);
  ContentPane* pane = new ContentPane;
  main->parent()->AsDockingSplitContainer()->SplitChild(
      kSplitVertical, main, pane);
  DockingSplitContainer* root = workspace.GetRoot()->AsDockingSplitContainer();
  DockingResizer resizer(root);
  resizer.Drag(CalculateDragPoint(resizer, -200, 10));
  resizer.Drag(CalculateDragPoint(resizer, -150, 10));
  resizer.CancelDrag();
  EXPECT_EQ(root->left(), main);
  EXPECT_EQ("0,0 498x1000", RectAsString(root->left()->GetScreenRect()));
  EXPECT_EQ(root->right(), pane);
  EXPECT_EQ("502,0 498x1000", RectAsString(root->right()->GetScreenRect()));
}

TEST_F(DockingTest, DragPastLeftEdge) {
  DockingWorkspace workspace;
  DockingSplitContainer::SetSplitterWidth(20);
  workspace.SetScreenRect(Rect(0, 0, 1000, 1000));
  MainDocument* main = new MainDocument;
  workspace.SetRoot(main);
  ContentPane* pane = new ContentPane;
  main->parent()->AsDockingSplitContainer()->SplitChild(
      kSplitVertical, main, pane);
  DockingSplitContainer* root = workspace.GetRoot()->AsDockingSplitContainer();
  DockingResizer resizer(root);
  resizer.Drag(CalculateDragPoint(resizer, -600, 10));
  // Not past left edge, with only the splitter remaining on the left. Note
  // large setting for splitter width in this test.
  EXPECT_EQ(root->left(), main);
  EXPECT_EQ("0,0 0x1000", RectAsString(root->left()->GetScreenRect()));
  EXPECT_EQ(root->right(), pane);
  EXPECT_EQ("20,0 980x1000", RectAsString(root->right()->GetScreenRect()));
}

TEST_F(DockingTest, DragPastRightEdge) {
  DockingWorkspace workspace;
  DockingSplitContainer::SetSplitterWidth(20);
  workspace.SetScreenRect(Rect(0, 0, 1000, 1000));
  MainDocument* main = new MainDocument;
  workspace.SetRoot(main);
  ContentPane* pane = new ContentPane;
  main->parent()->AsDockingSplitContainer()->SplitChild(
      kSplitVertical, main, pane);
  DockingSplitContainer* root = workspace.GetRoot()->AsDockingSplitContainer();
  DockingResizer resizer(root);
  // Not past right edge.
  resizer.Drag(CalculateDragPoint(resizer, 900, 10));
  EXPECT_EQ(root->left(), main);
  EXPECT_EQ("0,0 980x1000", RectAsString(root->left()->GetScreenRect()));
  EXPECT_EQ(root->right(), pane);
  EXPECT_EQ("1000,0 0x1000", RectAsString(root->right()->GetScreenRect()));
}

TEST_F(DockingTest, DragUpDown) {
  DockingWorkspace workspace;
  DockingSplitContainer::SetSplitterWidth(20);
  workspace.SetScreenRect(Rect(0, 0, 1000, 1000));
  MainDocument* main = new MainDocument;
  workspace.SetRoot(main);
  ContentPane* pane = new ContentPane;
  main->parent()->AsDockingSplitContainer()->SplitChild(
      kSplitHorizontal, main, pane);
  DockingSplitContainer* root = workspace.GetRoot()->AsDockingSplitContainer();
  DockingResizer resizer(root);
  resizer.Drag(CalculateDragPoint(resizer, 10, 100));
  EXPECT_EQ(root->left(), main);
  EXPECT_EQ("0,0 1000x590", RectAsString(root->left()->GetScreenRect()));
  EXPECT_EQ(root->right(), pane);
  EXPECT_EQ("0,610 1000x390", RectAsString(root->right()->GetScreenRect()));
}

TEST_F(DockingTest, AddAndDelete) {
  DockingWorkspace workspace;
  DockingSplitContainer::SetSplitterWidth(4);
  workspace.SetScreenRect(Rect(0, 0, 1000, 1000));
  MainDocument* main = new MainDocument;
  workspace.SetRoot(main);
  ContentPane* pane = new ContentPane;
  main->parent()->AsDockingSplitContainer()->SplitChild(
      kSplitVertical, main, pane);
  DockingSplitContainer* root = workspace.GetRoot()->AsDockingSplitContainer();
  EXPECT_EQ(root->left(), main);
  EXPECT_EQ("0,0 498x1000", RectAsString(root->left()->GetScreenRect()));
  EXPECT_EQ(root->right(), pane);
  EXPECT_EQ("502,0 498x1000", RectAsString(root->right()->GetScreenRect()));

  pane->parent()->AsDockingSplitContainer()->DeleteChild(pane);
  EXPECT_FALSE(workspace.GetRoot()->IsDockingSplitContainer());
  EXPECT_EQ(main, workspace.GetRoot());
  EXPECT_EQ("0,0 1000x1000",
            RectAsString(workspace.GetRoot()->GetScreenRect()));
}

TEST_F(DockingTest, AddAndRelease) {
  DockingWorkspace workspace;
  DockingSplitContainer::SetSplitterWidth(4);
  workspace.SetScreenRect(Rect(0, 0, 1000, 1000));
  MainDocument* main = new MainDocument;
  workspace.SetRoot(main);
  ContentPane* pane = new ContentPane;
  main->parent()->AsDockingSplitContainer()->SplitChild(
      kSplitVertical, main, pane);
  DockingSplitContainer* root = workspace.GetRoot()->AsDockingSplitContainer();
  EXPECT_EQ(root->left(), main);
  EXPECT_EQ("0,0 498x1000", RectAsString(root->left()->GetScreenRect()));
  EXPECT_EQ(root->right(), pane);
  EXPECT_EQ("502,0 498x1000", RectAsString(root->right()->GetScreenRect()));

  std::unique_ptr<Widget> result(
      pane->parent()->AsDockingSplitContainer()->ReleaseChild(pane));
  EXPECT_EQ(NULL, result->parent());
  EXPECT_FALSE(workspace.GetRoot()->IsDockingSplitContainer());
  EXPECT_EQ(main, workspace.GetRoot());
  EXPECT_EQ("0,0 1000x1000",
            RectAsString(workspace.GetRoot()->GetScreenRect()));
}

TEST_F(DockingTest, SubSplitRectSize) {
  DockingWorkspace workspace;
  DockingSplitContainer::SetSplitterWidth(0);
  workspace.SetScreenRect(Rect(0, 0, 1000, 1000));
  MainDocument* main = new MainDocument;
  workspace.SetRoot(main);
  ContentPane* pane1 = new ContentPane;
  ContentPane* pane2 = new ContentPane;
  ContentPane* pane3 = new ContentPane;
  main->parent()->AsDockingSplitContainer()->SplitChild(
      kSplitVertical, main, pane1);
  pane1->parent()->AsDockingSplitContainer()->SplitChild(
      kSplitVertical, pane1, pane2);
  pane1->parent()->AsDockingSplitContainer()->SplitChild(
      kSplitHorizontal, pane1, pane3);

  EXPECT_EQ("0,0 500x1000", RectAsString(main->GetScreenRect()));
  EXPECT_EQ("500,0 250x500", RectAsString(pane1->GetScreenRect()));
  EXPECT_EQ("750,0 250x1000", RectAsString(pane2->GetScreenRect()));
  EXPECT_EQ("500,500 250x500", RectAsString(pane3->GetScreenRect()));
}

TEST_F(DockingTest, SiblingRetrieval) {
  DockingWorkspace workspace;
  DockingSplitContainer::SetSplitterWidth(0);
  workspace.SetScreenRect(Rect(0, 0, 1000, 1000));
  MainDocument* main = new MainDocument;
  workspace.SetRoot(main);
  ContentPane* pane1 = new ContentPane;
  ContentPane* pane2 = new ContentPane;
  main->parent()->AsDockingSplitContainer()->SplitChild(
      kSplitVertical, main, pane1);
  pane1->parent()->AsDockingSplitContainer()->SplitChild(
      kSplitVertical, pane1, pane2);
  EXPECT_EQ(pane1->parent(), pane2->parent());
  EXPECT_EQ(pane2,
            pane1->parent()->AsDockingSplitContainer()->GetSiblingOf(pane1));
}
