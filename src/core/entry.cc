// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Derived from bgfx/examples/common/entry:
// Copyright 2011-2014 Branimir Karadzic. All rights reserved.
// License: http://www.opensource.org/licenses/BSD-2-Clause

#include "core/entry.h"

extern int Main(int argc, char** argv);

namespace core {

struct Event {
  enum Enum { Exit, Key, Mouse, Size, };
  Event::Enum type;
};

struct KeyEvent : public Event {
  Key::Enum key;
  uint8_t modifiers;
  bool down;
};

struct MouseEvent : public Event {
  int32_t mx;
  int32_t my;
  float delta;
  MouseButton::Enum button;
  uint8_t modifiers;
  bool down;
  bool move;
  bool wheel;
};

struct SizeEvent : public Event {
  uint32_t width;
  uint32_t height;
};

class EventQueue {
 public:
  EventQueue() {}

  void PostExitEvent() {
    Event* ev = new Event;
    ev->type = Event::Exit;
    queue_.push(ev);
  }

  void PostKeyEvent(Key::Enum key, uint8_t modifiers, bool down) {
    KeyEvent* ev = new KeyEvent;
    ev->type = Event::Key;
    ev->key = key;
    ev->modifiers = modifiers;
    ev->down = down;
    queue_.push(ev);
  }

  void PostMouseMoveEvent(int32_t mx, int32_t my) {
    MouseEvent* ev = new MouseEvent;
    ev->type = Event::Mouse;
    ev->mx = mx;
    ev->my = my;
    ev->delta = 0;
    ev->button = MouseButton::None;
    ev->modifiers = 0;
    ev->down = false;
    ev->move = true;
    ev->wheel = false;
    queue_.push(ev);
  }

  void PostMouseWheelEvent(int32_t mx,
                           int32_t my,
                           float delta,
                           uint8_t modifiers) {
    MouseEvent* ev = new MouseEvent;
    ev->type = Event::Mouse;
    ev->mx = mx;
    ev->my = my;
    ev->delta = delta;
    ev->modifiers = modifiers;
    ev->button = MouseButton::None;
    ev->down = false;
    ev->move = false;
    ev->wheel = true;
    queue_.push(ev);
  }

  void PostMouseButtonEvent(int32_t mx,
                            int32_t my,
                            MouseButton::Enum button,
                            bool down,
                            uint8_t modifiers) {
    MouseEvent* ev = new MouseEvent;
    ev->type = Event::Mouse;
    ev->mx = mx;
    ev->my = my;
    ev->delta = 0;
    ev->button = button;
    ev->modifiers = modifiers;
    ev->down = down;
    ev->move = false;
    ev->wheel = false;
    queue_.push(ev);
  }

  void PostSizeEvent(uint32_t width, uint32_t height) {
    SizeEvent* ev = new SizeEvent;
    ev->type = Event::Size;
    ev->width = width;
    ev->height = height;
    queue_.push(ev);
  }

  const Event* Poll() { return queue_.Pop(); }

  void Release(const Event* event) const { delete event; }

 private:
  SpScQueue<Event> queue_;

  CORE_DISALLOW_COPY_AND_ASSIGN(EventQueue);
};

struct MainThreadEntry {
  int argc_;
  char** argv_;

