// Author: Jake Rieger
// Created: 2/18/2025.
//

#include "Window.hpp"

#include "RasterizerState.hpp"

#include <windowsx.h>

namespace x {
    IWindow::IWindow(const str& title, const int width, const int height, WORD windowIcon) : mContext() {
        mInstance      = nullptr;
        mHwnd          = nullptr;
        mCurrentWidth  = width;
        mCurrentHeight = height;
        mTitle         = title;
        mWindowIcon    = windowIcon;
    }

    IWindow::~IWindow() {
        Shutdown();
    }

    int IWindow::Run() {
        mFocused = true;

        if (!Initialize()) {
            X_LOG_ERROR("Failed to initialize window");
            return -1;
        }

        MSG msg;
        ::ZeroMemory(&msg, sizeof(MSG));
        while (msg.message != WM_QUIT) {
            if (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
                if (msg.message == WM_QUIT) {
                    mQuitRequested = true;
                    break;
                }

                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
                continue;
            }

            if (mQuitRequested) break;

            OnUpdate();
            OnRender();
            mContext.Present();
        }

        Shutdown();

        return 0;
    }

    LRESULT IWindow::Quit() {
        mQuitRequested = true;
        ::DestroyWindow(mHwnd);
        return S_OK;
    }

    bool IWindow::Initialize() {
        // Initialize window
        const auto hr =
          ::CoInitializeEx(nullptr, COINIT_MULTITHREADED | COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
        if (FAILED(hr)) {
            X_LOG_ERROR("CoInitializeEx failed, hr = 0x%x", hr);
            return false;
        }

        WNDCLASSEXA wc {};
        wc.cbSize        = sizeof(WNDCLASSEXA);
        wc.style         = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc   = WndProc;
        wc.hInstance     = mInstance;
        wc.lpszClassName = "XEditorWindowClass";

        if (mWindowIcon != 0) {
            const HICON icon = ::LoadIcon(mInstance, MAKEINTRESOURCE(mWindowIcon));
            wc.hIcon         = icon;
            wc.hIconSm       = icon;
        }

        if (!::RegisterClassExA(&wc)) {
            X_LOG_ERROR("Failed to register window class");
            return false;
        }

        // Calculate screen center for window
        const int scrX = ::GetSystemMetrics(SM_CXSCREEN);
        const int scrY = ::GetSystemMetrics(SM_CYSCREEN);
        const int winX = (scrX - (i32)mCurrentWidth) / 2;
        const int winY = (scrY - (i32)mCurrentHeight) / 2;

        mHwnd = ::CreateWindowExA(WS_EX_APPWINDOW,
                                  wc.lpszClassName,
                                  mTitle.c_str(),
                                  WS_OVERLAPPEDWINDOW,
                                  winX,
                                  winY,
                                  CAST<i32>(mCurrentWidth),
                                  CAST<i32>(mCurrentHeight),
                                  nullptr,
                                  nullptr,
                                  mInstance,
                                  this);

        if (!mHwnd) {
            X_LOG_ERROR("Failed to create window");
            return false;
        }

        ::ShowWindow(mHwnd, mOpenMaximized ? SW_SHOWMAXIMIZED : SW_SHOWDEFAULT);
        ::UpdateWindow(mHwnd);

        // Initialize DirectX context and window viewport (final render output)
        mContext.Initialize(mHwnd, mCurrentWidth, mCurrentHeight);
        RasterizerStates::Initialize(mContext);
        mWindowViewport = make_unique<Viewport>(mContext);

        if (!mWindowViewport->Resize(mCurrentWidth, mCurrentHeight, true)) {
            X_LOG_ERROR("Failed to resize window viewport");
            return false;
        }

        // Call subclass initialize method
        OnInitialize();

        return true;
    }

    void IWindow::Shutdown() {
        OnShutdown();

        if (mWindowViewport) {
            mWindowViewport->BindRenderTarget();
            ID3D11RenderTargetView* nullRTV = nullptr;
            mContext.GetDeviceContext()->OMSetRenderTargets(1, &nullRTV, nullptr);
            mWindowViewport.reset();
        }

        ::CoUninitialize();
    }

    LRESULT IWindow::ResizeHandler(u32 width, u32 height) {
        if (!mFocused) return S_OK;

        mCurrentWidth  = width;
        mCurrentHeight = height;

        // Resize DirectX resources
        if (mWindowViewport.get() != nullptr) {
            if (!mWindowViewport->Resize(mCurrentWidth, mCurrentHeight, true)) {
                X_LOG_ERROR("Failed to resize window viewport");
                return E_FAIL;
            }
        }

        OnResize(width, height);
        Emit(WindowResizeEvent(width, height));

        return S_OK;
    }

    LRESULT IWindow::MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
            case WM_DESTROY:
                mQuitRequested = true;
                return 0;
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
                mFocused = false;
                Emit(WindowLostFocusEvent());
            }
                return 0;
            case WM_SETFOCUS: {
                mFocused = true;
                Emit(WindowFocusEvent());
            }
                return 0;
            default:
                break;
        }

        return ::DefWindowProcA(mHwnd, msg, wParam, lParam);
    }

    void IWindow::SetWindowTitle(const str& title) const {
        ::SetWindowTextA(mHwnd, title.c_str());
    }

    void IWindow::SetWindowIcon(WORD resourceId) const {
        HICON icon = ::LoadIconA(::GetModuleHandleA(NULL), MAKEINTRESOURCE(resourceId));
        ::SendMessageA(mHwnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);
        ::SendMessageA(mHwnd, WM_SETICON, ICON_BIG, (LPARAM)icon);
    }

    LRESULT IWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        IWindow* self = nullptr;

        if (msg == WM_CREATE) {
            const auto* pCreate = RCAST<CREATESTRUCTA*>(lParam);
            self                = CAST<IWindow*>(pCreate->lpCreateParams);
            ::SetWindowLongPtrA(hwnd, GWLP_USERDATA, RCAST<LONG_PTR>(self));
        } else {
            self = RCAST<IWindow*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
        }

        if (self) { return self->MessageHandler(msg, wParam, lParam); }

        return ::DefWindowProcA(hwnd, msg, wParam, lParam);
    }
}  // namespace x