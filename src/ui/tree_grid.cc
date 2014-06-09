// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/tree_grid.h"

#include <algorithm>

#include "core/gfx.h"
#include "nanovg.h"
#include "ui/skin.h"

TreeGridNodeValue::~TreeGridNodeValue() {
}

// --------------------------------------------------------------------
TreeGridNodeValueString::TreeGridNodeValueString(const std::string& value)
    : value_(value) {}

void TreeGridNodeValueString::Render() {
  // TODO: draw
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
  // TODO: nodes_ cleanup.
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

const TreeGridNodeValue* TreeGridNode::GetValue(int column) {
  std::map<int, TreeGridNodeValue*>::iterator i = items_.find(column);
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
  nvgSave(core::VG);
  nvgFontSize(core::VG, 14.f);  // TODO(font)
  nvgFontFace(core::VG, "sans");

  float ascender, descender, line_height;
  nvgTextMetrics(core::VG, &ascender, &descender, &line_height);

  nvgBeginPath(core::VG);
  const Rect& client_rect = GetClientRect();
  const ColorScheme& cs = Skin::current().GetColorScheme();
  nvgRect(core::VG,
          static_cast<float>(client_rect.x),
          static_cast<float>(client_rect.y),
          static_cast<float>(client_rect.w),
          static_cast<float>(client_rect.h));
  nvgFillColor(core::VG, cs.background());
  nvgFill(core::VG);

  const int kMarginWidth = static_cast<int>(line_height - descender);
  const int kPaddingFromMarginToBody = 3;
  Rect margin(0, 0, kMarginWidth, client_rect.h);
  Rect body(kMarginWidth + kPaddingFromMarginToBody,
            0,
            client_rect.w - (kMarginWidth + kPaddingFromMarginToBody),
            client_rect.h);

  nvgBeginPath(core::VG);
  nvgRect(core::VG,
          static_cast<float>(margin.x),
          static_cast<float>(margin.y),
          static_cast<float>(margin.w),
          static_cast<float>(margin.h));
  nvgFillColor(core::VG, cs.margin());
  nvgFill(core::VG);

  nvgFillColor(core::VG, cs.text());
  nvgText(core::VG,
          static_cast<float>(body.x),
          static_cast<float>(body.y + (line_height - descender)),
          "ui/stuff.cc, line 34",
          NULL);

  nvgRestore(core::VG);
}

std::vector<int> TreeGrid::GetColumnWidths(int layout_in_width) const {
  float total_fraction = 0.f;
  std::vector<int> ret(columns_.size());
  int remaining_width = layout_in_width;
  for (size_t j = 0; j < columns_.size(); ++j) {
    TreeGridColumn* i = columns_[j];
    if (i->width_fixed_ == -1) {
      total_fraction += i->width_fraction_;
    } else {
      remaining_width -= i->width_fixed_;
      ret[j] = std::min(layout_in_width, i->width_fixed_);
    }
  }

  if (remaining_width < 0)
    remaining_width = 0;

  for (size_t j = 0; j < columns_.size(); ++j) {
    TreeGridColumn* i = columns_[j];
    if (i->width_fixed_ == -1) {
      ret[j] = static_cast<int>((i->width_fraction_ / total_fraction) *
                                remaining_width);
    }
  }
  return ret;
}