  static int32_t ThreadFunc(void* user_data);
};

#if CORE_PLATFORM_WINDOWS

}  // namespace core
#include <windowsx.h>
namespace core {

#pragma comment(lib, "user32.lib")

#define WM_USER_SET_WINDOW_SIZE (WM_USER + 0)
#define WM_USER_SET_MOUSE_CURSOR (WM_USER + 1)

#define CORE_DEFAULT_WIDTH 1024
#define CORE_DEFAULT_HEIGHT 768

struct TranslateKeyModifiers_t {
  int vk;
  Modifier::Enum modifier;
};

static const TranslateKeyModifiers_t s_translateKeyModifiers[8] = {
    {VK_LMENU, Modifier::LeftAlt},
    {VK_RMENU, Modifier::RightAlt},
    {VK_LCONTROL, Modifier::LeftCtrl},
    {VK_RCONTROL, Modifier::RightCtrl},
    {VK_LSHIFT, Modifier::LeftShift},
    {VK_RSHIFT, Modifier::RightShift},
    {VK_LWIN, Modifier::LeftMeta},
    {VK_RWIN, Modifier::RightMeta}, };

static uint8_t TranslateKeyModifiers() {
  uint8_t modifiers = 0;
  for (uint32_t i = 0; i < CORE_COUNTOF(s_translateKeyModifiers); ++i) {
    const TranslateKeyModifiers_t& tkm = s_translateKeyModifiers[i];
    modifiers |= 0 > GetKeyState(tkm.vk) ? tkm.modifier : Modifier::None;
  }
  return modifiers;
}

static uint8_t s_translateKey[256];
static Key::Enum TranslateKey(WPARAM wparam) {
  return static_cast<Key::Enum>(s_translateKey[wparam & 0xff]);
}

static HCURSOR s_current_cursor;

extern void WinGfxSetHwnd(HWND hwnd);
extern void WinGfxSetDpiScale(float dpi_scale);

struct Context {
  Context() : init_(false), exit_(false) {
    memset(s_translateKey, 0, sizeof(s_translateKey));
    s_translateKey[VK_ESCAPE] = Key::Esc;
    s_translateKey[VK_RETURN] = Key::Return;
    s_translateKey[VK_TAB] = Key::Tab;
    s_translateKey[VK_BACK] = Key::Backspace;
    s_translateKey[VK_SPACE] = Key::Space;
    s_translateKey[VK_UP] = Key::Up;
    s_translateKey[VK_DOWN] = Key::Down;
    s_translateKey[VK_LEFT] = Key::Left;
    s_translateKey[VK_RIGHT] = Key::Right;
    s_translateKey[VK_PRIOR] = Key::PageUp;
    s_translateKey[VK_NEXT] = Key::PageUp;
    s_translateKey[VK_HOME] = Key::Home;
    s_translateKey[VK_END] = Key::End;
    s_translateKey[VK_SNAPSHOT] = Key::Print;
    s_translateKey[VK_OEM_PLUS] = Key::Plus;
    s_translateKey[VK_OEM_MINUS] = Key::Minus;
    s_translateKey[VK_F1] = Key::F1;
    s_translateKey[VK_F2] = Key::F2;
    s_translateKey[VK_F3] = Key::F3;
    s_translateKey[VK_F4] = Key::F4;
    s_translateKey[VK_F5] = Key::F5;
    s_translateKey[VK_F6] = Key::F6;
    s_translateKey[VK_F7] = Key::F7;
    s_translateKey[VK_F8] = Key::F8;
    s_translateKey[VK_F9] = Key::F9;
    s_translateKey[VK_F10] = Key::F10;
    s_translateKey[VK_F11] = Key::F11;
    s_translateKey[VK_F12] = Key::F12;
    s_translateKey[VK_NUMPAD0] = Key::NumPad0;
    s_translateKey[VK_NUMPAD1] = Key::NumPad1;
    s_translateKey[VK_NUMPAD2] = Key::NumPad2;
    s_translateKey[VK_NUMPAD3] = Key::NumPad3;
    s_translateKey[VK_NUMPAD4] = Key::NumPad4;
    s_translateKey[VK_NUMPAD5] = Key::NumPad5;
    s_translateKey[VK_NUMPAD6] = Key::NumPad6;
    s_translateKey[VK_NUMPAD7] = Key::NumPad7;
    s_translateKey[VK_NUMPAD8] = Key::NumPad8;
    s_translateKey[VK_NUMPAD9] = Key::NumPad9;
    s_translateKey['0'] = Key::Key0;
    s_translateKey['1'] = Key::Key1;
    s_translateKey['2'] = Key::Key2;
    s_translateKey['3'] = Key::Key3;
    s_translateKey['4'] = Key::Key4;
    s_translateKey['5'] = Key::Key5;
    s_translateKey['6'] = Key::Key6;
    s_translateKey['7'] = Key::Key7;
    s_translateKey['8'] = Key::Key8;
    s_translateKey['9'] = Key::Key9;
    s_translateKey['A'] = Key::KeyA;
    s_translateKey['B'] = Key::KeyB;
    s_translateKey['C'] = Key::KeyC;
    s_translateKey['D'] = Key::KeyD;
    s_translateKey['E'] = Key::KeyE;
    s_translateKey['F'] = Key::KeyF;
    s_translateKey['G'] = Key::KeyG;
    s_translateKey['H'] = Key::KeyH;
    s_translateKey['I'] = Key::KeyI;
    s_translateKey['J'] = Key::KeyJ;
    s_translateKey['K'] = Key::KeyK;
    s_translateKey['L'] = Key::KeyL;
    s_translateKey['M'] = Key::KeyM;
    s_translateKey['N'] = Key::KeyN;
    s_translateKey['O'] = Key::KeyO;
    s_translateKey['P'] = Key::KeyP;
    s_translateKey['Q'] = Key::KeyQ;
    s_translateKey['R'] = Key::KeyR;
    s_translateKey['S'] = Key::KeyS;
    s_translateKey['T'] = Key::KeyT;
    s_translateKey['U'] = Key::KeyU;
    s_translateKey['V'] = Key::KeyV;
    s_translateKey['W'] = Key::KeyW;
    s_translateKey['X'] = Key::KeyX;
    s_translateKey['Y'] = Key::KeyY;
    s_translateKey['Z'] = Key::KeyZ;
  }

