// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/gfx.h"

#define NANOVG_GL2_IMPLEMENTATION
#include <GL/glew.h>
#include "nanovg_gl.h"

#if CORE_PLATFORM_WINDOWS
#include <GL/glew.h>
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "opengl32.lib")
#endif  // CORE_PLATFORM_WINDOWS

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

#if CORE_PLATFORM_WINDOWS

static HWND s_hwnd;
void WinGfxSetHwnd(HWND hwnd) {
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

NVGcontext* VG;

static GlContext glctx_;

void GfxInit() {
  CORE_CHECK(VG == NULL, "GfxInit called twice?");
  glctx_.Create(0, 0);
  CORE_CHECK(glewInit() == GLEW_OK, "couldn't glewInit");
  VG = nvgCreateGL2(1024, 1024, NVG_ANTIALIAS);
}

void GfxShutdown() {
  CORE_CHECK(VG, "GfxInit not called?");
  nvgDeleteGL2(VG);
  VG = NULL;
  glctx_.Destroy();
}

void GfxFrame() {
  /*
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
            */

  glctx_.Swap();

  GL_CHECK(glClearColor(.3f, .3f, .3f, 1.f));
  GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
    GL_STENCIL_BUFFER_BIT));
}

}  // namespace core
