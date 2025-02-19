// Author: Jake Rieger
// Created: 2/18/2025.
//

#include "Window.hpp"

#include "RasterizerState.hpp"

#include <windowsx.h>

namespace x {
    Window::Window(const str& title, const int width, const int height) : _context() {
        _instance      = None;
        _hwnd          = None;
        _currentWidth  = width;
        _currentHeight = height;
        _title         = title;
    }

    Window::~Window() {
        Shutdown();
    }

    int Window::Run() {
        _focused = true;

        if (!Initialize()) {
            X_LOG_ERROR("Failed to initialize window");
            return -1;
        }

        MSG msg;
        ZeroMemory(&msg, sizeof(MSG));
        while (msg.message != WM_QUIT) {
            if (::PeekMessage(&msg, None, 0, 0, PM_REMOVE)) {
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
                continue;
            }

            _windowViewport->AttachViewport();
            MainLoop();
        }

        Shutdown();

        return 0;
    }

    LRESULT Window::Quit() {
        ::PostQuitMessage(0);
        return S_OK;
    }

    bool Window::Initialize() {
        // Initialize window
        const auto hr = ::CoInitializeEx(None, COINIT_MULTITHREADED);
        if (FAILED(hr)) {
            X_LOG_ERROR("CoInitializeEx failed, hr = 0x%x", hr);
            return false;
        }

        WNDCLASSEXA wc {};
        wc.cbSize        = sizeof(WNDCLASSEXA);
        wc.style         = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc   = WndProc;
        wc.hInstance     = _instance;
        wc.lpszClassName = "XEditorWindowClass";

        if (!::RegisterClassExA(&wc)) {
            X_LOG_ERROR("Failed to register window class");
            return false;
        }

        _hwnd = ::CreateWindowExA(WS_EX_APPWINDOW,
                                  wc.lpszClassName,
                                  _title.c_str(),
                                  WS_OVERLAPPEDWINDOW,
                                  CW_USEDEFAULT,
                                  CW_USEDEFAULT,
                                  CAST<i32>(_currentWidth),
                                  CAST<i32>(_currentHeight),
                                  None,
                                  None,
                                  _instance,
                                  this);

        if (!_hwnd) {
            X_LOG_ERROR("Failed to create window");
            return false;
        }

        ::ShowWindow(_hwnd, SW_SHOWDEFAULT);
        ::UpdateWindow(_hwnd);

        // Initialize DirectX context and window viewport (final render output)
        _context.Initialize(_hwnd, _currentWidth, _currentHeight);
        RasterizerStates::Initialize(_context);
        _windowViewport = make_unique<Viewport>(_context);

        if (!_windowViewport->Resize(_currentWidth, _currentHeight, true)) {
            X_LOG_ERROR("Failed to resize window viewport");
            return false;
        }

        // Call subclass initialize method
        OnInitialize();

        return true;
    }

    void Window::Shutdown() {
        OnShutdown();

        if (_windowViewport) {
            _windowViewport->BindRenderTarget();
            ID3D11RenderTargetView* nullRTV = None;
            _context.GetDeviceContext()->OMSetRenderTargets(1, &nullRTV, None);
            _windowViewport.reset();
        }

        ::CoUninitialize();
    }

    LRESULT Window::ResizeHandler(u32 width, u32 height) {
        if (!_focused) return S_OK;

        _currentWidth  = width;
        _currentHeight = height;

        // Resize DirectX resources
        if (_windowViewport.get() != None) {
            if (!_windowViewport->Resize(_currentWidth, _currentHeight, true)) {
                X_LOG_ERROR("Failed to resize window viewport");
                return E_FAIL;
            }
        }

        OnResize(width, height);
        Emit(WindowResizeEvent(width, height));

        return S_OK;
    }

    LRESULT Window::MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
            case WM_DESTROY:
                return Quit();
            case WM_SIZE:
                return ResizeHandler(LOWORD(lParam), HIWORD(lParam));

            case WM_KEYDOWN: {
                Emit(KeyPressedEvent((u32)(i32)wParam));
            }
                return 0;
            case WM_KEYUP: {
                Emit(KeyReleasedEvent((u32)(i32)wParam));
            }
                return 0;
            case WM_LBUTTONDOWN: {
                Emit(MouseButtonPressedEvent(0));
            }
                return 0;
            case WM_LBUTTONUP: {
                Emit(MouseButtonReleasedEvent(0));
            }
                return 0;
            case WM_RBUTTONDOWN: {
                Emit(MouseButtonPressedEvent(1));
            }
                return 0;
            case WM_RBUTTONUP: {
                Emit(MouseButtonReleasedEvent(1));
            }
                return 0;
            case WM_MOUSEMOVE: {
                const auto x = GET_X_LPARAM(lParam);
                const auto y = GET_Y_LPARAM(lParam);
                Emit(MouseMoveEvent(x, y));
            }
                return 0;
            case WM_KILLFOCUS: {
                _focused = false;
                Emit(WindowLostFocusEvent());
            }
                return 0;
            case WM_SETFOCUS: {
                _focused = true;
                Emit(WindowFocusEvent());
            }
                return 0;
            default:
                break;
        }

        return ::DefWindowProcA(_hwnd, msg, wParam, lParam);
    }

    LRESULT Window::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        Window* self = None;

        if (msg == WM_CREATE) {
            const auto* pCreate = RCAST<CREATESTRUCTA*>(lParam);
            self                = CAST<Window*>(pCreate->lpCreateParams);
            ::SetWindowLongPtrA(hwnd, GWLP_USERDATA, RCAST<LONG_PTR>(self));
        } else {
            self = RCAST<Window*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
        }

        if (self) { return self->MessageHandler(msg, wParam, lParam); }

        return ::DefWindowProcA(hwnd, msg, wParam, lParam);
    }
}  // namespace x