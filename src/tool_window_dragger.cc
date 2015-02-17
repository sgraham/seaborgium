// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tool_window_dragger.h"

#include <algorithm>

#include "docking_split_container.h"
#include "docking_tool_window.h"
#include "docking_workspace.h"
#include "gfx.h"
#include "skin.h"
#pragma message("todo")
// #include "sg/workspace.h"

namespace {

float kDetachedScale = 0.8f;
#if 0
float kHoveringAlpha = 0.75f;
float kDropTargetAlpha = 0.6f;
#endif

float IconWidth(Icon icon) {
  float w, h;
  GfxIconSize(icon, &w, &h);
  return w;
}

float IconHeight(Icon icon) {
  float w, h;
  GfxIconSize(icon, &w, &h);
  return h;
}

DropTargetIndicator IndicatorAt(Widget* dockable,
                                Icon icon,
                                float x,
                                float y,
                                DockingSplitDirection direction,
                                bool this_dockable_first) {
  DropTargetIndicator target;
  target.dockable = dockable;
  target.icon = icon;
  float w, h;
  GfxIconSize(icon, &w, &h);
  target.rect = Rect(x, y, IconWidth(icon), IconHeight(icon));
  target.direction = direction;
  target.this_dockable_first = this_dockable_first;
  return target;
}

void PlaceIndicatorsAroundEdge(const Rect& rect,
                               std::vector<DropTargetIndicator>* into,
                               Widget* dockable) {
  into->push_back(
      IndicatorAt(dockable,
                  Icon::kDockTop,
                  rect.x + rect.w / 2 - IconWidth(Icon::kDockTop) / 2,
                  rect.y,
                  kSplitHorizontal,
                  false));
  into->push_back(
      IndicatorAt(dockable,
                  Icon::kDockLeft,
                  rect.x,
                  rect.y + rect.h / 2 - IconHeight(Icon::kDockLeft) / 2,
                  kSplitVertical,
                  false));
  into->push_back(
      IndicatorAt(dockable,
                  Icon::kDockRight,
                  rect.x + rect.w - IconWidth(Icon::kDockRight),
                  rect.y + rect.h / 2 - IconHeight(Icon::kDockRight) / 2,
                  kSplitVertical,
                  true));
  into->push_back(
      IndicatorAt(dockable,
                  Icon::kDockBottom,
                  rect.x + rect.w / 2 - IconWidth(Icon::kDockBottom) / 2,
                  rect.y + rect.h - IconHeight(Icon::kDockBottom),
                  kSplitHorizontal,
                  true));
}

void PlaceIndicatorsAtCenter(const Rect& rect,
                             std::vector<DropTargetIndicator>* into,
                             Widget* dockable) {
  float cx = rect.x + rect.w / 2;
  float cy = rect.y + rect.h / 2;
  float offset = IconWidth(Icon::kDockTop);
  into->push_back(IndicatorAt(dockable,
                              Icon::kDockTop,
                              cx - offset / 2,
                              cy - offset * 2,
                              kSplitHorizontal,
                              false));
  into->push_back(IndicatorAt(dockable,
                              Icon::kDockLeft,
                              cx - offset * 2,
                              cy - offset / 2,
                              kSplitVertical,
                              false));
  into->push_back(IndicatorAt(dockable,
                              Icon::kDockRight,
                              cx + offset,
                              cy - offset / 2,
                              kSplitVertical,
                              true));
  into->push_back(IndicatorAt(dockable,
                              Icon::kDockBottom,
                              cx - offset / 2,
                              cy + offset,
                              kSplitHorizontal,
                              true));
}

}  // namespace

ToolWindowDragger::ToolWindowDragger(DockingToolWindow* dragging,
                                     DragSetup* drag_setup)
    : on_drop_target_(NULL), docking_workspace_(drag_setup->docking_workspace) {
  pick_up_offset_ = dragging->ScreenToClient(drag_setup->screen_position);
  initial_screen_rect_ = dragging->GetScreenRect();
  current_position_ = drag_setup->screen_position;

  // Save sibling, split direction, and fraction for cancel.
  DockingSplitContainer* dragging_parent_as_container =
      dragging->parent()->AsDockingSplitContainer();
  cancel_sibling_ = dragging_parent_as_container->GetSiblingOf(dragging);
  cancel_direction_ = dragging_parent_as_container->direction();
  cancel_fraction_ = dragging_parent_as_container->fraction();
  cancel_was_primary_ = dragging_parent_as_container->left() == dragging;

  // Remove from tree
  dragging_parent_as_container->ReleaseChild(dragging);
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
  std::vector<Widget*> dock_targets =
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
    dragging_->parent()->AsDockingSplitContainer()->ReleaseChild(
        dragging_.get());
    on_drop_target_ = NULL;
    dragging_->SetScreenRect(initial_screen_rect_);
  }

  for (size_t i = 0; i < targets_.size(); ++i) {
    DropTargetIndicator& dti = targets_[i];
    if (dti.rect.Contains(current_position_)) {
      on_drop_target_ = &dti;
      Widget* primary = dti.dockable;
      Widget* secondary = dragging_.get();
      if (!dti.this_dockable_first)
        std::swap(primary, secondary);
      dti.dockable->parent()->AsDockingSplitContainer()->SplitChild(
          dti.direction, primary, secondary);
      // Splitting/inserting sets the screen rect for us.
      break;
    }
  }
}

void ToolWindowDragger::CancelDrag() {
  // Reinsert at "old" location, based on saved information.
  Widget* primary = cancel_sibling_;
  Widget* secondary = dragging_.release();
  if (cancel_was_primary_)
    std::swap(primary, secondary);
  cancel_sibling_->parent()->AsDockingSplitContainer()->SplitChild(
      cancel_direction_, primary, secondary);
  cancel_sibling_->parent()->AsDockingSplitContainer()->SetFraction(
      cancel_fraction_);
}

void ToolWindowDragger::Render() {
  // TODO(scottmg): nanovg doesn't currently support render to texture
  // https://github.com/memononen/nanovg/issues/90 so we just do simple
  // outline box for now.
#if 0  // R-T-T
  // TODO(rendering): Not much practical reason to re-render this every frame
  // during drag. Investigate if it makes anything snappier if it's cached
  // after the first render.
  scoped_ptr<RenderToTextureRenderer> render_to_texture_renderer(
      renderer->CreateRenderToTextureRenderer(dragging_->GetScreenRect().w,
        dragging_->GetScreenRect().h));
  dragging_->Render(render_to_texture_renderer.get());
#endif

  for (const auto& dti : targets_)
    GfxDrawIcon(dti.icon, dti.rect, .7f);

  Rect draw_rect;
  if (on_drop_target_) {
    draw_rect = dragging_->GetScreenRect();
  } else {
    Point draw_at = current_position_.Subtract(
        pick_up_offset_.Scale(kDetachedScale));
    draw_rect = Rect(draw_at.x,
                     draw_at.y,
                     dragging_->GetClientRect().w * kDetachedScale,
                     dragging_->GetClientRect().h * kDetachedScale);
  }
#if 0  // R-T-T
  renderer->DrawRenderToTextureResult(
      render_to_texture_renderer.get(),
      draw_rect,
      kHoveringAlpha,
      0.f, 0.f, 1.f, 1.f);
  renderer->SetDrawColor(Color(0, 128, 128, kHoveringAlpha * 128));
  renderer->DrawFilledRect(draw_rect);
#else
  DrawSolidRect(draw_rect, Color(1, 1, 1, .25f));
#endif

  // TODO(scottmg): Workspace::Invalidate();
}
