// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKIN_H_
#define SKIN_H_

#include "core.h"
#include "gfx.h"

class ColorScheme {
 public:
  // TODO(config): Configuration from .json or something.
  ColorScheme();
  const Color& border() const { return border_; }
  const Color& background() const { return background_; }
  const Color& text() const { return text_; }
  const Color& text_selection() const { return text_selection_; }
  const Color& cursor() const { return cursor_; }
  const Color& title_bar_active_inner() const {
    return title_bar_active_inner_;
  }
  const Color& title_bar_active_outer() const {
    return title_bar_active_outer_;
  }
  const Color& title_bar_inactive_inner() const {
    return title_bar_inactive_inner_;
  }
  const Color& title_bar_inactive_outer() const {
    return title_bar_inactive_outer_;
  }
  const Color& title_bar_text_active() const {
    return title_bar_text_active_;
  }
  const Color& title_bar_text_inactive() const {
    return title_bar_text_inactive_;
  }
  const Color& title_bar_text_drop_shadow() const {
    return title_bar_text_drop_shadow_;
  }
  const Color& drop_shadow_inner() const { return drop_shadow_inner_; }
  const Color& drop_shadow_outer() const { return drop_shadow_outer_; }
  const Color& drop_indicator() const { return drop_indicator_; }

  const Color& comment() const { return comment_; }
  const Color& comment_preprocessor() const { return comment_preprocessor_; }
  const Color& error() const { return error_; }
  const Color& keyword() const { return keyword_; }
  const Color& keyword_type() const { return keyword_type_; }
  const Color& literal_number() const { return literal_number_; }
  const Color& literal_string() const { return literal_string_; }
  const Color& klass() const { return klass_; }
  const Color& op() const { return op_; }

  const Color& margin() const { return margin_; }
  const Color& margin_text() const { return margin_text_; }

  const Color& pc_indicator() const { return pc_indicator_; }

 private:
  Color border_;
  Color background_;
  Color text_;
  Color text_selection_;
  Color cursor_;
  Color title_bar_active_inner_;
  Color title_bar_active_outer_;
  Color title_bar_inactive_inner_;
  Color title_bar_inactive_outer_;
  Color title_bar_text_active_;
  Color title_bar_text_inactive_;
  Color title_bar_text_drop_shadow_;
  Color drop_shadow_inner_;
  Color drop_shadow_outer_;
  Color drop_indicator_;

  Color comment_;
  Color comment_preprocessor_;
  Color error_;
  Color keyword_;
  Color keyword_type_;
  Color literal_number_;
  Color literal_string_;
  Color klass_;
  Color op_;

  Color margin_;
  Color margin_text_;

  Color pc_indicator_;
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

  DISALLOW_COPY_AND_ASSIGN(Skin);
};

#endif  // SKIN_H_
