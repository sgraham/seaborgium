// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_SKIN_H_
#define UI_SKIN_H_

#include "core/core.h"
#include "core/gfx.h"

class ColorScheme {
 public:
  // TODO(config): Configuration from .json or something.
  ColorScheme();
  const core::Color& border() const { return border_; }
  const core::Color& background() const { return background_; }
  const core::Color& text() const { return text_; }
  const core::Color& text_selection() const { return text_selection_; }
  const core::Color& cursor() const { return cursor_; }
  const core::Color& title_bar_active_inner() const {
    return title_bar_active_inner_;
  }
  const core::Color& title_bar_active_outer() const {
    return title_bar_active_outer_;
  }
  const core::Color& title_bar_inactive_inner() const {
    return title_bar_inactive_inner_;
  }
  const core::Color& title_bar_inactive_outer() const {
    return title_bar_inactive_outer_;
  }
  const core::Color& title_bar_text_active() const {
    return title_bar_text_active_;
  }
  const core::Color& title_bar_text_inactive() const {
    return title_bar_text_inactive_;
  }
  const core::Color& title_bar_text_drop_shadow() const {
    return title_bar_text_drop_shadow_;
  }
  const core::Color& drop_shadow_inner() const { return drop_shadow_inner_; }
  const core::Color& drop_shadow_outer() const { return drop_shadow_outer_; }
  const core::Color& drop_indicator() const { return drop_indicator_; }

  const core::Color& comment() const { return comment_; }
  const core::Color& comment_preprocessor() const { return comment_preprocessor_; }
  const core::Color& error() const { return error_; }
  const core::Color& keyword() const { return keyword_; }
  const core::Color& keyword_type() const { return keyword_type_; }
  const core::Color& literal_number() const { return literal_number_; }
  const core::Color& literal_string() const { return literal_string_; }
  const core::Color& klass() const { return klass_; }
  const core::Color& op() const { return op_; }

  const core::Color& margin() const { return margin_; }
  const core::Color& margin_text() const { return margin_text_; }

  const core::Color& pc_indicator() const { return pc_indicator_; }

 private:
  core::Color border_;
  core::Color background_;
  core::Color text_;
  core::Color text_selection_;
  core::Color cursor_;
  core::Color title_bar_active_inner_;
  core::Color title_bar_active_outer_;
  core::Color title_bar_inactive_inner_;
  core::Color title_bar_inactive_outer_;
  core::Color title_bar_text_active_;
  core::Color title_bar_text_inactive_;
  core::Color title_bar_text_drop_shadow_;
  core::Color drop_shadow_inner_;
  core::Color drop_shadow_outer_;
  core::Color drop_indicator_;

  core::Color comment_;
  core::Color comment_preprocessor_;
  core::Color error_;
  core::Color keyword_;
  core::Color keyword_type_;
  core::Color literal_number_;
  core::Color literal_string_;
  core::Color klass_;
  core::Color op_;

  core::Color margin_;
  core::Color margin_text_;

  core::Color pc_indicator_;
};

class Skin {
 public:
  Skin();
  void Init();

  const ColorScheme& GetColorScheme() const { return color_scheme_; }

  float title_bar_size() const { return title_bar_size_; }
  float title_bar_text_size() const { return title_bar_text_size_; }
  float border_size() const { return border_size_; }
  float status_bar_size() const { return status_bar_size_; }
  float text_line_height() const { return text_line_height_; }

  const char* dock_top_icon() const { return dock_top_icon_; }
  const char* dock_left_icon() const { return dock_left_icon_; }
  const char* dock_right_icon() const { return dock_right_icon_; }
  const char* dock_bottom_icon() const { return dock_bottom_icon_; }
  // TODO(icons): These will all be chars from icons font.
#if 0
  const Texture* pc_indicator_texture() const { return &pc_indicator_texture_; }
  const Texture* breakpoint_texture() const { return &breakpoint_texture_; }
  const Texture* tree_collapsed_texture() const {
    return  &tree_collapsed_texture_;
  }
  const Texture* tree_expanded_texture() const {
    return  &tree_expanded_texture_;
  }
#endif

  static const Skin& current();
  static void LoadData();

 private:
  ColorScheme color_scheme_;

  float title_bar_size_;
  float title_bar_text_size_;
  float border_size_;
  float status_bar_size_;
  float text_line_height_;

  const char* dock_top_icon_;
  const char* dock_left_icon_;
  const char* dock_right_icon_;
  const char* dock_bottom_icon_;
#if 0
  Texture pc_indicator_texture_;
  Texture breakpoint_texture_;
  Texture vscrollbar_top_texture_;
  Texture vscrollbar_middle_texture_;
  Texture vscrollbar_bottom_texture_;
  Texture tree_collapsed_texture_;
  Texture tree_expanded_texture_;
#endif

  CORE_DISALLOW_COPY_AND_ASSIGN(Skin);
};

#endif  // UI_SKIN_H_
