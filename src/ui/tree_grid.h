// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_TREE_GRID_H_
#define UI_TREE_GRID_H_

#include <map>
#include <string>
#include <vector>

#include "ui/dockable.h"

class TreeGridNodeValue {
 public:
  virtual ~TreeGridNodeValue();
  virtual void Render() = 0;
};

class TreeGridNodeValueString : public TreeGridNodeValue {
 public:
  explicit TreeGridNodeValueString(const std::string& value);
  void Render() override;

 private:
  std::string value_;
};

class TreeGrid;

// Tree of nodes. All pointers are owned, and will be delete'd.
class TreeGridNode {
 public:
  TreeGridNode(TreeGrid* tree_grid, TreeGridNode* parent);
  ~TreeGridNode();

  bool Expanded() const { return expanded_; }
  void SetExpanded(bool expanded) { expanded_ = expanded; }

  const TreeGridNode* Parent() const { return parent_; }

  std::vector<TreeGridNode*>* Nodes();

  void SetValue(int column, TreeGridNodeValue* value);
  const TreeGridNodeValue* GetValue(int column);

  const TreeGrid* GetTreeGrid() const { return tree_grid_; }

 private:
  TreeGrid* tree_grid_;
  TreeGridNode* parent_;
  bool expanded_;
  std::vector<TreeGridNode*> nodes_;
  std::map<int, TreeGridNodeValue*> items_;
};

class TreeGridColumn {
 public:
  TreeGridColumn(TreeGrid* tree_grid, const std::string& caption);

  const std::string& GetCaption() const { return caption_; }
  void SetCaption(const std::string& caption) { caption_ = caption; }

 private:
  TreeGrid* tree_grid_;
  std::string caption_;
};

// Hierarchical view, with columns.
class TreeGrid : public Dockable {
 public:
  TreeGrid();
  virtual ~TreeGrid();

  std::vector<TreeGridNode*>* Nodes();
  std::vector<TreeGridColumn*>* Columns();

  void Render() override;

 private:
  std::vector<TreeGridNode*> nodes_;
  std::vector<TreeGridColumn*> columns_;

  CORE_DISALLOW_COPY_AND_ASSIGN(TreeGrid);
};

#endif  // UI_TREE_GRID_H_
