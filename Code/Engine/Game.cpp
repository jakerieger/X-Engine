#include "Game.hpp"
#include "RenderContext.hpp"
#include "Common/Timer.hpp"
#include "RasterizerState.hpp"
#include "ScriptTypeRegistry.hpp"
#include "Window.hpp"
#include <imgui.h>

#include "AssetManager.hpp"
#include "ShaderManager.hpp"
#include "StaticResources.hpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace x {
    void Game::Update(bool paused) {
        mClock.Tick();

        if ((!mIsPaused || !mIsFocused) && !paused) {
            auto& camera = mActiveScene->GetState().GetMainCamera();

            if (mMouse.IsCaptured()) {
                const auto deltaX = mInput.GetMouseDeltaX();
                const auto deltaY = mInput.GetMouseDeltaY();

                if (deltaX != 0.0f || deltaY != 0.0f) {
                    constexpr f32 mouseSensitivity = 0.001f;
                    const auto deltaYaw            = deltaX * mouseSensitivity;
                    const auto deltaPitch          = deltaY * mouseSensitivity;
                    camera.Rotate(deltaPitch, deltaYaw);
                }

                mInput.ResetMouseDeltas();
            }

            if (mInput.GetKeyDown(KeyCode::D)) { camera.MoveRight(0.1f); }
            if (mInput.GetKeyDown(KeyCode::A)) { camera.MoveRight(-0.1f); }
            if (mInput.GetKeyDown(KeyCode::W)) { camera.MoveForward(0.1f); }
            if (mInput.GetKeyDown(KeyCode::S)) { camera.MoveForward(-0.1f); }

            mActiveScene->Update(CAST<f32>(mClock.GetDeltaTime()));
        }
    }

    void Game::RenderDepthOnly(const SceneState& state) const {
        if (mActiveScene->GetNumEntities() == 0) return;

        for (const auto& [entity, model] : state.GetComponents<ModelComponent>()) {
            Matrix world                  = XMMatrixIdentity();
            const auto transformComponent = state.GetComponent<TransformComponent>(entity);
            if (transformComponent) { world = transformComponent->GetTransformMatrix(); }
            mRenderSystem->UpdateShadowPassParameters(state.GetLightState().Sun.lightViewProj,
                                                      XMMatrixTranspose(world));

            model.Draw(mRenderContext);
        }
    }

    // This should never modify game state (always iterate as const)
    void Game::RenderScene(const SceneState& state) const {
        if (mActiveScene->GetNumEntities() == 0) return;
        // for (const auto& [entity, model] : state.GetComponents<ModelComponent>()) {
        //     if (model.GetMaterial()->Transparent()) { continue; }
        //
        //     Matrix world = XMMatrixIdentity();
        //     auto view    = state.GetMainCamera().GetViewMatrix();
        //     auto proj    = state.GetMainCamera().GetProjectionMatrix();
        //
        //     const auto transformComponent = state.GetComponent<TransformComponent>(entity);
        //     if (transformComponent) { world = transformComponent->GetTransformMatrix(); }
        //     model.Draw(mRenderContext, {world, view, proj}, state.GetLightState(),
        //     state.GetMainCamera().GetPosition());
        // }
    }

    void Game::RegisterEventHandlers() {
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

    void Game::RenderFrame() {
        if (!mIsFocused) return;

        const auto& state = mActiveScene->GetState();

        {
            // Do our depth-only shadow pass first
            mRenderSystem->BeginShadowPass();
            RenderDepthOnly(state);
            ID3D11ShaderResourceView* depthSRV = mRenderSystem->EndShadowPass();

            // Do our fully lit pass using our previous depth-only pass as input for our shadow mapping shader
            mRenderSystem->BeginLightPass(depthSRV);
            if (mActiveScene->GetNumEntities() > 0) {
                mActiveScene->DrawOpaque();
                mActiveScene->DrawTransparent();
            }
            ID3D11ShaderResourceView* sceneSRV = mRenderSystem->EndLightPass();

            // We can now pass our fully lit scene texture to the post processing pipeline to be processed and displayed
            // on screen
            mRenderSystem->PostProcessPass(sceneSRV);

            // Draw debug UI last (on top of everything else)
            if (mDebugUIEnabled) {
                mDebugUI->BeginFrame();  // begin ImGui frame
                mDebugUI->Draw(mRenderContext, mClock);
                mDevConsole.Draw();
                mDebugUI->EndFrame();  // end imgui frame
            }
        }
    }

    void Game::OnResize(u32 width, u32 height) const {
        for (const auto& v : mVolatiles) {
            v->OnResize(width, height);
        }
    }

    void Game::OnKeyDown(u32 key) {
        mInput.UpdateKeyState(key, true);

        if (mWindow) {
            if (key == KeyCode::Escape && mMouse.IsCaptured()) { mMouse.ReleaseMouse(mWindow->GetHandle()); }
        }
    }

    void Game::OnKeyUp(u32 key) {
        mInput.UpdateKeyState(key, false);
    }

    void Game::OnMouseButtonDown(u32 button) {
        mInput.UpdateMouseButtonState(button, true);

        if (mWindow) {
            if (button == MouseButton::Left) {
                if (!mMouse.IsCaptured()) { mMouse.CaptureMouse(mWindow->GetHandle()); }
            }
        }
    }

    void Game::OnMouseButtonUp(u32 button) {
        mInput.UpdateMouseButtonState(button, false);
    }

    void Game::OnMouseMove(u32 x, u32 y) {
        if (mWindow) { mMouse.OnMouseMove(mWindow->GetHandle(), mInput, x, y); }
    }

    void Game::OnLostFocus() {
        if (mWindow) { mMouse.ReleaseMouse(mWindow->GetHandle()); }
        mIsFocused = false;
    }

    void Game::OnGainedFocus() {
        mIsFocused = true;
    }

    void Game::RegisterVolatile(Volatile* vol) {
        mVolatiles.push_back(vol);
    }

    Game::Game(RenderContext& context) : mRenderContext(context) {
        RegisterEventHandlers();
    }

    Game::~Game() {
        Shutdown();
    }

    void Game::Initialize(Window* window, Viewport* viewport, const Filesystem::Path& workingDir) {
        mWindow = window;

        // These need to be loaded first before the rest of the engine can use them!
        if (!ShaderManager::LoadShaders(mRenderContext)) { X_LOG_FATAL("Failed to load shaders!"); }
        if (!AssetManager::LoadAssets(workingDir)) { X_LOG_FATAL("Failed to load assets"); }

        // Find and load all of our scene descriptors
        const auto sceneIds = AssetManager::GetScenes();
        for (const auto& scene : sceneIds) {
            auto sceneData = AssetManager::GetAssetData(scene);
            if (!sceneData) { X_LOG_FATAL("Failed to load scene data!"); }
            SceneDescriptor descriptor;
            SceneParser::Parse(*sceneData, descriptor);
            mScenes[descriptor.mName] = descriptor;
        }

        mRenderSystem = make_unique<RenderSystem>(mRenderContext, viewport);
        mRenderSystem->Initialize();
        mRenderSystem->SetClearColor(0.392f, 0.584f, 0.929f, 1.f);  // Cornflower Blue

        RegisterVolatile(mRenderSystem.get());
        InitializeEngine();

        mActiveScene = make_unique<Scene>(mRenderContext, mScriptEngine);
        X_LOG_INFO("Initialization complete")
    }

    void Game::Shutdown() {
        mRenderSystem.reset();
        mActiveScene.reset();  // probably isn't even necessary
        mWindow = nullptr;
    }

    void Game::Pause() {
        mIsPaused = true;
    }

    void Game::Resume() {
        mIsPaused = false;
    }

    void Game::InitializeEngine() {
        // Initialize the script engine
        {
            auto& lua = mScriptEngine.GetLuaState();

            // register game globals
            auto gameGlobal    = lua.new_usertype<Game>("Game");
            gameGlobal["Quit"] = [this] { mWindow->Quit(); };
            mInput.RegisterLuaGlobals(lua);

            // TODO: register scene globals

            // Register other engine types
            mScriptEngine.RegisterTypes<Float3, TransformComponent, BehaviorEntity, Camera>();
        }

        if (mDebugUIEnabled) { mDebugUI = make_unique<DebugUI>(mRenderContext); }

        mDevConsole.RegisterCommand("quit", [this](auto) { mWindow->Quit(); })
          .RegisterCommand("close", [this](auto) { mDevConsole.ToggleVisible(); })
          .RegisterCommand("p_ShowFrameGraph",
                           [this](auto args) {
                               if (args.size() < 1) { return; }
                               const auto show = CAST<int>(strtol(args[0].c_str(), nullptr, 10));
                               mDebugUI->SetShowFrameGraph(CAST<bool>(show));
                           })
          .RegisterCommand("p_ShowDeviceInfo",
                           [this](auto args) {
                               if (args.size() < 1) { return; }
                               const auto show = CAST<int>(strtol(args[0].c_str(), nullptr, 10));
                               mDebugUI->SetShowDeviceInfo(CAST<bool>(show));
                           })
          .RegisterCommand("p_ShowFrameInfo",
                           [this](auto args) {
                               if (args.size() < 1) { return; }
                               const auto show = CAST<int>(strtol(args[0].c_str(), nullptr, 10));
                               mDebugUI->SetShowFrameInfo(CAST<bool>(show));
                           })
          .RegisterCommand("p_ShowAll",
                           [this](auto args) {
                               if (args.size() < 1) { return; }
                               const auto show = CAST<int>(strtol(args[0].c_str(), nullptr, 10));
                               mDebugUI->SetShowFrameGraph(CAST<bool>(show));
                               mDebugUI->SetShowDeviceInfo(CAST<bool>(show));
                               mDebugUI->SetShowFrameInfo(CAST<bool>(show));
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

    void Game::TransitionScene(const str& name) {
        if (name.empty()) {
            X_LOG_WARN("Attempted to load blank scene")
            return;
        }

        mActiveScene.reset();
        mActiveScene = make_unique<Scene>(mRenderContext, mScriptEngine);
        mActiveScene->Load(mScenes[name]);
        mActiveScene->RegisterVolatiles(mVolatiles);
        mActiveScene->Update(0.0f);
    }

    void Game::Resize(u32 width, u32 height) const {
        OnResize(width, height);
    }

    Scene* Game::GetActiveScene() const {
        return mActiveScene.get();
    }

    bool Game::SceneValid() const {
        if (mActiveScene.get() == nullptr) { return false; }
        // additional checks here...
        return true;
    }

    PostProcessSystem* Game::GetPostProcess() const {
        return mRenderSystem->GetPostProcess();
    }

    ScriptEngine& Game::GetScriptEngine() {
        return mScriptEngine;
    }

    RenderSystem* Game::GetRenderSystem() const {
        return mRenderSystem.get();
    }
}  // namespace x