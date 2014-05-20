// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/gfx.h"

#if CORE_PLATFORM_WINDOWS
#include <GL/GL.h>
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "opengl32.lib")
#endif  // CORE_PLATFORM_WINDOWS

#include <vector>

// Super ghetto OpenGL 1.x renderer. We only need basic texturing, blending,
// etc. and don't intend to port to mobile. Hopefully this is easily portable
// to desktop platforms and doesn't have a substantial old-path penalty.
//
// Because of the assumed limited number of textures (just fonts, gutter
// icons, etc.) only one GL texture (atlas) is used. This means texture
// coordinates have to be offset. The upside of this is that there is no
// batching required as all primitives are the same and so can be rendered in
// one draw call. And then.. amusingly I just used silly glBegin() so there's
// no batching to worry about anyway. Oh well, maybe I'll switch to arrays at
// some point and want to do only one kick.

#include "core/rect_pack.h"
#include "core/zevv-peep.h"

namespace {

const char* GlEnumName(GLenum en) {
#define GLENUM(_ty) \
  case _ty:         \
    return #_ty

  switch (en) {
    GLENUM(GL_TEXTURE);

    GLENUM(GL_INVALID_ENUM);
    GLENUM(GL_INVALID_VALUE);
    GLENUM(GL_INVALID_OPERATION);
    GLENUM(GL_OUT_OF_MEMORY);
  }
#undef GLENUM
  return "<GLenum?>";
}

#define _GL_CHECK(check, call)                                          \
  do {                                                                  \
    /*CORE_TRACE(#call);*/                                              \
    call;                                                               \
    GLenum err = glGetError();                                          \
    check(err == 0, #call "; GL error 0x%x: %s", err, GlEnumName(err)); \
    CORE_UNUSED(err);                                                   \
  } while (0)

#define IGNORE_GL_ERROR_CHECK(...) \
  do {                             \
  } while (0)

#if CORE_CONFIG_DEBUG
#define GL_CHECK(call) _GL_CHECK(CORE_CHECK, call)
#else
#define GL_CHECK(_call) _call
#endif  // CORE_CONFIG_DEBUG

inline void glColorABGR(uint32_t abgr) {
  GL_CHECK(glColor4f(((abgr & 0xff) >> 0) / 255.f,
                     ((abgr & 0xff00) >> 8) / 255.f,
                     ((abgr & 0xff0000) >> 16) / 255.f,
                     ((abgr & 0xff000000) >> 24) / 255.f));
}

}  // namespace

namespace core {
namespace gfx {

#if CORE_PLATFORM_WINDOWS

static HWND s_hwnd;
void WinSetHwnd(HWND hwnd) {
  s_hwnd = hwnd;
}

typedef BOOL(WINAPI* PFNWGLSWAPINTERVALEXTPROC)(int interval);
static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;

class GlContext {
 public:
  GlContext() : context_(NULL), hdc_(NULL) {}
  void Create(uint32_t width, uint32_t height);
  void Destroy();
  void Resize(uint32_t width, uint32_t height, bool vsync);
  void Swap();

 private:
  HGLRC context_;
  HDC hdc_;

