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
void GfxResize(uint32_t width, uint32_t height);
void GfxFrame();
void GfxShutdown();


float GfxTextf(float x, float y, const char* format, ...);

void GfxDrawFps();

float GetDpiScale();

extern NVGcontext* VG;

}  // namespace core

#endif  // CORE_GFX_H_
