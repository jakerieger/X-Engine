// Author: Jake Rieger
// Created: 2/18/2025.
//

#include "Window.hpp"

namespace x {
    Window::Window(const str& title, const int width, const int height) {
        _instance      = None;
        _hwnd          = None;
        _currentWidth  = width;
        _currentHeight = height;
        _title         = title;
    }

    Window::~Window() {}

    int Window::Run() {}

    LRESULT Window::Quit() {}

    bool Window::Initialize() {}

    void Window::Shutdown() {}

    LRESULT Window::ResizeHandler(u32 width, u32 height) {
        _currentWidth  = width;
        _currentHeight = height;
        Emit(WindowResizeEvent(width, height));

        return S_OK;
    }

    LRESULT Window::MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) {}

    LRESULT Window::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {}
}  // namespace x