  int32_t Run(int argc, char** argv) {
    ::SetDllDirectory(".");

    HINSTANCE instance = reinterpret_cast<HINSTANCE>(::GetModuleHandle(NULL));

    WNDCLASSEX wnd = {0};
    wnd.cbSize = sizeof(wnd);
    wnd.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wnd.lpfnWndProc = WndProc;
    wnd.hInstance = instance;
    wnd.hIcon = ::LoadIcon(NULL, IDI_APPLICATION);
    wnd.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    wnd.lpszClassName = "seaborgium";
    wnd.hIconSm = ::LoadIcon(NULL, IDI_APPLICATION);
    ::RegisterClassExA(&wnd);

    HDC screen_dc = ::GetDC(NULL);
    float dpi_scale_x = GetDeviceCaps(screen_dc, LOGPIXELSX) / 96.f;
    float dpi_scale_y = GetDeviceCaps(screen_dc, LOGPIXELSY) / 96.f;
    CORE_CHECK(dpi_scale_x == dpi_scale_y,
               "Don't handle non-uniform DPI scale");
    WinGfxSetDpiScale(dpi_scale_x);
    ::ReleaseDC(NULL, screen_dc);

    hwnd_ =
        ::CreateWindowA("seaborgium",
                        "Seaborgium",
                        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                        50,
                        50,
                        static_cast<int>(CORE_DEFAULT_WIDTH * GetDpiScale()),
                        static_cast<int>(CORE_DEFAULT_HEIGHT * GetDpiScale()),
                        NULL,
                        NULL,
                        instance,
                        0);

    WinGfxSetHwnd(hwnd_);

    Adjust(static_cast<int>(CORE_DEFAULT_WIDTH * GetDpiScale()),
           static_cast<int>(CORE_DEFAULT_HEIGHT * GetDpiScale()));

    MainThreadEntry mte;
    mte.argc_ = argc;
    mte.argv_ = argv;

    Thread thread;
    thread.Init(mte.ThreadFunc, &mte);
    init_ = true;

    event_queue_.PostSizeEvent(
        static_cast<int>(CORE_DEFAULT_WIDTH * GetDpiScale()),
        static_cast<int>(CORE_DEFAULT_HEIGHT * GetDpiScale()));

    ::ShowWindow(hwnd_, SW_MAXIMIZE);

    MSG msg;
    msg.message = WM_NULL;

    while (!exit_) {
      ::WaitMessage();

      while (0 != ::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
      }
    }

    thread.Shutdown();

    ::DestroyWindow(hwnd_);

    return 0;
  }

  void MouseFromLparam(LPARAM lparam, int32_t* mx, int32_t* my) {
    *mx = static_cast<int32_t>(GET_X_LPARAM(lparam) / GetDpiScale());
    *my = static_cast<int32_t>(GET_Y_LPARAM(lparam) / GetDpiScale());
  }

