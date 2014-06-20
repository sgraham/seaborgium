// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/tree_grid.h"

#include <algorithm>

#include "core/gfx.h"
#include "nanovg.h"
#include "ui/draggable.h"
#include "ui/drawing_common.h"
#include "ui/skin.h"

namespace {
const float kTextPadding = 3;
}

TreeGridNodeValue::~TreeGridNodeValue() {
}

// --------------------------------------------------------------------
TreeGridNodeValueString::TreeGridNodeValueString(const std::string& value)
    : value_(value) {}

void TreeGridNodeValueString::Render(const Rect& rect) const {
  const ColorScheme& cs = Skin::current().GetColorScheme();
  DrawTextInRect(rect, value_, cs.text(), kTextPadding);
}

// --------------------------------------------------------------------
TreeGridNode::TreeGridNode(TreeGrid* tree_grid, TreeGridNode* parent)
    : tree_grid_(tree_grid),
      parent_(parent),
      expanded_(false),
      selected_(false) {}

TreeGridNode::~TreeGridNode() {
  for (std::map<int, TreeGridNodeValue*>::iterator i(items_.begin());
       i != items_.end();
       ++i) {
    delete i->second;
  }
  // TODO(scottmg): nodes_ cleanup.
}

const std::vector<TreeGridNode*>* TreeGridNode::Nodes() const {
  return &nodes_;
}

std::vector<TreeGridNode*>* TreeGridNode::Nodes() {
  return &nodes_;
}

void TreeGridNode::SetValue(int column, TreeGridNodeValue* value) {
  std::map<int, TreeGridNodeValue*>::iterator i = items_.find(column);
  if (i != items_.end())
    delete i->second;
  items_[column] = value;
}

const TreeGridNodeValue* TreeGridNode::GetValue(int column) const {
  std::map<int, TreeGridNodeValue*>::const_iterator i = items_.find(column);
  if (i == items_.end())
    return NULL;
  return i->second;
}

// --------------------------------------------------------------------
TreeGridColumn::TreeGridColumn(TreeGrid* tree_grid, const std::string& caption)
    : tree_grid_(tree_grid), caption_(caption) {}

void TreeGridColumn::SetWidthPercentage(float fraction) {
  width_fraction_ = fraction;
}

void TreeGridColumn::SetPercentageToMatchWidth(float width, float whole_width) {
  // Not really wrong, but doesn't do anything when percentage-based.
  if (tree_grid_->Columns()->size() == 1)
    return;
  std::vector<TreeGridColumn*>::iterator self = std::find(
      tree_grid_->Columns()->begin(), tree_grid_->Columns()->end(), this);
  CORE_CHECK(self != tree_grid_->Columns()->end(), "couldn't find column");
  size_t self_index = self - tree_grid_->Columns()->begin();
  CORE_CHECK(self_index < tree_grid_->Columns()->size() - 1,
             "shouldn't be used on the last column");
  std::vector<float> column_widths = tree_grid_->GetColumnWidths(whole_width);

  // Add/remove the width to our neighbour.
  float delta = column_widths[self_index] - width;
  column_widths[self_index] = width;
  column_widths[self_index + 1] += delta;

  // And rebalance the two percentages for this column and the neighbour based
  // on those new widths.
  float two_column_total_fraction =
      width_fraction_ +
      tree_grid_->Columns()->at(self_index + 1)->width_fraction_;
  float two_column_total_width =
      column_widths[self_index] + column_widths[self_index + 1];
  width_fraction_ = column_widths[self_index] / two_column_total_width *
                    two_column_total_fraction;
  tree_grid_->Columns()->at(self_index + 1)->width_fraction_ =
      column_widths[self_index + 1] / two_column_total_width *
      two_column_total_fraction;
}

