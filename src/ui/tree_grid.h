// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_TREE_GRID_H_
#define UI_TREE_GRID_H_

#include "ui/dockable.h"

class TreeGrid : public Dockable {
 public:
  TreeGrid();
  virtual ~TreeGrid();

  void Render() override;

 private:

  CORE_DISALLOW_COPY_AND_ASSIGN(TreeGrid);
};

#endif  // UI_TREE_GRID_H_