  LRESULT Process(HWND hwnd, UINT id, WPARAM wparam, LPARAM lparam) {
    if (init_) {
      switch (id) {
        case WM_USER_SET_WINDOW_SIZE: {
          uint32_t width = GET_X_LPARAM(lparam);
          uint32_t height = GET_Y_LPARAM(lparam);
          Adjust(width, height);
        } break;

        case WM_USER_SET_MOUSE_CURSOR: {
          MouseCursor::Enum cursor = static_cast<MouseCursor::Enum>(lparam);
          static LPSTR IdcCursorForMouseCursor[] = {
              IDC_ARROW,    // Default
              IDC_SIZEWE,   // DragLeftRight
              IDC_SIZENS,   // DragUpDown
              IDC_SIZEALL,  // DragAll,
              IDC_HAND,     // Pointer,
          };
          s_current_cursor = ::LoadCursor(NULL, IdcCursorForMouseCursor[cursor]);
          ::SetCursor(s_current_cursor);
        } break;

        case WM_SETCURSOR:
          if (LOWORD(lparam) == HTCLIENT) {
            ::SetCursor(s_current_cursor);
            return TRUE;
          }

        case WM_DESTROY:
          break;

        case WM_QUIT:
        case WM_CLOSE:
          exit_ = true;
          event_queue_.PostExitEvent();
          break;

        case WM_SIZE: {
          uint32_t width = GET_X_LPARAM(lparam);
          uint32_t height = GET_Y_LPARAM(lparam);
          event_queue_.PostSizeEvent(width, height);
        } break;

        case WM_MOUSEMOVE: {
          int32_t mx, my;
          MouseFromLparam(lparam, &mx, &my);
          event_queue_.PostMouseMoveEvent(mx, my);
        } break;

        case WM_MOUSEWHEEL: {
          int32_t mx, my;
          MouseFromLparam(lparam, &mx, &my);
          uint8_t modifiers = TranslateKeyModifiers();
          event_queue_.PostMouseWheelEvent(
              mx,
              my,
              static_cast<float>(wparam) / static_cast<float>(WHEEL_DELTA),
              modifiers);
        } break;

        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_LBUTTONDBLCLK: {
          int32_t mx, my;
          MouseFromLparam(lparam, &mx, &my);
          uint8_t modifiers = TranslateKeyModifiers();
          event_queue_.PostMouseButtonEvent(
              mx, my, MouseButton::Left, id == WM_LBUTTONDOWN, modifiers);
        } break;

        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MBUTTONDBLCLK: {
          int32_t mx, my;
          MouseFromLparam(lparam, &mx, &my);
          uint8_t modifiers = TranslateKeyModifiers();
          event_queue_.PostMouseButtonEvent(
              mx, my, MouseButton::Middle, id == WM_MBUTTONDOWN, modifiers);
        } break;

        case WM_RBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONDBLCLK: {
          int32_t mx, my;
          MouseFromLparam(lparam, &mx, &my);
          uint8_t modifiers = TranslateKeyModifiers();
          event_queue_.PostMouseButtonEvent(
              mx, my, MouseButton::Right, id == WM_RBUTTONDOWN, modifiers);
        } break;



        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP: {
          uint8_t modifiers = TranslateKeyModifiers();
          Key::Enum key = TranslateKey(wparam);

          if (Key::Print == key && 0x3 == ((uint32_t)(lparam) >> 30)) {
            // VK_SNAPSHOT doesn't generate keydown event. Fire on down event
            // when previous key state bit is set to 1 and transition state bit
            // is set to 1.
            //
            // http://msdn.microsoft.com/en-us/library/windows/desktop/ms646280%28v=vs.85%29.aspx
            event_queue_.PostKeyEvent(key, modifiers, true);
          }

          event_queue_.PostKeyEvent(
              key, modifiers, id == WM_KEYDOWN || id == WM_SYSKEYDOWN);
        } break;

        // These don't appear sufficient to avoid flicker during drag resizing
        // with GL rendering. I don't know why.
        case WM_ERASEBKGND:
          return TRUE;

        case WM_PAINT: {
          ::ValidateRect(hwnd, NULL);
          return TRUE;
        }

        default:
          break;
      }
    }

    return ::DefWindowProc(hwnd, id, wparam, lparam);
  }