void TreeGridColumn::SetPercentageToMatchPosition(float splitter_position,
                                                  float whole_width) {
  std::vector<TreeGridColumn*>::iterator self = std::find(
      tree_grid_->Columns()->begin(), tree_grid_->Columns()->end(), this);
  CORE_CHECK(self != tree_grid_->Columns()->end(), "couldn't find column");
  size_t self_index = self - tree_grid_->Columns()->begin();
  CORE_CHECK(self_index < tree_grid_->Columns()->size() - 1,
             "shouldn't be used on the last column");
  std::vector<float> column_widths = tree_grid_->GetColumnWidths(whole_width);

  float last_x = 0.f;
  for (size_t i = 0; i < self_index; ++i)
    last_x += column_widths[i];
  float max_width = column_widths[self_index] + column_widths[self_index + 1] -
                    Skin::current().border_size();
  float new_width = std::min(
      max_width,
      std::max(Skin::current().border_size(), splitter_position - last_x));

  SetPercentageToMatchWidth(new_width, whole_width);
}

// --------------------------------------------------------------------
TreeGrid::TreeGrid() : focused_node_(NULL) {
}

TreeGrid::~TreeGrid() {
}

std::vector<TreeGridNode*>* TreeGrid::Nodes() {
  return &nodes_;
}

std::vector<TreeGridColumn*>* TreeGrid::Columns() {
  return &columns_;
}

TreeGrid::LayoutData TreeGrid::CalculateLayout(const Rect& client_rect) {
  TreeGrid::LayoutData ret;

  ScopedSansSetup text_setup;

  float ascender, descender, line_height;
  nvgTextMetrics(core::VG, &ascender, &descender, &line_height);

  const float kMarginWidth = line_height - descender;
  const float kHeaderHeight = kMarginWidth + 5;

  ret.margin =
      Rect(0, kHeaderHeight, kMarginWidth, client_rect.h - kHeaderHeight);
  ret.header =
      Rect(kMarginWidth, 0, client_rect.w - kMarginWidth, kHeaderHeight);

  ret.body = Rect(kMarginWidth,
                  kHeaderHeight,
                  client_rect.w - kMarginWidth,
                  client_rect.h - kHeaderHeight);

  ret.column_widths = GetColumnWidths(ret.body.w);
  CORE_DCHECK(columns_.size() == ret.column_widths.size(),
              "num columns broken");

  float last_x = 0;
  for (size_t i = 0; i < columns_.size(); ++i) {
    Rect header_column = ret.header;
    header_column.x = ret.header.x + last_x;
    header_column.w = ret.column_widths[i];
    ret.header_columns.push_back(header_column);
    last_x += ret.column_widths[i];
    float splitter_x = ret.header.x + last_x;
    ret.column_splitters.push_back(splitter_x);
  }

  float y_position = 0.f;
  CalculateLayoutNodes(
      *Nodes(), ret.column_widths, kMarginWidth, 0.f, &y_position, &ret);

  return ret;
}

void TreeGrid::CalculateLayoutNodes(const std::vector<TreeGridNode*>& nodes,
                                    const std::vector<float>& column_widths,
                                    const float depth_per_indent,
                                    float current_indent,
                                    float* y_position,
                                    TreeGrid::LayoutData* layout_data) {
  float kLineHeight = depth_per_indent;  // TODO(scottmg): Wrong height, just
                                         // happens to be about right.
  for (size_t i = 0; i < nodes.size(); ++i) {
    TreeGridNode* node = nodes[i];
    float last_x = 0.f;
    for (size_t j = 0; j < column_widths.size(); ++j) {
      float x = last_x;
      if (j == 0) {
        if (node->Nodes()->size() > 0) {
          layout_data->expansion_boxes.push_back(LayoutData::RectAndNode(
              Rect(current_indent + layout_data->margin.w,
                   *y_position + layout_data->header.h,
                   kLineHeight,
                   kLineHeight),
              node));
        }

        // Then space for the text.
        const float kIndicatorWidth = kLineHeight;
        x = current_indent + kIndicatorWidth;
      }
      // -1 on width for column separator.
      float adjusted_width =
          std::max(0.f, column_widths[j] - 1.f - (x - last_x));
      last_x += column_widths[j];
      Rect box(x + layout_data->margin.w,
               *y_position + layout_data->header.h,
               adjusted_width,
               kLineHeight);
      layout_data->cells.push_back(LayoutData::RectNodeAndIndex(box, node, j));
    }
    *y_position += kLineHeight;
    if (node->Expanded()) {
      CalculateLayoutNodes(*node->Nodes(),
                           column_widths,
                           depth_per_indent,
                           current_indent + depth_per_indent,
                           y_position,
                           layout_data);
    }
  }
}

