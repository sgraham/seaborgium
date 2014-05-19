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

  uint32_t width, height;
  while (!core::ProcessEvents(&width, &height)) {
    core::gfx::Frame();
  }

  core::gfx::Shutdown();

  return 0;
}
