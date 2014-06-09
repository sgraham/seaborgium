// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/tree_grid.h"

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
    : tree_grid_(tree_grid), parent_(parent), expanded_(false) {}

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
