// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/core.h"
#include "core/entry.h"
#include "core/gfx.h"

int Main(int argc, char** argv) {
  core::gfx::Init();
  CORE_UNUSED(argc);
  CORE_UNUSED(argv);

  uint32_t test_texture_data[4] = {
    0xff0000ff, 0xff00ffff,
    0xff00ff00, 0xffff0000,
  };
  core::gfx::TextureId test_texture =
      core::gfx::LoadTexture(test_texture_data, 2, 2);

  uint32_t width, height;
  while (!core::ProcessEvents(&width, &height)) {
    core::gfx::DrawRect(5, 5, 100, 100, 0x60000000);
    core::gfx::DrawSprite(150, 150, 0xffffffff, test_texture);
    core::gfx::DrawTexturedRect(200, 200, 200, 200, 0xffffffff, test_texture);
    core::gfx::Frame();
  }

  core::gfx::Shutdown();

  return 0;
}
