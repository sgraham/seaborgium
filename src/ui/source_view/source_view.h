// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_SOURCE_VIEW_SOURCE_VIEW_H_
#define UI_SOURCE_VIEW_SOURCE_VIEW_H_

#include "core/core.h"
#include "ui/scroll_helper.h"
#include "ui/widget.h"

#include <string>

class SourceView : public Widget, public ScrollHelperDataProvider {
 public:
  SourceView();
  ~SourceView() override;

  void SetFilePath(const std::string& path);

  // Implementation of core::InputHandler:
  bool WantMouseEvents() override { return true; }
  bool NotifyMouseWheel(int x, int y, float delta, uint8_t modifiers) override;
  bool NotifyKey(core::Key::Enum key, bool down, uint8_t modifiers) override;

  // Widget:
  void Render() override;

  // ScrollHelperDataProvider:
  int GetContentSize() override;
  const Rect& GetScreenRect() const override;

 private:
  ScrollHelper scroll_;

  CORE_DISALLOW_COPY_AND_ASSIGN(SourceView);
};

#endif  // UI_SOURCE_VIEW_SOURCE_VIEW_H_
