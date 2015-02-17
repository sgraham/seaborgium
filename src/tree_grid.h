// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TREE_GRID_H_
#define TREE_GRID_H_

#include <map>
#include <string>
#include <vector>

#include "widget.h"

class TextEdit;

class TreeGridNodeValue {
 public:
  virtual ~TreeGridNodeValue();
  virtual void Render(const Rect& rect) const = 0;
  virtual std::string AsString() const = 0;
};

class TreeGridNodeValueString : public TreeGridNodeValue {
 public:
  explicit TreeGridNodeValueString(const std::string& value);
  virtual void Render(const Rect& rect) const override;
  virtual std::string AsString() const override {
    return value_;
  }

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
  TreeGridNode* Parent() { return parent_; }

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
  float WidthPercentage() const { return width_fraction_; }
  void SetPercentageToMatchWidth(float width, float whole_width);
  void SetPercentageToMatchPosition(float splitter_position, float whole_width);

 private:
  friend TreeGrid;
  TreeGrid* tree_grid_;
  std::string caption_;

  float width_fraction_;
};

class TreeGridEditObserver {
 public:
  virtual ~TreeGridEditObserver() {}

  // Node will be removed from the tree. Returning false cancels removal.
  // There is no removal notification as it's delete'd on removal.
  virtual bool NodeWillRemove(TreeGridNode* node) = 0;

  // Returning false disallows editing. Only column 0 is editable.
  virtual bool NodeWillStartEdit(TreeGridNode* node) = 0;
  // Returning false reverts to original text. Only column 0 is editable.
  virtual bool NodeWillCompleteEdit(TreeGridNode* node) = 0;

  // false disallows insert.
  virtual bool NodeWillInsert(TreeGridNode* node) = 0;
  // Notification only.
  virtual void NodeInserted(TreeGridNode* node) = 0;
};

// Hierarchical view, with columns.
class TreeGrid : public Widget {
 public:
  TreeGrid();
  virtual ~TreeGrid();

  std::vector<TreeGridNode*>* Nodes();
  std::vector<TreeGridColumn*>* Columns();

  virtual void Render() override;

  virtual bool CouldStartDrag(DragSetup* drag_setup) override;

  virtual bool WantMouseEvents() override { return true; }
  virtual bool WantKeyEvents() override { return true; }
  virtual bool NotifyKey(Key::Enum key, bool down, uint8_t modifiers) override;
  virtual bool NotifyMouseButton(int x,
                                 int y,
                                 MouseButton::Enum button,
                                 bool down,
                                 uint8_t modifiers) override;

  std::vector<float> GetColumnWidths(float layout_in_width) const;

  // Unset is read-only.
  void SetEditObserver(std::unique_ptr<TreeGridEditObserver> observer);

  enum FocusDirection {
    kFocusUp,
    kFocusDown,
    kFocusLeft,
    kFocusRight,
  };
  void MoveFocusByDirection(FocusDirection direction);

  // Always focused node.
  void TryStartEdit();

  const TreeGridNode* GetFocusedNode() const { return focused_node_; }
  TreeGridNode* GetFocusedNode() { return focused_node_; }

 private:
  struct LayoutData {
    struct RectAndNode {
      RectAndNode(const Rect& rect, TreeGridNode* node)
          : rect(rect), node(node) {}
      Rect rect;
      TreeGridNode* node;
    };

    struct RectNodeAndIndex {
      RectNodeAndIndex(const Rect& rect, TreeGridNode* node, int index)
          : rect(rect), node(node), index(index) {}
      Rect rect;
      TreeGridNode* node;
      int index;
    };

    std::vector<RectNodeAndIndex> cells;
    std::vector<RectAndNode> expansion_boxes;
    std::vector<float> column_splitters;
    std::vector<float> column_widths;
    std::vector<Rect> header_columns;
    Rect margin;
    Rect header;
    Rect body;
    Rect focus;
  };

  TreeGridNode* focused_node_;

  std::unique_ptr<TreeGridEditObserver> edit_observer_;
  // TODO(scottmg): This should eventually be a pluggable thing.
  std::unique_ptr<TextEdit> inline_edit_;

  LayoutData CalculateLayout(const Rect& client_rect);
  void CalculateLayoutNodes(const std::vector<TreeGridNode*>& nodes,
                            const std::vector<float>& column_widths,
                            const float depth_per_indent,
                            float current_indent,
                            float* y_position,
                            TreeGrid::LayoutData* layout_data);

  TreeGridNode* GetLastVisibleChild(TreeGridNode* root);
  TreeGridNode* GetNextVisibleInDirection(TreeGridNode* node,
                                          FocusDirection direction);
  TreeGridNode* GetSibling(TreeGridNode* node, int direction);

  std::vector<TreeGridNode*> nodes_;
  std::vector<TreeGridColumn*> columns_;

  CORE_DISALLOW_COPY_AND_ASSIGN(TreeGrid);
};

#endif  // TREE_GRID_H_
