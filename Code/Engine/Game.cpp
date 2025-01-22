#include "Game.hpp"

#include <stdexcept>

namespace x {
    IGame::IGame(const HINSTANCE instance, str title, const u32 width, const u32 height): _instance(instance),
        _hwnd(None), _currentWidth(width), _currentHeight(height),
        _title(std::move(title)) {}

    IGame::~IGame() {
        if (_consoleEnabled) { ::FreeConsole(); }
        if (_isRunning) { Shutdown(); }
    }

    void IGame::Run() {
        try {
            Initialize();
        } catch (const std::runtime_error& e) {
            if (_consoleEnabled) {
                std::ignore = fprintf(stderr, "Exception thrown when initializing IGame instance: %s\n", e.what());
            } else {
                OutputDebugStringA(e.what());
            }
            return;
        }

        _isRunning = true;
        LoadContent();

        MSG msg{};
        while (msg.message != WM_QUIT && _isRunning) {
            if (::PeekMessageA(&msg, None, 0, 0, PM_REMOVE)) {
                ::TranslateMessage(&msg);
                ::DispatchMessageA(&msg);
            } else {
                Update();
                Render();
            }
        }

        Shutdown();
    }

    void IGame::Quit() {
        _isRunning = false;
        ::PostQuitMessage(0);
    }

    bool IGame::EnableConsole() {
        ::AllocConsole();

        FILE* pStdin  = stdin;
        FILE* pStdout = stdout;
        FILE* pStderr = stderr;

        errno_t res = freopen_s(&pStdin, "CONIN$", "r", stdin);
        if (res) { return false; }
        res = freopen_s(&pStdout, "CONOUT$", "w", stdout);
        if (res) { return false; }
        res = freopen_s(&pStderr, "CONOUT$", "w", stderr);
        if (res) { return false; }

        _consoleEnabled = true;

        return true;
    }

    u32 IGame::GetWidth() const {
        return _currentWidth;
    }

    u32 IGame::GetHeight() const {
        return _currentHeight;
    }

    f32 IGame::GetAspect() const {
        return CAST<f32>(_currentWidth) / CAST<f32>(_currentHeight);
    }

    void IGame::Initialize() {
        WNDCLASSEXA wc{};
        wc.cbSize        = sizeof(WNDCLASSEXA);
        wc.style         = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc   = WndProc;
        wc.hInstance     = _instance;
        wc.lpszClassName = "SpaceGameWindowClass";

        if (!::RegisterClassExA(&wc)) { throw std::runtime_error("Failed to register window class."); }

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

        if (!_hwnd) { throw std::runtime_error("Failed to create window."); }

        ::ShowWindow(_hwnd, SW_SHOWDEFAULT);
        ::UpdateWindow(_hwnd);

        if (!renderer.Initialize(_hwnd, _currentWidth, _currentHeight)) {
            throw std::runtime_error("Failed to intialize renderer.");
        }

        renderSystem = make_unique<RenderSystem>(renderer);
        renderSystem->Initialize();
    }

    void IGame::Shutdown() {
        UnloadContent();
        renderSystem.reset();
    }

    LRESULT IGame::ResizeHandler(u32 width, u32 height) {
        _currentWidth  = width;
        _currentHeight = height;

        if (const auto context = renderer.GetContext(); context) {
            renderer.ResizeSwapchainBuffers(width, height);

            D3D11_VIEWPORT vp;
            vp.TopLeftX = 0.0f;
            vp.TopLeftY = 0.0f;
            vp.Width    = CAST<f32>(width);
            vp.Height   = CAST<f32>(height);
            vp.MinDepth = 0.0f;
            vp.MaxDepth = 1.0f;

            context->RSSetViewports(1, &vp);
        }

        OnResize(width, height);
        return S_OK;
    }

    LRESULT IGame::MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
            case WM_DESTROY:
                Quit();
                return 0;

            case WM_SIZE:
                return ResizeHandler(LOWORD(lParam), HIWORD(lParam));
        }

        return ::DefWindowProcA(_hwnd, msg, wParam, lParam);
    }

    LRESULT IGame::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        IGame* game = None;

        if (msg == WM_CREATE) {
            const auto* pCreate = RCAST<CREATESTRUCTA*>(lParam);
            game                = CAST<IGame*>(pCreate->lpCreateParams);
            ::SetWindowLongPtrA(hwnd, GWLP_USERDATA, RCAST<LONG_PTR>(game));
        } else {
            game = RCAST<IGame*>(::GetWindowLongPtrA(hwnd, GWLP_USERDATA));
        }

        if (game) { return game->MessageHandler(msg, wParam, lParam); }

        return ::DefWindowProcA(hwnd, msg, wParam, lParam);
    }
}