// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_SKIN_H_
#define UI_SKIN_H_

#include "core/core.h"
#include "nanovg.h"

class ColorScheme {
 public:
  // TODO(config): Configuration from .json or something.
  ColorScheme();
  const NVGcolor& border() const { return border_; }
  const NVGcolor& background() const { return background_; }
  const NVGcolor& text() const { return text_; }
  const NVGcolor& text_selection() const { return text_selection_; }
  const NVGcolor& cursor() const { return cursor_; }
  const NVGcolor& title_bar_active_inner() const {
    return title_bar_active_inner_;
  }
  const NVGcolor& title_bar_active_outer() const {
    return title_bar_active_outer_;
  }
  const NVGcolor& title_bar_inactive_inner() const {
    return title_bar_inactive_inner_;
  }
  const NVGcolor& title_bar_inactive_outer() const {
    return title_bar_inactive_outer_;
  }
  const NVGcolor& title_bar_text_active() const {
    return title_bar_text_active_;
  }
  const NVGcolor& title_bar_text_inactive() const {
    return title_bar_text_inactive_;
  }
  const NVGcolor& title_bar_text_drop_shadow() const {
    return title_bar_text_drop_shadow_;
  }
  const NVGcolor& drop_shadow_inner() const { return drop_shadow_inner_; }
  const NVGcolor& drop_shadow_outer() const { return drop_shadow_outer_; }
  const NVGcolor& drop_indicator() const { return drop_indicator_; }

  const NVGcolor& comment() const { return comment_; }
  const NVGcolor& comment_preprocessor() const { return comment_preprocessor_; }
  const NVGcolor& error() const { return error_; }
  const NVGcolor& keyword() const { return keyword_; }
  const NVGcolor& keyword_type() const { return keyword_type_; }
  const NVGcolor& literal_number() const { return literal_number_; }
  const NVGcolor& literal_string() const { return literal_string_; }
  const NVGcolor& klass() const { return klass_; }
  const NVGcolor& op() const { return op_; }

  const NVGcolor& margin() const { return margin_; }
  const NVGcolor& margin_text() const { return margin_text_; }

  const NVGcolor& pc_indicator() const { return pc_indicator_; }

 private:
  NVGcolor border_;
  NVGcolor background_;
  NVGcolor text_;
  NVGcolor text_selection_;
  NVGcolor cursor_;
  NVGcolor title_bar_active_inner_;
  NVGcolor title_bar_active_outer_;
  NVGcolor title_bar_inactive_inner_;
  NVGcolor title_bar_inactive_outer_;
  NVGcolor title_bar_text_active_;
  NVGcolor title_bar_inactive_;
  NVGcolor title_bar_text_inactive_;
  NVGcolor title_bar_text_drop_shadow_;
  NVGcolor drop_shadow_inner_;
  NVGcolor drop_shadow_outer_;
  NVGcolor drop_indicator_;

  NVGcolor comment_;
  NVGcolor comment_preprocessor_;
  NVGcolor error_;
  NVGcolor keyword_;
  NVGcolor keyword_type_;
  NVGcolor literal_number_;
  NVGcolor literal_string_;
  NVGcolor klass_;
  NVGcolor op_;

  NVGcolor margin_;
  NVGcolor margin_text_;

  NVGcolor pc_indicator_;
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
  const Texture* vscrollbar_top_texture() const {
    return &vscrollbar_top_texture_;
  }
  const Texture* vscrollbar_middle_texture() const {
    return &vscrollbar_middle_texture_;
  }
  const Texture* vscrollbar_bottom_texture() const {
    return &vscrollbar_bottom_texture_;
  }
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