  void Adjust(uint32_t width, uint32_t height) {
    ::ShowWindow(hwnd_, SW_SHOWNORMAL);
    RECT newrect = {0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};

    RECT rect;
    ::GetWindowRect(hwnd_, &rect);
    DWORD style = ::GetWindowLong(hwnd_, GWL_STYLE);

    ::SetWindowLong(hwnd_, GWL_STYLE, style);
    ::AdjustWindowRect(&newrect, style, FALSE);
    ::UpdateWindow(hwnd_);

    int32_t left = rect.left;
    int32_t top = rect.top;
    int32_t newwidth = (newrect.right - newrect.left);
    int32_t newheight = (newrect.bottom - newrect.top);

    ::SetWindowPos(
        hwnd_, HWND_TOP, left, top, newwidth, newheight, SWP_SHOWWINDOW);
    ::ShowWindow(hwnd_, SW_RESTORE);
  }

  static LRESULT CALLBACK
      WndProc(HWND hwnd, UINT id, WPARAM wparam, LPARAM lparam);

  EventQueue event_queue_;

  HWND hwnd_;
  bool init_;
  bool exit_;
};

static Context s_ctx;

LRESULT CALLBACK
Context::WndProc(HWND hwnd, UINT id, WPARAM wparam, LPARAM lparam) {
  return s_ctx.Process(hwnd, id, wparam, lparam);
}

const Event* Poll() { return s_ctx.event_queue_.Poll(); }

void Release(const Event* event) { s_ctx.event_queue_.Release(event); }

void SetWindowSize(uint32_t width, uint32_t height) {
  ::PostMessage(s_ctx.hwnd_,
                WM_USER_SET_WINDOW_SIZE,
                0,
                (height << 16) | (width & 0xffff));
}

void SetMouseCursor(MouseCursor::Enum cursor) {
  ::PostMessage(s_ctx.hwnd_, WM_USER_SET_MOUSE_CURSOR, 0, cursor);
}

int32_t MainThreadEntry::ThreadFunc(void* user_data) {
  MainThreadEntry* self = reinterpret_cast<MainThreadEntry*>(user_data);
  int32_t result = Main(self->argc_, self->argv_);
  ::PostMessage(s_ctx.hwnd_, WM_QUIT, 0, 0);
  return result;
}


#endif  // CORE_PLATFORM_WINDOWS


#if CORE_PLATFORM_LINUX


#endif  // CORE_PLATFORM_LINUX

bool ProcessEvents(uint32_t* width, uint32_t* height, InputHandler* handler) {
  const Event* ev;
  do {
    struct SE {
      const Event* ev_;
      SE() : ev_(Poll()) {}
      ~SE() {
        if (NULL != ev_) {
          Release(ev_);
        }
      }
    } scoped_event;
    ev = scoped_event.ev_;

    if (ev) {
      switch (ev->type) {
        case Event::Exit:
          return true;

        case Event::Mouse:
          if (handler->WantMouseEvents()) {
            const MouseEvent& mouse_event = *static_cast<const MouseEvent*>(ev);
            if (mouse_event.move) {
              handler->NotifyMouseMoved(
                  mouse_event.mx, mouse_event.my, mouse_event.modifiers);
            } else if (mouse_event.wheel) {
              handler->NotifyMouseWheel(mouse_event.mx,
                                        mouse_event.my,
                                        mouse_event.delta,
                                        mouse_event.modifiers);
            } else {
              // Button press.
              handler->NotifyMouseButton(
                  mouse_event.button, mouse_event.down, mouse_event.modifiers);
            }
          }
          break;

        case Event::Key:
          CORE_UNUSED(handler);
          break;

        case Event::Size: {
          const SizeEvent& size_event = *static_cast<const SizeEvent*>(ev);
          *width = size_event.width;
          *height = size_event.height;
          break;
        }

        default:
          break;
      }
    }
  } while (ev);

  return false;
}

}  // namespace core

int main(int argc, char** argv) {
  return core::s_ctx.Run(argc, argv);
}
