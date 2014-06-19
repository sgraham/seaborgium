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
  virtual void Render(const Rect& rect) const = 0;
};

class TreeGridNodeValueString : public TreeGridNodeValue {
 public:
  explicit TreeGridNodeValueString(const std::string& value);
  virtual void Render(const Rect& rect) const override;

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

  bool Selected() const { return selected_; }
  void SetSelected(bool selected) { selected_ = selected; }

  const TreeGridNode* Parent() const { return parent_; }

  const std::vector<TreeGridNode*>* Nodes() const;
  std::vector<TreeGridNode*>* Nodes();

  void SetValue(int column, TreeGridNodeValue* value);
  const TreeGridNodeValue* GetValue(int column) const;

  const TreeGrid* GetTreeGrid() const { return tree_grid_; }

 private:
  TreeGrid* tree_grid_;
  TreeGridNode* parent_;
  bool expanded_;
  bool selected_;
  std::vector<TreeGridNode*> nodes_;
  std::map<int, TreeGridNodeValue*> items_;
};

class TreeGridColumn {
 public:
  TreeGridColumn(TreeGrid* tree_grid, const std::string& caption);

  const TreeGrid* GetTreeGrid() const { return tree_grid_; }

  const std::string& GetCaption() const { return caption_; }
  void SetCaption(const std::string& caption) { caption_ = caption; }

  void SetWidthPercentage(float fraction);
  void SetWidthFixed(int size);

 private:
  friend TreeGrid;
  TreeGrid* tree_grid_;
  std::string caption_;

  float width_fraction_;
  int width_fixed_;
};

// Hierarchical view, with columns.
class TreeGrid : public Dockable {
 public:
  TreeGrid();
  virtual ~TreeGrid();

  std::vector<TreeGridNode*>* Nodes();
  std::vector<TreeGridColumn*>* Columns();

  virtual void Render() override;

  virtual bool CouldStartDrag(DragSetup* drag_setup) override;

  virtual bool NotifyMouseButton(int x,
                                 int y,
                                 core::MouseButton::Enum button,
                                 bool down,
                                 uint8_t modifiers) override;

  std::vector<float> GetColumnWidths(float layout_in_width) const;

 private:
  void RenderNodes(const std::vector<TreeGridNode*>& nodes,
                   const std::vector<float>& column_widths,
                   const float depth_per_indent,
                   float current_indent,
                   float* y_position);

  std::vector<TreeGridNode*> nodes_;
  std::vector<TreeGridColumn*> columns_;

  std::vector<float> column_splitters_;

  struct ExpansionBoxPosition {
    ExpansionBoxPosition(const Rect& rect, TreeGridNode* node)
        : rect(rect), node(node) {}
    Rect rect;
    TreeGridNode* node;
  };
  std::vector<ExpansionBoxPosition> expansion_boxes_;

  CORE_DISALLOW_COPY_AND_ASSIGN(TreeGrid);
};

#endif  // UI_TREE_GRID_H_
