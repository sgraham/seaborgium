// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_GFX_H_
#define CORE_GFX_H_

#include "core/core.h"
struct NVGcontext;
struct NVGcolor;

namespace core {

void GfxInit();
void GfxFrame();
void GfxShutdown();

extern NVGcontext* VG;

}  // namespace core

#endif  // CORE_GFX_H_
