// Author: Jake Rieger
// Created: 2/18/2025.
//

#include "Editor.hpp"
#include "Engine/Platform.hpp"
#include "Engine/RasterizerState.hpp"

namespace x::Editor {
    Editor::Editor()
        : _instance(None), _hwnd(None), _currentWidth(1600), _currentHeight(900), _title("XEditor"), _context() {}

    Editor::~Editor() {
        Shutdown();
    }

    int Editor::Run() {
        if (!Initialize()) { return -1; }

        MSG msg {};
        while (msg.message != WM_QUIT) {
            if (::PeekMessage(&msg, None, 0, 0, PM_REMOVE)) {
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
                continue;
            }

            // Do editor stuff here, like update rendering
        }

        Shutdown();

        return 0;
    }

    LRESULT Editor::Quit() {
        ::PostQuitMessage(0);
        return S_OK;
    }

    bool Editor::Initialize() {
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

        _context.Initialize(_hwnd, (int)_currentWidth, (int)_currentHeight);
        RasterizerStates::Initialize(_context);

        return true;
    }

    void Editor::Shutdown() {
        ::CoUninitialize();
    }

    LRESULT Editor::ResizeHandler(u32 width, u32 height) {
        _currentWidth  = width;
        _currentHeight = height;

        return S_OK;
    }

    LRESULT Editor::MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
            case WM_DESTROY:
                return Quit();
            case WM_SIZE:
                return ResizeHandler(LOWORD(lParam), HIWORD(lParam));
            default:
                break;
        }

        return ::DefWindowProcA(_hwnd, msg, wParam, lParam);
    }

    LRESULT Editor::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        Editor* self = None;

        if (msg == WM_CREATE) {
            const auto* pCreate = RCAST<CREATESTRUCTA*>(lParam);
            self                = CAST<Editor*>(pCreate->lpCreateParams);
            ::SetWindowLongPtrA(hwnd, GWLP_USERDATA, RCAST<LONG_PTR>(self));
        } else {
            self = RCAST<Editor*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
        }

        if (self) { return self->MessageHandler(msg, wParam, lParam); }

        return ::DefWindowProcA(hwnd, msg, wParam, lParam);
    }
}  // namespace x::Editor
