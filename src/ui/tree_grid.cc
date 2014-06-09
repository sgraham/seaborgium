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
  nvgRect(core::VG, client_rect.x, client_rect.y, client_rect.w, client_rect.h);
  nvgFillColor(core::VG, cs.background());
  nvgFill(core::VG);

  const float kMarginWidth = line_height - descender;
  const float kHeaderHeight = kMarginWidth;
  const float kPaddingFromMarginToBody = 3;
  const float kPaddingFromHeaderToBody = 3;
  const float kTitlePadding = 3;

  Rect margin(0, kHeaderHeight, kMarginWidth, client_rect.h - kHeaderHeight);
  Rect header(kMarginWidth, 0, client_rect.w - kMarginWidth, kHeaderHeight);

  Rect body(kMarginWidth + kPaddingFromMarginToBody,
            kHeaderHeight + kPaddingFromHeaderToBody,
            client_rect.w - (kMarginWidth + kPaddingFromMarginToBody),
            client_rect.h - (kHeaderHeight + kPaddingFromHeaderToBody));

  nvgBeginPath(core::VG);
  nvgRect(core::VG, margin.x, margin.y, margin.w, margin.h);
  nvgFillColor(core::VG, cs.margin());
  nvgFill(core::VG);

  nvgBeginPath(core::VG);
  nvgRect(core::VG, header.x, header.y, header.w, header.h);
  nvgFillColor(core::VG, cs.margin());
  nvgFill(core::VG);

  std::vector<float> column_widths = GetColumnWidths(body.w);
  nvgFillColor(core::VG, cs.margin_text());
  nvgStrokeColor(core::VG, cs.border());
  CORE_DCHECK(columns_.size() == column_widths.size(), "num columns broken");
  float last_x = 0;
  nvgBeginPath(core::VG);
  nvgMoveTo(core::VG, header.x + last_x, header.y);
  nvgLineTo(core::VG, header.x + last_x, client_rect.h);
  nvgStroke(core::VG);
  for (size_t i = 0; i < columns_.size(); ++i) {
    // TODO: clip
    nvgText(core::VG,
            header.x + last_x + kTitlePadding,
            header.y + line_height,
            columns_[i]->GetCaption().c_str(),
            NULL);
    last_x += column_widths[i];
    nvgBeginPath(core::VG);
    nvgMoveTo(core::VG, header.x + last_x, header.y);
    nvgLineTo(core::VG, header.x + last_x, client_rect.h - header.y);
    nvgStroke(core::VG);
  }

  /*
  nvgFillColor(core::VG, cs.text());
  nvgText(core::VG,
          body.x,
          body.y + (line_height - descender),
          "ui/stuff.cc, line 34",
          NULL);
          */

  nvgRestore(core::VG);
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
