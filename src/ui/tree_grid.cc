// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/tree_grid.h"

#include <algorithm>

#include "core/gfx.h"
#include "nanovg.h"
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
      expanded_(true),
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
  width_fixed_ = -1;
}

void TreeGridColumn::SetWidthFixed(int size) {
  width_fraction_ = 0.f;
  width_fixed_ = size;
}

// --------------------------------------------------------------------
TreeGrid::TreeGrid() {
}

TreeGrid::~TreeGrid() {
}

std::vector<TreeGridNode*>* TreeGrid::Nodes() {
  return &nodes_;
}

std::vector<TreeGridColumn*>* TreeGrid::Columns() {
  return &columns_;
}

void TreeGrid::Render() {
  ScopedSansSetup text_setup;

  float ascender, descender, line_height;
  nvgTextMetrics(core::VG, &ascender, &descender, &line_height);

  const Rect& client_rect = GetClientRect();
  const ColorScheme& cs = Skin::current().GetColorScheme();
  DrawSolidRect(client_rect, cs.background());

  const float kMarginWidth = line_height - descender;
  const float kHeaderHeight = kMarginWidth + 5;

  Rect margin(0, kHeaderHeight, kMarginWidth, client_rect.h - kHeaderHeight);
  Rect header(kMarginWidth, 0, client_rect.w - kMarginWidth, kHeaderHeight);

  Rect body(kMarginWidth,
            kHeaderHeight,
            client_rect.w - kMarginWidth,
            client_rect.h - kHeaderHeight);

  DrawSolidRect(margin, cs.margin());
  DrawSolidRect(header, cs.margin());

  std::vector<float> column_widths = GetColumnWidths(body.w);
  CORE_DCHECK(columns_.size() == column_widths.size(), "num columns broken");
  float last_x = 0;
  DrawVerticalLine(cs.border(), header.x, header.y, client_rect.h);
  for (size_t i = 0; i < columns_.size(); ++i) {
    Rect header_column = header;
    header_column.x = header.x + last_x;
    header_column.w = column_widths[i];
    last_x += column_widths[i];
    // nanovg doesn't clip if width of scissor is 0, so we have to not render
    // in that case.
    if (header_column.w <= 1.f)
      continue;
    DrawTextInRect(header_column,
                   columns_[i]->GetCaption(),
                   cs.margin_text(),
                   kTextPadding);
    DrawVerticalLine(
        cs.border(), header.x + last_x, header.y, client_rect.h - header.y);
  }
  DrawHorizontalLine(cs.border(), header.x, header.x + header.w, header.h);

  expansion_boxes_.clear();

  const float kIndentDepth = kMarginWidth;
  float y_position = 0.f;
  ScopedRenderOffset past_header_and_margin(body, true);
  RenderNodes(*Nodes(), column_widths, kIndentDepth, 0.f, &y_position);

  for (size_t i = 0; i < expansion_boxes_.size(); ++i) {
    ExpansionBoxPosition& ebp = expansion_boxes_[i];
    // Adjust to client-relative.
    ebp.rect.x += body.x;
    ebp.rect.y += body.y;
  }
}

void TreeGrid::RenderNodes(const std::vector<TreeGridNode*>& nodes,
                           const std::vector<float>& column_widths,
                           const float depth_per_indent,
                           float current_indent,
                           float* y_position) {
  const ColorScheme& cs = Skin::current().GetColorScheme();
  float kLineHeight = depth_per_indent;  // TODO(scottmg): Wrong height, just
                                         // happens to be about right.
  for (size_t i = 0; i < nodes.size(); ++i) {
    TreeGridNode* node = nodes[i];
    float last_x = 0.f;
    for (size_t j = 0; j < column_widths.size(); ++j) {
      float x = last_x;
      if (j == 0) {
        if (node->Nodes()->size() > 0) {
          // Draw expansion indicator first.
          ScopedIconsSetup icons;
          const char* kSquaredPlus = "\xE2\x8A\x9E";
          const char* kSquaredMinus = "\xE2\x8A\x9F";
          nvgFillColor(core::VG, cs.text());
          nvgText(core::VG,
                  current_indent,
                  kLineHeight + *y_position,
                  node->Expanded() ? kSquaredMinus : kSquaredPlus,
                  NULL);
          // TODO(scottmg): Maybe measure text?
          expansion_boxes_.push_back(ExpansionBoxPosition(
              Rect(current_indent, *y_position, kLineHeight, kLineHeight),
              node));
        }

        // Then draw the text in the common case.
        const float kIndicatorWidth = kLineHeight;
        x = current_indent + kIndicatorWidth;
      }
      // -1 on width for column separator.
      float adjusted_width = column_widths[j] - 1.f - (x - last_x);
      last_x += column_widths[j];
      // nanovg doesn't actually clip if width of scissor is 0, so we have to
      // make sure not to render here.
      if (adjusted_width <= 1.f)
        continue;
      Rect box(x, *y_position, adjusted_width, kLineHeight);
      node->GetValue(j)->Render(box);
    }
    *y_position += kLineHeight;
    if (node->Expanded()) {
      RenderNodes(*node->Nodes(),
                  column_widths,
                  depth_per_indent,
                  current_indent + depth_per_indent,
                  y_position);
    }
  }
}

bool TreeGrid::NotifyMouseButton(int x,
                                 int y,
                                 core::MouseButton::Enum button,
                                 bool down,
                                 uint8_t modifiers) {
  CORE_UNUSED(modifiers);
  Point client_point = Point(static_cast<float>(x), static_cast<float>(y))
                           .RelativeTo(GetScreenRect());
  if (down && button == core::MouseButton::Left) {
    for (const auto& ebp : expansion_boxes_) {
      if (ebp.rect.Contains(client_point)) {
        ebp.node->SetExpanded(!ebp.node->Expanded());
        return true;
      }
    }
  }
  return false;
}

std::vector<float> TreeGrid::GetColumnWidths(float layout_in_width) const {
  float total_fraction = 0.f;
  std::vector<float> ret(columns_.size());
  float remaining_width = layout_in_width;
  for (size_t j = 0; j < columns_.size(); ++j) {
    TreeGridColumn* i = columns_[j];
    if (i->width_fixed_ == -1) {
      total_fraction += i->width_fraction_;
    } else {
      remaining_width -= i->width_fixed_;
      ret[j] = std::min(layout_in_width, static_cast<float>(i->width_fixed_));
    }
  }

  if (remaining_width < 0)
    remaining_width = 0;

  for (size_t j = 0; j < columns_.size(); ++j) {
    TreeGridColumn* i = columns_[j];
    if (i->width_fixed_ == -1) {
      ret[j] = (i->width_fraction_ / total_fraction) * remaining_width;
    }
  }
  return ret;
}
