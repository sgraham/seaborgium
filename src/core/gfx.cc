// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/gfx.h"

#include "core/entry.h"

#define NANOVG_GL2_IMPLEMENTATION
#include <GL/glew.h>  // NOLINT(build/include)
#include "nanovg_gl.h"

#if CORE_PLATFORM_WINDOWS
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

static float s_dpi_scale = 1.f;

}  // namespace

namespace core {

#if CORE_PLATFORM_WINDOWS

static HWND s_hwnd;
void WinGfxSetHwnd(HWND hwnd) {
  s_hwnd = hwnd;
}

void WinGfxSetDpiScale(float dpi_scale) {
  s_dpi_scale = dpi_scale;
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
  CORE_CHECK(wglMakeCurrent(hdc_, context_), "wglMakeCurrent");

  wglSwapIntervalEXT =
      (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
  wglSwapIntervalEXT(1);
}

void GlContext::Destroy() {
  CORE_CHECK(wglMakeCurrent(NULL, NULL), "wglMakeCurrent");
  CORE_CHECK(wglDeleteContext(context_), "wglDeleteContext");
  context_ = NULL;
  ::ReleaseDC(s_hwnd, hdc_);
  hdc_ = NULL;
}

void GlContext::Swap() {
  if (s_hwnd) {
    CORE_CHECK(wglMakeCurrent(hdc_, context_), "wglMakeCurrent");
    ::SwapBuffers(hdc_);
  }
}

#else
#error "GL context not implemented"
#endif

NVGcontext* VG;

static GlContext s_glctx;
static uint32_t s_width;
static uint32_t s_height;

void GfxInit() {
  CORE_CHECK(VG == NULL, "GfxInit called twice?");
  s_glctx.Create(0, 0);
  CORE_CHECK(glewInit() == GLEW_OK, "couldn't glewInit");
  GL_CHECK(glDisable(GL_DEPTH_TEST));
  GL_CHECK(glDisable(GL_STENCIL_TEST));
  VG = nvgCreateGL2(2048, 2048, NVG_ANTIALIAS);
}

void GfxResize(uint32_t width, uint32_t height) {
  s_width = width;
  s_height = height;
}

void GfxShutdown() {
  CORE_CHECK(VG, "GfxInit not called?");
  nvgDeleteGL2(VG);
  VG = NULL;
  s_glctx.Destroy();
}

float GfxTextf(float x, float y, const char* format, ...) {
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
  va_end(arg_list);

  return nvgText(core::VG, x, y, out, out + len);
}

void GfxDrawFps() {
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

  nvgFontSize(VG, 13.f);
  nvgFontFace(VG, "mono");
  nvgFillColor(VG, nvgRGBA(0x00, 0xa0, 0x00, 0x60));
  GfxTextf(10,
           16 * pos++,
           // utf-8 sequences are UPWARDS ARROW and DOWNWARDS ARROW.
           "      Frame: %7.3f, % 7.3f \xe2\x86\x91, % 7.3f \xe2\x86\x93 [ms] "
           "/ % 6.2f FPS ",
           static_cast<double>(frame_time) * to_ms,
           static_cast<double>(min) * to_ms,
           static_cast<double>(max) * to_ms,
           freq / frame_time);
  GfxTextf(10, 16 * pos++, "  GL_VENDOR: %s", glGetString(GL_VENDOR));
  GfxTextf(10, 16 * pos++, "GL_RENDERER: %s", glGetString(GL_RENDERER));
  GfxTextf(10, 16 * pos++, " GL_VERSION: %s", glGetString(GL_VERSION));
}

float GetDpiScale() {
  return s_dpi_scale;
}

void GfxFrame() {
  s_glctx.Swap();

  GL_CHECK(glViewport(0, 0, s_width, s_height));

  GL_CHECK(glClearColor(.3f, .3f, .32f, 1.f));
  GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
    GL_STENCIL_BUFFER_BIT));
}

}  // namespace core
