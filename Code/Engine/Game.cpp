#include "Game.hpp"

#include <Vendor/imgui/imgui.h>
#include <stdexcept>
#include <thread>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace x {
    IGame::IGame(const HINSTANCE instance, str title, const u32 width, const u32 height): _instance(instance),
        _hwnd(None), _currentWidth(width), _currentHeight(height),
        _title(std::move(title)), renderer() {}

    IGame::~IGame() {
        if (_consoleEnabled) { ::FreeConsole(); }
        if (_isRunning) { Shutdown(); }
    }

    void IGame::Run() {
        try {
            Initialize();
        } catch (const std::runtime_error& e) {
            ::MessageBoxA(_hwnd, "An unknown error occured while initializing the engine.", "SpaceGame", MB_OK);
            return;
        }

        _isRunning = true;
        LoadContent(_state);

        MSG msg{};
        while (msg.message != WM_QUIT && _isRunning) {
            if (::PeekMessageA(&msg, None, 0, 0, PM_REMOVE)) {
                ::TranslateMessage(&msg);
                ::DispatchMessageA(&msg);
            } else {
                // Only tick engine forward if we're not currently paused
                if (!_isPaused) {
                    _clock.Tick();
                    Update(_state, _clock);
                }

                // Continue rendering whether we're paused or not
                renderer.BeginScenePass();
                Render(_state);
                renderer.EndScenePass();
                renderer.RenderPostProcess();

                if (_debugUIEnabled) {
                    debugUI->BeginFrame(); // begin ImGui frame
                    debugUI->Draw(renderer, _clock); // draw built-in debug ui
                    DrawDebugUI(); // draw user-defined debug ui
                    devConsole.Draw(); // draw developer console last so it overlaps correctly
                    debugUI->EndFrame(); // end imgui frame
                }

                renderer.EndFrame();
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

    void IGame::EnableDebugUI() {
        _debugUIEnabled = true;
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

    void IGame::DrawDebugUI() {}

    void IGame::Initialize() {
        // Initialize COM (for DirectXTex)
        const auto hr = ::CoInitializeEx(None, COINIT_MULTITHREADED);
        PANIC_IF_FAILED(hr, "Failed to initialize COM");

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

        // Tell the engine that these classes need to handle resizing when the window size changes
        RegisterVolatile(&renderer);
        RegisterVolatile(&_state.GetMainCamera());

        if (_debugUIEnabled) {
            debugUI = make_unique<DebugUI>(_hwnd, renderer);
        }

        devConsole.RegisterCommand("quit",
                                   [this](auto) {
                                       Quit();
                                   });

        devConsole.RegisterCommand("close",
                                   [this](auto) {
                                       devConsole.ToggleVisible();
                                   });

        devConsole.RegisterCommand("r_FrameGraph",
                                   [this](auto args) {
                                       if (args.size() < 1) {
                                           return;
                                       }
                                       const auto show = CAST<int>(strtol(args[0].c_str(), None, 10));
                                       debugUI->SetShowFrameGraph(CAST<bool>(show));
                                   });

        devConsole.RegisterCommand("r_DeviceInfo",
                                   [this](auto args) {
                                       if (args.size() < 1) { return; }
                                       const auto show = CAST<int>(strtol(args[0].c_str(), None, 10));
                                       debugUI->SetShowDeviceInfo(CAST<bool>(show));
                                   });

        devConsole.RegisterCommand("r_FrameInfo",
                                   [this](auto args) {
                                       if (args.size() < 1) { return; }
                                       const auto show = CAST<int>(strtol(args[0].c_str(), None, 10));
                                       debugUI->SetShowFrameInfo(CAST<bool>(show));
                                   });

        devConsole.RegisterCommand("r_ToggleAll",
                                   [this](auto args) {
                                       if (args.size() < 1) { return; }
                                       const auto show = CAST<int>(strtol(args[0].c_str(), None, 10));
                                       debugUI->SetShowFrameGraph(CAST<bool>(show));
                                       debugUI->SetShowDeviceInfo(CAST<bool>(show));
                                       debugUI->SetShowFrameInfo(CAST<bool>(show));
                                   });

        devConsole.RegisterCommand("r_Pause",
                                   [this](auto) {
                                       Pause();
                                   });

        devConsole.RegisterCommand("r_Resume", [this](auto) { Resume(); });
    }

    void IGame::Shutdown() {
        UnloadContent();
        debugUI.reset();

        ::CoUninitialize();
    }

    void IGame::Pause() {
        _isPaused = true;
    }

    void IGame::Resume() {
        _isPaused = false;
    }

    LRESULT IGame::ResizeHandler(u32 width, u32 height) {
        _currentWidth  = width;
        _currentHeight = height;

        for (const auto& vol : volatiles) {
            vol->OnResize(width, height);
        }

        OnResize(width, height);
        return S_OK;
    }

    LRESULT IGame::MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) {
        if (_debugUIEnabled && ImGui_ImplWin32_WndProcHandler(_hwnd, msg, wParam, lParam))
            return true;

        switch (msg) {
            case WM_DESTROY:
                Quit();
                return 0;

            case WM_SIZE:
                return ResizeHandler(LOWORD(lParam), HIWORD(lParam));

            case WM_KEYDOWN: {
                // Backtick/tilde key
                if (wParam == VK_OEM_3) {
                    devConsole.ToggleVisible();
                }
            }
                return 0;
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