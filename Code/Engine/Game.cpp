#include "Game.hpp"
#include "RenderContext.hpp"
#include "Common/Timer.hpp"
#include "RasterizerState.hpp"
#include "ScriptTypeRegistry.hpp"
#include <imgui.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace x {
    Game::Game(const HINSTANCE instance, str title, const u32 width, const u32 height)
        : _instance(instance), _hwnd(None), _currentWidth(width), _currentHeight(height), _title(std::move(title)),
          _renderContext() {
#ifndef X_DISTRIBUTION
        _debugUIEnabled = true;
#endif
    }

    Game::~Game() {
        if (_isRunning) { Shutdown(); }
        X_LOG_INFO("Game shutdown")
    }

    void Game::Run(const str& initialScene) {
        Initialize();
        TransitionScene(initialScene);

        _isRunning = true;

        MSG msg {};
        while (msg.message != WM_QUIT && _isRunning) {
            if (PeekMessageA(&msg, None, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
                continue;
            }

            _clock.Tick();

            Update();
            RenderFrame();
        }

        Shutdown();
    }

    void Game::Quit() {
        _isRunning = false;
        PostQuitMessage(0);
    }

    u32 Game::GetWidth() const {
        return _currentWidth;
    }

    u32 Game::GetHeight() const {
        return _currentHeight;
    }

    f32 Game::GetAspect() const {
        return CAST<f32>(_currentWidth) / CAST<f32>(_currentHeight);
    }

    void Game::Update() {
        if (!_isPaused) {
            auto& camera = _activeScene->GetState().GetMainCamera();

            if (_mouse.IsCaptured()) {
                const auto deltaX = _input.GetMouseDeltaX();
                const auto deltaY = _input.GetMouseDeltaY();

                if (deltaX != 0.0f || deltaY != 0.0f) {
                    constexpr f32 mouseSensitivity = 0.001f;
                    const auto deltaYaw            = deltaX * mouseSensitivity;
                    const auto deltaPitch          = deltaY * mouseSensitivity;
                    camera.Rotate(deltaPitch, deltaYaw);
                }

                _input.ResetMouseDeltas();
            }

            if (_input.GetKeyDown(KeyCode::D)) { camera.MoveRight(0.1f); }
            if (_input.GetKeyDown(KeyCode::A)) { camera.MoveRight(-0.1f); }
            if (_input.GetKeyDown(KeyCode::W)) { camera.MoveForward(0.1f); }
            if (_input.GetKeyDown(KeyCode::S)) { camera.MoveForward(-0.1f); }

            _activeScene->Update(CAST<f32>(_clock.GetDeltaTime()));
        }
    }

    void Game::RenderDepthOnly(const SceneState& state) {
        for (const auto& [entity, model] : state.GetComponents<ModelComponent>()) {
            Matrix world                  = XMMatrixIdentity();
            const auto transformComponent = state.GetComponent<TransformComponent>(entity);
            if (transformComponent) { world = transformComponent->GetTransformMatrix(); }
            _renderSystem->UpdateShadowPassParameters(state.GetLightState().Sun.lightViewProj,
                                                      XMMatrixTranspose(world));

            model.Draw(_renderContext);
        }
    }

    // This should never modify game state (always iterate as const)
    void Game::RenderScene(const SceneState& state) {
        for (const auto& [entity, model] : state.GetComponents<ModelComponent>()) {
            Matrix world = XMMatrixIdentity();
            auto view    = state.GetMainCamera().GetViewMatrix();
            auto proj    = state.GetMainCamera().GetProjectionMatrix();

            const auto transformComponent = state.GetComponent<TransformComponent>(entity);
            if (transformComponent) { world = transformComponent->GetTransformMatrix(); }
            model.Draw(_renderContext, {world, view, proj}, state.GetLightState(), state.GetMainCamera().GetPosition());
        }
    }

    void Game::RenderFrame() {
        const auto& state = _activeScene->GetState();

        _renderSystem->BeginFrame();
        {
            // Do our depth-only shadow pass first
            _renderSystem->BeginShadowPass();
            RenderDepthOnly(state);
            ID3D11ShaderResourceView* depthSRV = _renderSystem->EndShadowPass();

            // Do our fully lit pass using our previous depth-only pass as input for our shadow mapping shader
            _renderSystem->BeginLightPass(depthSRV);
            RenderScene(state);
            ID3D11ShaderResourceView* sceneSRV = _renderSystem->EndLightPass();

            // We can now pass our fully lit scene texture to the post processing pipeline to be processed and displayed
            // on screen
            _renderSystem->PostProcessPass(sceneSRV);

            // Draw debug UI last (on top of everything else)
            if (_debugUIEnabled) {
                _debugUI->BeginFrame();  // begin ImGui frame
                _debugUI->Draw(_renderContext, _clock);
                _devConsole.Draw();
                _debugUI->EndFrame();  // end imgui frame
            }
        }
        _renderSystem->EndFrame();
    }

    void Game::Initialize() {
        InitializeWindow();
        InitializeDX();
        InitializeEngine();
        X_LOG_INFO("Initialization complete")
    }

    void Game::Shutdown() {
        _activeScene.reset();  // probably isn't even necessary
        CoUninitialize();
    }

    void Game::Pause() {
        _isPaused = true;
    }

    void Game::Resume() {
        _isPaused = false;
    }

    void Game::InitializeWindow() {
        // Initialize COM (for DirectXTex)
        const auto hr = CoInitializeEx(None, COINIT_MULTITHREADED);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to initialize COM");

        WNDCLASSEXA wc {};
        wc.cbSize        = sizeof(WNDCLASSEXA);
        wc.style         = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc   = WndProc;
        wc.hInstance     = _instance;
        wc.lpszClassName = "SpaceGameWindowClass";

        if (!RegisterClassExA(&wc)) { X_LOG_FATAL("Failed to register window class") }

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

        if (!_hwnd) { X_LOG_FATAL("Failed to create window."); }

        ShowWindow(_hwnd, SW_SHOWDEFAULT);
        UpdateWindow(_hwnd);

        X_LOG_DEBUG("Initialized window")
    }

    void Game::InitializeDX() {
        _renderContext.Initialize(_hwnd, (int)_currentWidth, (int)_currentHeight);
        _renderSystem = make_unique<RenderSystem>(_renderContext);
        _renderSystem->Initialize(_currentWidth, _currentHeight);

        // Tell the engine that these classes need to handle resizing when the window size changes
        RegisterVolatile(_renderSystem.get());

        RasterizerStates::Initialize(_renderContext);
        // Setup our rasterizer states for future use

        X_LOG_DEBUG("Initialized DirectX")
    }

    void Game::InitializeEngine() {
        // Initialize the script engine
        {
            auto& lua = _scriptEngine.GetLuaState();

            // register game globals
            auto gameGlobal    = lua.new_usertype<Game>("Game");
            gameGlobal["Quit"] = [this] { Quit(); };
            _input.RegisterLuaGlobals(lua);

            // TODO: register scene globals

            // Register other engine types
            _scriptEngine.RegisterTypes<Float3, TransformComponent, BehaviorEntity, Camera>();
        }

        if (_debugUIEnabled) { _debugUI = make_unique<DebugUI>(_hwnd, _renderContext); }

        _devConsole.RegisterCommand("quit", [this](auto) { Quit(); })
          .RegisterCommand("close", [this](auto) { _devConsole.ToggleVisible(); })
          .RegisterCommand("p_ShowFrameGraph",
                           [this](auto args) {
                               if (args.size() < 1) { return; }
                               const auto show = CAST<int>(strtol(args[0].c_str(), None, 10));
                               _debugUI->SetShowFrameGraph(CAST<bool>(show));
                           })
          .RegisterCommand("p_ShowDeviceInfo",
                           [this](auto args) {
                               if (args.size() < 1) { return; }
                               const auto show = CAST<int>(strtol(args[0].c_str(), None, 10));
                               _debugUI->SetShowDeviceInfo(CAST<bool>(show));
                           })
          .RegisterCommand("p_ShowFrameInfo",
                           [this](auto args) {
                               if (args.size() < 1) { return; }
                               const auto show = CAST<int>(strtol(args[0].c_str(), None, 10));
                               _debugUI->SetShowFrameInfo(CAST<bool>(show));
                           })
          .RegisterCommand("p_ShowAll",
                           [this](auto args) {
                               if (args.size() < 1) { return; }
                               const auto show = CAST<int>(strtol(args[0].c_str(), None, 10));
                               _debugUI->SetShowFrameGraph(CAST<bool>(show));
                               _debugUI->SetShowDeviceInfo(CAST<bool>(show));
                               _debugUI->SetShowFrameInfo(CAST<bool>(show));
                           })
          .RegisterCommand("g_Pause", [this](auto) { Pause(); })
          .RegisterCommand("g_Resume", [this](auto) { Resume(); })
          .RegisterCommand("g_Load", [this](auto args) {
              if (args.size() < 1) { return; }
              const auto& sceneName = args[0];
              const auto scenePath  = "Scenes\\" + sceneName + ".xscn";
              TransitionScene(scenePath);
          });

        X_LOG_DEBUG("Initialized engine")
    }

    void Game::TransitionScene(const str& path) {
        if (path.empty()) {
            X_LOG_WARN("Attempted to load blank scene")
            return;
        }

        _activeScene.reset();
        _activeScene = make_unique<Scene>(_renderContext, _scriptEngine);
        _activeScene->Load(path);
        _activeScene->RegisterVolatiles(_volatiles);
    }

    LRESULT Game::ResizeHandler(u32 width, u32 height) {
        _currentWidth  = width;
        _currentHeight = height;

        for (const auto& vol : _volatiles) {
            vol->OnResize(width, height);
        }

        return S_OK;
    }

    LRESULT Game::MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) {
        if (_debugUIEnabled && ImGui_ImplWin32_WndProcHandler(_hwnd, msg, wParam, lParam)) return true;

        switch (msg) {
            case WM_DESTROY:
                Quit();
                return 0;

            case WM_SIZE:
                return ResizeHandler(LOWORD(lParam), HIWORD(lParam));

            case WM_KEYDOWN: {
                // Backtick/tilde key
                if (wParam == VK_OEM_3) {
                    _devConsole.ToggleVisible();
                    if (_devConsole.IsVisible()) {
                        _input.SetEnabled(false);
                    } else {
                        _input.SetEnabled(true);
                    }
                }

                if (wParam == VK_ESCAPE && _mouse.IsCaptured()) {
                    _mouse.ReleaseMouse(_hwnd);
                    return 0;
                }

                _input.UpdateKeyState((int)wParam, true);
            }
                return 0;
            case WM_KEYUP: {
                _input.UpdateKeyState((int)wParam, false);
            }
                return 0;
            case WM_LBUTTONDOWN: {
                _input.UpdateMouseButtonState(MouseButton::Left, true);

                if (!_mouse.IsCaptured()) { _mouse.CaptureMouse(_hwnd); }
            }
                return 0;
            case WM_LBUTTONUP: {
                _input.UpdateMouseButtonState(MouseButton::Left, false);
            }
                return 0;
            case WM_RBUTTONDOWN: {
                _input.UpdateMouseButtonState(MouseButton::Right, true);
            }
                return 0;
            case WM_RBUTTONUP: {
                _input.UpdateMouseButtonState(MouseButton::Right, false);
            }
                return 0;
            case WM_MOUSEMOVE: {
                _mouse.OnMouseMove(_hwnd, _input, lParam);
            }
                return 0;
            case WM_KILLFOCUS: {
                _mouse.ReleaseMouse(_hwnd);
            }
                return 0;
        }

        return DefWindowProcA(_hwnd, msg, wParam, lParam);
    }

    LRESULT Game::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        Game* game = None;

        if (msg == WM_CREATE) {
            const auto* pCreate = RCAST<CREATESTRUCTA*>(lParam);
            game                = CAST<Game*>(pCreate->lpCreateParams);
            SetWindowLongPtrA(hwnd, GWLP_USERDATA, RCAST<LONG_PTR>(game));
        } else {
            game = RCAST<Game*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
        }

        if (game) { return game->MessageHandler(msg, wParam, lParam); }

        return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
}  // namespace x