void TreeGrid::Render() {
  ScopedSansSetup text_setup;
  const Rect& client_rect = GetClientRect();
  const LayoutData& ld = CalculateLayout(client_rect);

  const ColorScheme& cs = Skin::current().GetColorScheme();
  DrawSolidRect(client_rect, cs.background());

  DrawSolidRect(ld.margin, cs.margin());
  DrawSolidRect(ld.header, cs.margin());

  DrawVerticalLine(cs.border(), ld.header.x, ld.header.y, client_rect.h);
  for (size_t i = 0; i < columns_.size(); ++i) {
    DrawTextInRect(ld.header_columns[i],
                   columns_[i]->GetCaption(),
                   cs.margin_text(),
                   kTextPadding);
    DrawVerticalLine(cs.border(),
                     ld.column_splitters[i],
                     ld.header.y,
                     client_rect.h - ld.header.y);
  }
  DrawHorizontalLine(
      cs.border(), ld.header.x, ld.header.x + ld.header.w, ld.header.h);

  for (const auto& cell : ld.cells)
    cell.node->GetValue(cell.index)->Render(cell.rect);

  {
    ScopedIconsSetup icons;
    nvgFillColor(core::VG, cs.text());
    const char* kSquaredPlus = "\xE2\x8A\x9E";
    const char* kSquaredMinus = "\xE2\x8A\x9F";
    for (const auto& button : ld.expansion_boxes) {
      nvgText(core::VG,
              button.rect.x,
              button.rect.y + button.rect.h,
              button.node->Expanded() ? kSquaredMinus : kSquaredPlus,
              NULL);
    }
  }
}

class ColumnDragHelper : public Draggable {
 public:
  ColumnDragHelper(TreeGrid* tree_grid,
                   int column,
                   float initial_position,
                   const Rect& body)
      : tree_grid_(tree_grid),
        column_(column),
        initial_position_(initial_position),
        body_(body) {}

  virtual void Drag(const Point& screen_position) override {
    Point client_point =
        screen_position.RelativeTo(tree_grid_->GetScreenRect());
    Point body_point = client_point.RelativeTo(body_);
    // N columns have N-1 draggable sizers:
    //
    // I   c0   |      c1       |    c2    I
    //          ^               ^
    // When dragging column sizer N, it's enough to modify the width of the
    // Nth column.
    tree_grid_->Columns()->at(column_)->SetPercentageToMatchPosition(
        body_point.x, body_.w);
  }

  virtual void CancelDrag() override {
  }

  virtual void Render() override {
  }

 private:
  TreeGrid* tree_grid_;
  int column_;
  float initial_position_;
  Rect body_;

  CORE_DISALLOW_COPY_AND_ASSIGN(ColumnDragHelper);
};

bool TreeGrid::CouldStartDrag(DragSetup* drag_setup) {
  // This layout calculation is pretty expensive (or would be for many nodes).
  // Removing the cache for now, but it could also be saved off in Render and
  // just reused here.
  const LayoutData& layout_data = CalculateLayout(GetClientRect());
  Point client_point = drag_setup->screen_position.RelativeTo(GetScreenRect());
  float half_width = Skin::current().border_size() / 2.f;
  for (size_t i = 0; i < layout_data.column_splitters.size(); ++i) {
    float column_x = layout_data.column_splitters[i];
    if (client_point.x > column_x - half_width &&
        client_point.x <= column_x + half_width) {
      // TODO(scottmg): Should this only be in the header?
      drag_setup->drag_direction = kDragDirectionLeftRight;
      if (drag_setup->draggable) {
        drag_setup->draggable->reset(
            new ColumnDragHelper(this, i, column_x, layout_data.body));
      }
      return true;
    }
  }
  return false;
}