  CORE_DISALLOW_COPY_AND_ASSIGN(GlContext);
};

void GlContext::Create(uint32_t /*width*/, uint32_t /*height*/) {
  PIXELFORMATDESCRIPTOR pfd = {
      sizeof(PIXELFORMATDESCRIPTOR), 1,
      PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER |
          PFD_SUPPORT_COMPOSITION | PFD_GENERIC_ACCELERATED,  // Flags
      PFD_TYPE_RGBA,  // The kind of framebuffer. RGBA or palette.
      32,             // Color depth of the framebuffer.
      0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      24,  // Number of bits for the depthbuffer
      8,   // Number of bits for the stencilbuffer
      0,   // Number of Aux buffers in the framebuffer.
      PFD_MAIN_PLANE, 0, 0, 0, 0};

  hdc_ = ::GetDC(s_hwnd);
  CORE_CHECK(hdc_, "GetDC failed");

  int pixel_format = ::ChoosePixelFormat(hdc_, &pfd);
  CORE_CHECK(::SetPixelFormat(hdc_, pixel_format, &pfd),
             "SetPixelFormat failed");

  context_ = ::wglCreateContext(hdc_);
  wglMakeCurrent(hdc_, context_);

  wglSwapIntervalEXT =
      (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
  wglSwapIntervalEXT(1);
}

void GlContext::Destroy() {
  wglMakeCurrent(NULL, NULL);
  wglDeleteContext(context_);
  context_ = NULL;
  ReleaseDC(s_hwnd, hdc_);
  hdc_ = NULL;
}

void GlContext::Swap() {
  if (s_hwnd) {
    wglMakeCurrent(hdc_, context_);
    SwapBuffers(hdc_);
  }
}

#else
#error "GL context not implemented"
#endif

const int kAtlasSize = 2<<10;

struct TextureInAtlas {
  // All in pixels, not normalized.
  float tx;
  float ty;
  float width;
  float height;
  TextureInAtlas(float tx, float ty, float width, float height)
      : tx(tx), ty(ty), width(width), height(height) {}
  TextureInAtlas(int tx, int ty, int width, int height)
      : tx(static_cast<float>(tx)),
        ty(static_cast<float>(ty)),
        width(static_cast<float>(width)),
        height(static_cast<float>(height)) {}
};

class Renderer {
 public:
  Renderer() : rect_pack_(kAtlasSize, kAtlasSize) {}
  void Init();
  void Shutdown();
  void Swap();

  TextureId LoadTexture(uint32_t* texture_data, int width, int height);
  const TextureInAtlas& GetTextureInAtlas(TextureId texid) const {
    return textures_[texid];
  }

  const char* vendor;
  const char* renderer;
  const char* version;

 private:
  // Draws the 'peep' font into the top row of the texture atlas in white.
  void DrawDebugFontIntoAtlas();
  void BlitToAtlas(uint32_t* texture_data, int x, int y, int width, int height);
  void ReloadAtlas();

  GlContext glctx_;
  uint32_t* tex_atlas_backing_;
  GLuint tex_atlas_id_;
  RectPack rect_pack_;
  std::vector<TextureInAtlas> textures_;

  CORE_DISALLOW_COPY_AND_ASSIGN(Renderer);
};

void Renderer::Init() {
  glctx_.Create(0, 0);

  vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
  renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
  version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
  GL_CHECK(glEnable(GL_TEXTURE_2D));
  GL_CHECK(glEnable(GL_BLEND));
  GL_CHECK(glDisable(GL_LIGHTING));
  GL_CHECK(glDisable(GL_DEPTH_TEST));
  GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

  GL_CHECK(glMatrixMode(GL_PROJECTION));
  GL_CHECK(glOrtho(0, 1024, 768, 0, -100, 100));  // XXX dimensions
  GL_CHECK(glMatrixMode(GL_MODELVIEW));
  GL_CHECK(glLoadIdentity());

  const size_t kTextureWords = kAtlasSize * kAtlasSize;
  tex_atlas_backing_ = new uint32_t[kTextureWords];
  memset(tex_atlas_backing_, 0x00, kTextureWords * sizeof(uint32_t));
  DrawDebugFontIntoAtlas();
  GL_CHECK(glGenTextures(1, &tex_atlas_id_));
  ReloadAtlas();
}

void Renderer::Shutdown() {
  glctx_.Destroy();
  delete[] tex_atlas_backing_;
}

void Renderer::Swap() {
  glctx_.Swap();
}

TextureId Renderer::LoadTexture(uint32_t* texture_data, int width, int height) {
  int x, y;
  CORE_CHECK(rect_pack_.Insert(width, height, &x, &y),
             "Couldn't insert %dx%d texture into atlas", width, height);
  BlitToAtlas(texture_data, x, y, width, height);
  textures_.push_back(TextureInAtlas(x, y, width, height));
  ReloadAtlas();
  return static_cast<TextureId>(textures_.size() - 1);
}

void Renderer::DrawDebugFontIntoAtlas() {
  // Debug font is bit packed 8x16, target is RGBA, top row.
  // Char 0 is solid white, so we can use it to module for "untextured" draws.
  int out_x, out_y;
  CORE_CHECK(rect_pack_.Insert(kAtlasSize, 16, &out_x, &out_y),
             "Couldn't insert font");
  CORE_CHECK(out_x == 0 && out_y == 0, "Font wasn't assigned top left");
  textures_.push_back(TextureInAtlas(out_x, out_y, kAtlasSize, 16));
  for (int ch = 0; ch < 256; ++ch) {
    const unsigned char* char_data = &ZEVV_PEEP_FONT[ch * 16];
    uint32_t* in_texture = &tex_atlas_backing_[ch * 8];
    for (int row = 0; row < 16; ++row, ++char_data, in_texture += kAtlasSize) {
      for (int bit = 0; bit < 8; ++bit) {
        in_texture[7 - bit] = *char_data & (1 << bit) ? 0xffffffffu : 0u;
      }
    }
  }
}

void Renderer::BlitToAtlas(uint32_t* texture_data,
                           int x,
                           int y,
                           int width,
                           int height) {
  CORE_DCHECK(
      x >= 0 && y >= 0 && x + width < kAtlasSize && y + height < kAtlasSize,
      "bad pos/size blitting into atlas");
  for (int row = 0; row < height; ++row) {
    memcpy(&tex_atlas_backing_[(y + row) * kAtlasSize + x],
           &texture_data[row * width],
           width * sizeof(uint32_t));
  }
}

void Renderer::ReloadAtlas() {
  GL_CHECK(glBindTexture(GL_TEXTURE_2D, tex_atlas_id_));
  GL_CHECK(glTexImage2D(GL_TEXTURE_2D,
                        0,
                        GL_RGBA,
                        kAtlasSize,
                        kAtlasSize,
                        0,
                        GL_RGBA,
                        GL_UNSIGNED_BYTE,
                        tex_atlas_backing_));
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
  GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
}

static Renderer s_ctx;

void Init() {
  s_ctx.Init();
}

void Shutdown() {
  s_ctx.Shutdown();
}

void Frame() {
  int64_t now = core::GetHPCounter();
  static int64_t last = now;
  int64_t frame_time = now - last;
  last = now;
  static int64_t min = frame_time;
  static int64_t max = frame_time;
  min = min > frame_time ? frame_time : min;
  max = max < frame_time ? frame_time : max;

  double freq = static_cast<double>(core::GetHPFrequency());
  double to_ms = 1000.0 / freq;
  float pos = 1;
  DrawTextf(10,
            16 * pos++,
            TextAlignmentLeft,
            0xff00a000,
            "      Frame: %7.3f, % 7.3f \x1f, % 7.3f \x1e [ms] / % 6.2f FPS ",
            static_cast<double>(frame_time) * to_ms,
            static_cast<double>(min) * to_ms,
            static_cast<double>(max) * to_ms,
            freq / frame_time);
  DrawTextf(10, 16 * pos++, TextAlignmentLeft, 0xff008000,
            "  GL_VENDOR: %s", s_ctx.vendor);
  DrawTextf(10, 16 * pos++, TextAlignmentLeft, 0xff008000,
            "GL_RENDERER: %s", s_ctx.renderer);
  DrawTextf(10, 16 * pos++, TextAlignmentLeft, 0xff008000,
            " GL_VERSION: %s", s_ctx.version);

  s_ctx.Swap();

  GL_CHECK(glClearColor(.3f, .3f, .3f, 1.f));
  GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
    GL_STENCIL_BUFFER_BIT));
}

void DrawText(float x,
              float y,
              TextAlignment align,
              uint32_t abgr,
              const char* text) {
  CORE_UNUSED(align);
  glPushMatrix();
  glTranslatef(x, y, 0.f);
  const float char_tex_width = 8.f / static_cast<float>(kAtlasSize);
  const float char_tex_height = 16.f / static_cast<float>(kAtlasSize);
  glColorABGR(abgr);

  glBegin(GL_QUADS);
  float dx = 0.0;
  for (const char* ch = text; *ch; ++ch) {
    float tex_x = static_cast<float>(*ch) * char_tex_width;

    glTexCoord2f(tex_x, 0.f);
    glVertex2f(dx, 0.f);

    glTexCoord2f(tex_x + char_tex_width, 0.f);
    glVertex2f(8.f + dx, 0.f);

    glTexCoord2f(tex_x + char_tex_width, char_tex_height);
    glVertex2f(8.f + dx, 16.f);

    glTexCoord2f(tex_x, char_tex_height);
    glVertex2f(0.f + dx, 16.f);

    dx += 8.0;
  }
  glEnd();
  glPopMatrix();
}

void DrawTextf(float x,
               float y,
               TextAlignment align,
               uint32_t abgr,
               const char* format,
               ...) {
  va_list arg_list;
  va_start(arg_list, format);

  char temp[1024];
  char* out = temp;
  int32_t len = core::vsnprintf(out, sizeof(temp), format, arg_list);
  if ((int32_t)sizeof(temp) < len) {
    out = reinterpret_cast<char*>(alloca(len + 1));
    len = core::vsnprintf(out, len, format, arg_list);
  }
  out[len] = '\0';

  DrawText(x, y, align, abgr, out);
  va_end(arg_list);
}

void DrawRect(float x, float y, float w, float h, uint32_t abgr) {
  glColorABGR(abgr);
  glBegin(GL_QUADS);
  const float inset = 1.f / kAtlasSize;
  glTexCoord2f(inset, inset);
  glVertex2f(x, y);
  glTexCoord2f(inset * 4, inset);
  glVertex2f(x + w, y);
  glTexCoord2f(inset * 4, inset * 4);
  glVertex2f(x + w, y + h);
  glTexCoord2f(inset, inset * 4);
  glVertex2f(x, y + h);
  glEnd();
}

TextureId LoadTexture(uint32_t* texture_data, int width, int height) {
  return s_ctx.LoadTexture(texture_data, width, height);
}

void DrawSprite(float x, float y, uint32_t abgr, TextureId texid) {
  const TextureInAtlas& tia = s_ctx.GetTextureInAtlas(texid);
  DrawTexturedRect(x, y, tia.width, tia.height, abgr, texid);
}

void DrawTexturedRect(float x,
                      float y,
                      float w,
                      float h,
                      uint32_t abgr,
                      TextureId texid) {
  glColorABGR(abgr);
  glBegin(GL_QUADS);
  const TextureInAtlas& tia = s_ctx.GetTextureInAtlas(texid);
  glTexCoord2f(tia.tx / kAtlasSize, tia.ty / kAtlasSize);
  glVertex2f(x, y);
  glTexCoord2f((tia.tx + tia.width) / kAtlasSize, tia.ty / kAtlasSize);
  glVertex2f(x + w, y);
  glTexCoord2f((tia.tx + tia.width) / kAtlasSize,
               (tia.ty + tia.height) / kAtlasSize);
  glVertex2f(x + w, y + h);
  glTexCoord2f(tia.tx / kAtlasSize,
               (tia.ty + tia.height) / kAtlasSize);
  glVertex2f(x, y + h);
  glEnd();
}

}  // namespace gfx
}  // namespace core
