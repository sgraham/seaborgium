// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/tool_window_dragger.h"

#include <algorithm>

#include "ui/docking_split_container.h"
#include "ui/docking_tool_window.h"
#include "ui/docking_workspace.h"
#include "ui/skin.h"
#pragma message("todo")
//#include "sg/workspace.h"

namespace {

float kDetachedScale = 0.8f;
float kHoveringAlpha = 0.75f;
float kDropTargetAlpha = 0.6f;

DropTargetIndicator IndicatorAt(
    Dockable* dockable,
    const Texture* texture,
    int x, int y,
    DockingSplitDirection direction,
    bool this_dockable_first) {
  DropTargetIndicator target;
  CORE_CHECK(false, "");
  CORE_UNUSED(dockable);
  CORE_UNUSED(texture);
  CORE_UNUSED(x);
  CORE_UNUSED(y);
  CORE_UNUSED(direction);
  CORE_UNUSED(this_dockable_first);
#if 0
  target.dockable = dockable;
  target.texture = texture;
  target.rect = Rect(x, y, texture->width, texture->height);
  target.direction = direction;
  target.this_dockable_first = this_dockable_first;
#endif
  return target;
}

void PlaceIndicatorsAroundEdge(
    const Rect& rect,
    std::vector<DropTargetIndicator>* into,
    Dockable* dockable) {
  CORE_UNUSED(rect);
  CORE_UNUSED(into);
  CORE_UNUSED(dockable);
#if 0
  const Skin& skin = Skin::current();
  into->push_back(IndicatorAt(
      dockable,
      skin.dock_top_texture(),
      rect.x + rect.w / 2 - skin.dock_top_texture()->width / 2,
      rect.y,
      kSplitHorizontal, false));
  into->push_back(IndicatorAt(
      dockable,
      skin.dock_left_texture(),
      rect.x,
      rect.y + rect.h / 2 - skin.dock_left_texture()->height / 2,
      kSplitVertical, false));
  into->push_back(IndicatorAt(
      dockable,
      skin.dock_right_texture(),
      rect.x + rect.w - skin.dock_right_texture()->width,
      rect.y + rect.h / 2 - skin.dock_right_texture()->height / 2,
      kSplitVertical, true));
  into->push_back(IndicatorAt(
      dockable,
      skin.dock_bottom_texture(),
      rect.x + rect.w / 2 - skin.dock_bottom_texture()->width / 2,
      rect.y + rect.h - skin.dock_bottom_texture()->height,
      kSplitHorizontal, true));
#endif
}

void PlaceIndicatorsAtCenter(
    const Rect& rect,
    std::vector<DropTargetIndicator>* into,
    Dockable* dockable) {
  CORE_UNUSED(rect);
  CORE_UNUSED(into);
  CORE_UNUSED(dockable);
#if 0
  const Skin& skin = Skin::current();
  int cx = rect.x + rect.w / 2;
  int cy = rect.y + rect.h / 2;
  into->push_back(IndicatorAt(
      dockable,
      skin.dock_top_texture(),
      cx - skin.dock_top_texture()->width / 2,
      cy - skin.dock_top_texture()->height * 2,
      kSplitHorizontal, false));
  into->push_back(IndicatorAt(
      dockable,
      skin.dock_left_texture(),
      cx - skin.dock_left_texture()->height * 2,
      cy - skin.dock_left_texture()->height / 2,
      kSplitVertical, false));
  into->push_back(IndicatorAt(
      dockable,
      skin.dock_right_texture(),
      cx + skin.dock_right_texture()->width,
      cy - skin.dock_right_texture()->height / 2,
      kSplitVertical, true));
  into->push_back(IndicatorAt(
      dockable,
      skin.dock_bottom_texture(),
      cx - skin.dock_bottom_texture()->width / 2,
      cy + skin.dock_bottom_texture()->height,
      kSplitHorizontal, true));
#endif
}

}  // namespace

ToolWindowDragger::ToolWindowDragger(
    DockingToolWindow* dragging,
    DragSetup* drag_setup)
    : on_drop_target_(NULL),
      docking_workspace_(drag_setup->docking_workspace) {
  pick_up_offset_ = dragging->ToClient(drag_setup->screen_position);
  initial_screen_rect_ = dragging->GetScreenRect();
  current_position_ = drag_setup->screen_position;

  // Save sibling, split direction, and fraction for cancel.
  cancel_sibling_ = dragging->parent()->GetSiblingOf(dragging);
  cancel_direction_ = dragging->parent()->direction();
  cancel_fraction_ = dragging->parent()->fraction();
  cancel_was_primary_ = dragging->parent()->left() == dragging;

  // Remove from tree
  dragging->parent()->ReleaseChild(dragging);
  dragging_.reset(dragging);

  RefreshTargets();
}