bool TreeGrid::NotifyKey(core::Key::Enum key, bool down, uint8_t modifiers) {
  CORE_UNUSED(key);
  CORE_UNUSED(down);
  CORE_UNUSED(modifiers);
  return false;
}

bool TreeGrid::NotifyMouseButton(int x,
                                 int y,
                                 core::MouseButton::Enum button,
                                 bool down,
                                 uint8_t modifiers) {
  // See CouldStartDrag (this could be cached).
  const LayoutData& layout_data = CalculateLayout(GetClientRect());
  CORE_UNUSED(modifiers);
  Point client_point = Point(static_cast<float>(x), static_cast<float>(y))
                           .RelativeTo(GetScreenRect());
  if (down && button == core::MouseButton::Left) {
    for (const auto& eb : layout_data.expansion_boxes) {
      if (eb.rect.Contains(client_point)) {
        eb.node->SetExpanded(!eb.node->Expanded());
        return true;
      }
    }
  }

  return false;
}

std::vector<float> TreeGrid::GetColumnWidths(float layout_in_width) const {
  float total_fraction = 0.f;
  std::vector<float> ret(columns_.size());
  for (size_t j = 0; j < columns_.size(); ++j) {
    TreeGridColumn* i = columns_[j];
    total_fraction += i->width_fraction_;
  }

  for (size_t j = 0; j < columns_.size(); ++j) {
    TreeGridColumn* i = columns_[j];
    ret[j] = (i->width_fraction_ / total_fraction) * layout_in_width;
  }
  return ret;
}

TreeGridNode* TreeGrid::GetSibling(TreeGridNode* node, int direction) {
  // This is kind of ugly. Maybe a dummy TreeGridNode at the root would be
  // nicer.
  CORE_DCHECK(direction == -1 || direction == 1, "bad direction");
  std::vector<TreeGridNode*>* parent_nodes;
  if (!node->Parent())
    parent_nodes = Nodes();
  else
    parent_nodes = node->Parent()->Nodes();

  std::vector<TreeGridNode*>::iterator it =
      std::find(parent_nodes->begin(), parent_nodes->end(), node);
  if (it == parent_nodes->end())
    return NULL;
  if (it == parent_nodes->begin() && direction < 0)
    return node;
  if (it == parent_nodes->end() - 1 && direction > 0)
    return node;
  return *(it + direction);
}

TreeGridNode* TreeGrid::GetLastVisibleChild(TreeGridNode* root) {
  if (!root->Nodes()->empty() && root->Expanded())
    return GetLastVisibleChild(root->Nodes()->back());
  else
    return root;
}

TreeGridNode* TreeGrid::GetNextVisibleInDirection(TreeGridNode* node,
                                                  FocusDirection direction) {
  if (direction == kFocusDown) {
    // If we have children, and we're expanded, go to the first child.
    // Otherwise, to our sibling.
    if (node->Expanded() && !node->Nodes()->empty())
      return node->Nodes()->at(0);
    else
      return GetSibling(node, 1);
  } else if (direction == kFocusUp) {
    // This one's more tricky, we want our previous sibling's deepest visible
    // child.
    TreeGridNode* prev_sibling = GetSibling(node, -1);
    if (prev_sibling == node) {
      if (node->Parent())
        return node->Parent();
      return node;
    }
    return GetLastVisibleChild(prev_sibling);
  }
  CORE_CHECK(direction == kFocusDown || direction == kFocusUp,
             "should only be used for up and down");
  return NULL;
}

void TreeGrid::MoveFocusByDirection(FocusDirection direction) {
  if (!focused_node_ && (direction == kFocusDown || direction == kFocusLeft) &&
      !Nodes()->empty()) {
    focused_node_ = Nodes()->front();
    return;
  } else if (!focused_node_ &&
             (direction == kFocusUp || direction == kFocusRight) &&
             !Nodes()->empty()) {
    focused_node_ = Nodes()->back();
    return;
  }

  if (!focused_node_)
    return;

  if (direction == kFocusUp || direction == kFocusDown)
    focused_node_ = GetNextVisibleInDirection(focused_node_, direction);
}
