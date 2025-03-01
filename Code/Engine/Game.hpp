#pragma once

#include "Clock.hpp"
#include "DebugUI.hpp"
#include "DevConsole.hpp"
#include "EngineCommon.hpp"
#include "EventListener.hpp"
#include "Input.hpp"
#include "Mouse.hpp"
#include "SceneState.hpp"
#include "Common/Types.hpp"
#include "Platform.hpp"
#include "Volatile.hpp"
#include "RenderSystem.hpp"
#include "Scene.hpp"
#include "ScriptEngine.hpp"
#include "Viewport.hpp"

namespace x {
    class Game : public EventListener {
        X_CLASS_PREVENT_MOVES_COPIES(Game)

    public:
        explicit Game(RenderContext& context);
        ~Game();

        void Initialize(Window* window, Viewport* viewport);
        void Shutdown();
        void Update(bool paused = false);
        void RenderFrame();
        void TransitionScene(const str& path);
        void Resize(u32 width, u32 height) const;

        X_NODISCARD Scene* GetActiveScene() const;
        X_NODISCARD bool SceneValid() const;
        X_NODISCARD PostProcessSystem* GetPostProcess() const;
        X_NODISCARD ScriptEngine& GetScriptEngine();
        X_NODISCARD RenderSystem* GetRenderSystem() const;

    private:
        bool mDebugUIEnabled {false};
        bool mIsRunning {false};
        bool mIsPaused {false};
        bool mIsFocused {true};
        Clock mClock;
        unique_ptr<RenderSystem> mRenderSystem;
        unique_ptr<Scene> mActiveScene;
        std::unique_ptr<DebugUI> mDebugUI;
        vector<Volatile*> mVolatiles;
        DevConsole mDevConsole;
        RenderContext& mRenderContext;
        ScriptEngine mScriptEngine;
        Input mInput;
        Mouse mMouse;
        Window* mWindow;

        void Pause();
        void Resume();

        void InitializeEngine();

        void RenderDepthOnly(const SceneState& state) const;
        void RenderScene(const SceneState& state) const;

        void RegisterEventHandlers();
        void OnResize(u32 width, u32 height) const;

        void OnKeyDown(u32 key);
        void OnKeyUp(u32 key);
        void OnMouseButtonDown(u32 button);
        void OnMouseButtonUp(u32 button);
        void OnMouseMove(u32 x, u32 y);
        void OnLostFocus();
        void OnGainedFocus();

        void RegisterVolatile(Volatile* vol);
    };
}  // namespace x