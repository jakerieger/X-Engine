#include "Game.hpp"
#include "RenderContext.hpp"
#include "Common/Timer.hpp"
#include "RasterizerState.hpp"
#include "ScriptTypeRegistry.hpp"
#include "Window.hpp"
#include <imgui.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace x {
    void Game::Update(bool paused) {
        _clock.Tick();

        if ((!_isPaused || !_isFocused) && !paused) {
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

    void Game::RenderDepthOnly(const SceneState& state) const {
        if (_activeScene->GetNumEntities() == 0) return;

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
    void Game::RenderScene(const SceneState& state) const {
        if (_activeScene->GetNumEntities() == 0) return;

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
        if (!_isFocused) return;

        const auto& state = _activeScene->GetState();

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
    }

    void Game::OnResize(u32 width, u32 height) const {
        for (const auto& v : _volatiles) {
            v->OnResize(width, height);
        }
    }

    void Game::OnKeyDown(u32 key) {
        _input.UpdateKeyState(key, true);

        if (_window) {
            if (key == KeyCode::Escape && _mouse.IsCaptured()) { _mouse.ReleaseMouse(_window->GetHandle()); }
        }
    }

    void Game::OnKeyUp(u32 key) {
        _input.UpdateKeyState(key, false);
    }

    void Game::OnMouseButtonDown(u32 button) {
        _input.UpdateMouseButtonState(button, true);

        if (_window) {
            if (button == MouseButton::Left) {
                if (!_mouse.IsCaptured()) { _mouse.CaptureMouse(_window->GetHandle()); }
            }
        }
    }

    void Game::OnMouseButtonUp(u32 button) {
        _input.UpdateMouseButtonState(button, false);
    }

    void Game::OnMouseMove(u32 x, u32 y) {
        if (_window) { _mouse.OnMouseMove(_window->GetHandle(), _input, x, y); }
    }

    void Game::OnLostFocus() {
        if (_window) { _mouse.ReleaseMouse(_window->GetHandle()); }
        _isFocused = false;
    }

    void Game::OnGainedFocus() {
        _isFocused = true;
    }

    void Game::RegisterVolatile(Volatile* vol) {
        _volatiles.push_back(vol);
    }

    Game::Game(RenderContext& context) : _renderContext(context) {
        RegisterHandler<WindowResizeEvent>(
          [this](const WindowResizeEvent& e) { OnResize(e.GetWidth(), e.GetHeight()); });
        RegisterHandler<KeyPressedEvent>([this](const KeyPressedEvent& e) { OnKeyDown(e.GetKey()); });
        RegisterHandler<KeyReleasedEvent>([this](const KeyReleasedEvent& e) { OnKeyUp(e.GetKey()); });
        RegisterHandler<MouseButtonPressedEvent>(
          [this](const MouseButtonPressedEvent& e) { OnMouseButtonDown(e.GetButton()); });
        RegisterHandler<MouseButtonReleasedEvent>(
          [this](const MouseButtonReleasedEvent& e) { OnMouseButtonUp(e.GetButton()); });
        RegisterHandler<MouseMoveEvent>([this](const MouseMoveEvent& e) { OnMouseMove(e.GetX(), e.GetY()); });
    }

    Game::~Game() {
        Shutdown();
    }

    void Game::Initialize(Window* window, Viewport* viewport) {
        _window = window;

        _renderSystem = make_unique<RenderSystem>(_renderContext, viewport);
        _renderSystem->Initialize();

        RegisterVolatile(_renderSystem.get());
        InitializeEngine();

        _activeScene = make_unique<Scene>(_renderContext, _scriptEngine);
        X_LOG_INFO("Initialization complete")
    }

    void Game::Shutdown() {
        _renderSystem.reset();
        _activeScene.reset();  // probably isn't even necessary
        _window = None;
    }

    void Game::Pause() {
        _isPaused = true;
    }

    void Game::Resume() {
        _isPaused = false;
    }

    void Game::InitializeEngine() {
        // Initialize the script engine
        {
            auto& lua = _scriptEngine.GetLuaState();

            // register game globals
            // auto gameGlobal    = lua.new_usertype<Game>("Game");
            // gameGlobal["Quit"] = [this] { Quit(); };
            // _input.RegisterLuaGlobals(lua);

            // TODO: register scene globals

            // Register other engine types
            _scriptEngine.RegisterTypes<Float3, TransformComponent, BehaviorEntity, Camera>();
        }

        // if (_debugUIEnabled) { _debugUI = make_unique<DebugUI>(_hwnd, _renderContext); }

        // _devConsole.RegisterCommand("quit", [this](auto) { Quit(); })
        //   .RegisterCommand("close", [this](auto) { _devConsole.ToggleVisible(); })
        //   .RegisterCommand("p_ShowFrameGraph",
        //                    [this](auto args) {
        //                        if (args.size() < 1) { return; }
        //                        const auto show = CAST<int>(strtol(args[0].c_str(), None, 10));
        //                        _debugUI->SetShowFrameGraph(CAST<bool>(show));
        //                    })
        //   .RegisterCommand("p_ShowDeviceInfo",
        //                    [this](auto args) {
        //                        if (args.size() < 1) { return; }
        //                        const auto show = CAST<int>(strtol(args[0].c_str(), None, 10));
        //                        _debugUI->SetShowDeviceInfo(CAST<bool>(show));
        //                    })
        //   .RegisterCommand("p_ShowFrameInfo",
        //                    [this](auto args) {
        //                        if (args.size() < 1) { return; }
        //                        const auto show = CAST<int>(strtol(args[0].c_str(), None, 10));
        //                        _debugUI->SetShowFrameInfo(CAST<bool>(show));
        //                    })
        //   .RegisterCommand("p_ShowAll",
        //                    [this](auto args) {
        //                        if (args.size() < 1) { return; }
        //                        const auto show = CAST<int>(strtol(args[0].c_str(), None, 10));
        //                        _debugUI->SetShowFrameGraph(CAST<bool>(show));
        //                        _debugUI->SetShowDeviceInfo(CAST<bool>(show));
        //                        _debugUI->SetShowFrameInfo(CAST<bool>(show));
        //                    })
        //   .RegisterCommand("g_Pause", [this](auto) { Pause(); })
        //   .RegisterCommand("g_Resume", [this](auto) { Resume(); })
        //   .RegisterCommand("g_Load", [this](auto args) {
        //       if (args.size() < 1) { return; }
        //       const auto& sceneName = args[0];
        //       const auto scenePath  = "Scenes\\" + sceneName + ".xscn";
        //       TransitionScene(scenePath);
        //   });

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
        _activeScene->Update(0.0f);
    }

    void Game::Resize(u32 width, u32 height) const {
        OnResize(width, height);
    }
}  // namespace x