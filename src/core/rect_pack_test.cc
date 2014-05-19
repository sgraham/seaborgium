// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/rect_pack.h"

#include "gtest/gtest.h"

namespace core {

TEST(RectPackTest, Basic) {
  RectPack rp(128, 128);
  int x, y;
  EXPECT_TRUE(rp.Insert(16, 16, &x, &y));
  EXPECT_EQ(0, x);
  EXPECT_EQ(0, y);

  EXPECT_TRUE(rp.Insert(16, 16, &x, &y));
  EXPECT_EQ(16, x);
  EXPECT_EQ(0, y);

  EXPECT_TRUE(rp.Insert(128, 32, &x, &y));
  EXPECT_EQ(0, x);
  EXPECT_EQ(32, y);
}

}  // namespace core
