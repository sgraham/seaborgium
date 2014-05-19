// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_GFX_H_
#define CORE_GFX_H_

#include "core/core.h"

namespace core {
namespace gfx {

enum TextAlignment {
  TextAlignmentLeft,
  TextAlignmentCenter,
  TextAlignmentRight,
};

typedef int TextureId;

void Init();
void Shutdown();

void Frame();

void DrawText(float x,
              float y,
              TextAlignment align,
              uint32_t abgr,
              const char* text);
void DrawTextf(float x,
               float y,
               TextAlignment align,
               uint32_t abgr,
               const char* format,
               ...);
void DrawRect(float x, float y, float w, float h, uint32_t abgr);
void DrawLine(float x0, float y0, float x1, float y1, uint32_t abgr);

TextureId LoadTexture(uint32_t* texture_data, int width, int height);

void DrawSprite(float x, float y, uint32_t abgr, TextureId texid);
void DrawTexturedRect(float x,
                      float y,
                      float w,
                      float h,
                      uint32_t abgr,
                      TextureId texid);

}  // namespace gfx
}  // namespace core

#endif  // CORE_GFX_H_
