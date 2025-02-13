#include "Game.hpp"
#include "RenderContext.hpp"
#include "Common/Timer.hpp"

#include <Vendor/imgui/imgui.h>
#include <stdexcept>
#include <thread>

#include "RasterizerState.hpp"
#include "ScriptTypeRegistry.hpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam);

namespace x {
    IGame::IGame(const HINSTANCE instance,
                 str title,
                 const u32 width,
                 const u32 height)
        : _instance(instance), _hwnd(None), _currentWidth(width),
          _currentHeight(height), _title(std::move(title)),
          _renderContext() {}

    IGame::~IGame() {
        if (_consoleEnabled) { FreeConsole(); }
        if (_isRunning) { Shutdown(); }
    }

    void IGame::Run() {
        try {
            Initialize();
        } catch (const std::runtime_error&) {
            MessageBoxA(_hwnd,
                        "An unknown error occured while initializing the engine.",
                        "SpaceGame",
                        MB_OK);
            return;
        }

        _isRunning = true;
        LoadContent(_activeScene.get());

        MSG msg{};
        while (msg.message != WM_QUIT && _isRunning) {
            if (PeekMessageA(&msg, None, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
            } else {
                auto& state = _activeScene->GetState();

                // Only tick engine forward if we're not currently paused
                if (!_isPaused) {
                    // ScopedTimer updateTimer("UpdateTime");

                    _clock.Tick();
                    _activeScene->Update(CAST<f32>(_clock.GetDeltaTime()));
                    Update(state, _clock);
                }

                // Continue rendering whether we're paused or not
                _renderSystem->BeginFrame();
                {
                    // ScopedTimer frameTimer("FrameTime");

                    // Do our depth-only shadow pass first
                    ID3D11ShaderResourceView* depthSRV;
                    {
                        // ScopedTimer _("  - ShadowPass");
                        _renderSystem->BeginShadowPass();
                        RenderDepthOnly(state);
                        depthSRV = _renderSystem->EndShadowPass();
                    }

                    // Do our fully lit pass using our previous depth-only pass as input for our shadow mapping shader
                    ID3D11ShaderResourceView* sceneSRV;
                    {
                        // ScopedTimer _("  - LightPass");
                        _renderSystem->BeginLightPass(depthSRV);
                        RenderScene(state);
                        sceneSRV = _renderSystem->EndLightPass();
                    }

                    // We can now pass our fully lit scene texture to the post processing pipeline to be processed and displayed on screen
                    {
                        // ScopedTimer _("  - PostProcessPass");
                        _renderSystem->PostProcessPass(sceneSRV);
                    }

                    // Draw debug UI last (on top of everything else)
                    {
                        if (_debugUIEnabled) {
                            _debugUI->BeginFrame(); // begin ImGui frame
                            _debugUI->Draw(_renderContext, _clock);
                            // draw built-in debug ui
                            DrawDebugUI(state); // draw user-defined debug ui
                            _devConsole.Draw();
                            // draw developer console last so it overlaps correctly
                            _debugUI->EndFrame(); // end imgui frame
                        }
                    }
                }
                _renderSystem->EndFrame();
            }
        }

        Shutdown();
    }

    void IGame::Quit() {
        _isRunning = false;
        PostQuitMessage(0);
    }

    bool IGame::EnableConsole() {
        AllocConsole();

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

    void IGame::EnableDebugUI() { _debugUIEnabled = true; }

    u32 IGame::GetWidth() const { return _currentWidth; }

    u32 IGame::GetHeight() const { return _currentHeight; }

    f32 IGame::GetAspect() const {
        return CAST<f32>(_currentWidth) / CAST<f32>(_currentHeight);
    }

    void IGame::Update(SceneState& state, const Clock& clock) {}

    void IGame::RenderDepthOnly(const SceneState& state) {
        for (const auto& [entity, model] : state.GetComponents<
                 ModelComponent>()) {
            Matrix world                  = XMMatrixIdentity();
            const auto transformComponent = state.GetComponent<
                TransformComponent>(entity);
            if (transformComponent) {
                world = transformComponent->GetTransformMatrix();
            }
            _renderSystem->UpdateShadowPassParameters(
                state.GetLightState().Sun.lightViewProj,
                XMMatrixTranspose(world));

            model.Draw(_renderContext);
        }
    }

    // This should never modify game state (always iterate as const)
    void IGame::RenderScene(const SceneState& state) {
        for (const auto& [entity, model] : state.GetComponents<
                 ModelComponent>()) {
            Matrix world = XMMatrixIdentity();
            auto view    = state.GetMainCamera().GetViewMatrix();
            auto proj    = state.GetMainCamera().GetProjectionMatrix();

            const auto transformComponent = state.GetComponent<
                TransformComponent>(entity);
            if (transformComponent) {
                world = transformComponent->GetTransformMatrix();
            }
            model.Draw(_renderContext,
                       {world, view, proj},
                       state.GetLightState(),
                       state.GetMainCamera().GetPosition());
        }
    }

    void IGame::Initialize() {
        InitializeWindow();
        InitializeDX();
        InitializeEngine();
    }

    void IGame::Shutdown() {
        UnloadContent();
        _debugUI.reset();

        CoUninitialize();
    }

    void IGame::Pause() { _isPaused = true; }

    void IGame::Resume() { _isPaused = false; }

    void IGame::InitializeWindow() {
        // Initialize COM (for DirectXTex)
        const auto hr = CoInitializeEx(None, COINIT_MULTITHREADED);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to initialize COM");

        WNDCLASSEXA wc{};
        wc.cbSize        = sizeof(WNDCLASSEXA);
        wc.style         = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc   = WndProc;
        wc.hInstance     = _instance;
        wc.lpszClassName = "SpaceGameWindowClass";

        if (!RegisterClassExA(&wc)) {
            throw std::runtime_error("Failed to register window class.");
        }

        _hwnd = CreateWindowExA(WS_EX_APPWINDOW,
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

        ShowWindow(_hwnd, SW_SHOWDEFAULT);
        UpdateWindow(_hwnd);

        X_LOG_INFO("Initialized window")
    }

    void IGame::InitializeDX() {
        _renderContext.Initialize(_hwnd, _currentWidth, _currentHeight);
        _renderSystem = make_unique<RenderSystem>(_renderContext);
        _renderSystem->Initialize(_currentWidth, _currentHeight);

        _activeScene = make_unique<Scene>(_renderContext);

        // Tell the engine that these classes need to handle resizing when the window size changes
        RegisterVolatile(_renderSystem.get());
        _activeScene->RegisterVolatiles(_volatiles);

        RasterizerStates::Init(_renderContext);
        // Setup our rasterizer states for future use
    }

    void IGame::InitializeEngine() {
        // Initialize the script engine
        {
            auto& scriptEngine = ScriptEngine::Get();

            // Register this class
            auto gameGlobal = scriptEngine.GetLuaState().new_usertype<IGame>(
                "Game");
            gameGlobal["Quit"] = [this] { Quit(); };

            // Register other engine types
            scriptEngine.RegisterTypes<
                Float3, TransformComponent, BehaviorEntity>();
        }

        if (_debugUIEnabled) {
            _debugUI = make_unique<DebugUI>(_hwnd, _renderContext);
        }

        _devConsole.RegisterCommand("quit", [this](auto) { Quit(); })
                   .RegisterCommand("close",
                                    [this](auto) {
                                        _devConsole.ToggleVisible();
                                    })
                   .RegisterCommand("p_ShowFrameGraph",
                                    [this](auto args) {
                                        if (args.size() < 1) { return; }
                                        const auto show = CAST<int>(strtol(
                                            args[0].c_str(),
                                            None,
                                            10));
                                        _debugUI->SetShowFrameGraph(
                                            CAST<bool>(show));
                                    })
                   .RegisterCommand("p_ShowDeviceInfo",
                                    [this](auto args) {
                                        if (args.size() < 1) { return; }
                                        const auto show = CAST<int>(strtol(
                                            args[0].c_str(),
                                            None,
                                            10));
                                        _debugUI->SetShowDeviceInfo(
                                            CAST<bool>(show));
                                    })
                   .RegisterCommand("p_ShowFrameInfo",
                                    [this](auto args) {
                                        if (args.size() < 1) { return; }
                                        const auto show = CAST<int>(strtol(
                                            args[0].c_str(),
                                            None,
                                            10));
                                        _debugUI->SetShowFrameInfo(
                                            CAST<bool>(show));
                                    })
                   .RegisterCommand("p_ShowAll",
                                    [this](auto args) {
                                        if (args.size() < 1) { return; }
                                        const auto show = CAST<int>(strtol(
                                            args[0].c_str(),
                                            None,
                                            10));
                                        _debugUI->SetShowFrameGraph(
                                            CAST<bool>(show));
                                        _debugUI->SetShowDeviceInfo(
                                            CAST<bool>(show));
                                        _debugUI->SetShowFrameInfo(
                                            CAST<bool>(show));
                                    })
                   .RegisterCommand("r_Pause", [this](auto) { Pause(); })
                   .RegisterCommand("r_Resume", [this](auto) { Resume(); });
    }

    LRESULT IGame::ResizeHandler(u32 width, u32 height) {
        _currentWidth  = width;
        _currentHeight = height;

        for (const auto& vol : _volatiles) {
            vol->OnResize(width, height);
        }

        OnResize(width, height);
        return S_OK;
    }

    LRESULT IGame::MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) {
        if (_debugUIEnabled && ImGui_ImplWin32_WndProcHandler(
                _hwnd,
                msg,
                wParam,
                lParam))
            return true;

        switch (msg) {
            case WM_DESTROY:
                Quit();
                return 0;

            case WM_SIZE:
                return ResizeHandler(LOWORD(lParam), HIWORD(lParam));

            case WM_KEYDOWN: {
                // Backtick/tilde key
                if (wParam == VK_OEM_3) { _devConsole.ToggleVisible(); }
                if (wParam == VK_ESCAPE) { Quit(); }
            }
                return 0;
        }

        return DefWindowProcA(_hwnd, msg, wParam, lParam);
    }

    LRESULT IGame::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        IGame* game = None;

        if (msg == WM_CREATE) {
            const auto* pCreate = RCAST<CREATESTRUCTA*>(lParam);
            game                = CAST<IGame*>(pCreate->lpCreateParams);
            SetWindowLongPtrA(hwnd, GWLP_USERDATA, RCAST<LONG_PTR>(game));
        } else {
            game = RCAST<IGame*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
        }

        if (game) { return game->MessageHandler(msg, wParam, lParam); }

        return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
} // namespace x