void ToolWindowDragger::RefreshTargets() {
  targets_.clear();
  // Find all possible target split containers and make a list of drag targets
  // and icons of those, plus ones for the root.
  Rect workspace_rect = docking_workspace_->GetScreenRect();
  PlaceIndicatorsAroundEdge(
      workspace_rect, &targets_, docking_workspace_->GetRoot());
  std::vector<Dockable*> dock_targets =
      docking_workspace_->GetAllDockTargets();
  for (size_t i = 0; i < dock_targets.size(); ++i) {
    if (dock_targets[i] != dragging_.get()) {
      PlaceIndicatorsAtCenter(
          dock_targets[i]->GetScreenRect(), &targets_, dock_targets[i]);
    }
  }
}

ToolWindowDragger::~ToolWindowDragger() {
  // We were added into a tree.
  if (on_drop_target_ && dragging_.get())
    dragging_.release();
  else
    CancelDrag();
}

void ToolWindowDragger::Drag(const Point& screen_point) {
  current_position_ = screen_point;

  // We're repeatedly removing/inserting on every move, which is kind of
  // silly. Probably simpler than trying to cache more state information here
  // though.

  if (on_drop_target_) {
    dragging_->parent()->ReleaseChild(dragging_.get());
    on_drop_target_ = NULL;
    dragging_->SetScreenRect(initial_screen_rect_);
  }

  for (size_t i = 0; i < targets_.size(); ++i) {
    DropTargetIndicator& dti = targets_[i];
    if (dti.rect.Contains(current_position_)) {
      on_drop_target_ = &dti;
      Dockable* primary = dti.dockable;
      Dockable* secondary = dragging_.get();
      if (!dti.this_dockable_first)
        std::swap(primary, secondary);
      dti.dockable->parent()->SplitChild(dti.direction, primary, secondary);
      // Splitting/inserting sets the screen rect for us.
      break;
    }
  }
}

void ToolWindowDragger::CancelDrag() {
  // Reinsert at "old" location, based on saved information.
  Dockable* primary = cancel_sibling_;
  Dockable* secondary = dragging_.release();
  if (cancel_was_primary_)
    std::swap(primary, secondary);
  cancel_sibling_->parent()->SplitChild(cancel_direction_, primary, secondary);
  cancel_sibling_->parent()->SetFraction(cancel_fraction_);
}

void ToolWindowDragger::Render() {
#if 0
  // TODO(rendering): Not much practical reason to re-render this every frame
  // during drag. Investigate if it makes anything snappier if it's cached
  // after the first render.
  scoped_ptr<RenderToTextureRenderer> render_to_texture_renderer(
      renderer->CreateRenderToTextureRenderer(
          dragging_->GetScreenRect().w,
          dragging_->GetScreenRect().h));
  dragging_->Render(render_to_texture_renderer.get());

  for (size_t i = 0; i < targets_.size(); ++i) {
    const DropTargetIndicator& dti = targets_[i];
    renderer->DrawTexturedRectAlpha(
        dti.texture, dti.rect, kDropTargetAlpha, 0, 0, 1, 1);
  }

  Rect draw_rect;
  if (on_drop_target_) {
    draw_rect = dragging_->GetScreenRect();
  } else {
    Point draw_at = current_position_.Subtract(
        pick_up_offset_.Scale(kDetachedScale));
    draw_rect = Rect(
        draw_at.x, draw_at.y,
        static_cast<int>(dragging_->GetClientRect().w * kDetachedScale),
        static_cast<int>(dragging_->GetClientRect().h * kDetachedScale));
  }
  renderer->DrawRenderToTextureResult(
      render_to_texture_renderer.get(),
      draw_rect,
      kHoveringAlpha,
      0.f, 0.f, 1.f, 1.f);
  renderer->SetDrawColor(Color(0, 128, 128, kHoveringAlpha * 128));
  renderer->DrawFilledRect(draw_rect);

  Workspace::Invalidate();
#endif
}
