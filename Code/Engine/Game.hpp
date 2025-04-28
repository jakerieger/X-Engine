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
#include "Common/Platform.hpp"
#include "Volatile.hpp"
#include "RenderSystem.hpp"
#include "Scene.hpp"
#include "SceneParser.hpp"
#include "ScriptEngine.hpp"
#include "Viewport.hpp"

namespace x {
    class Game : public EventListener {
        X_CLASS_PREVENT_MOVES_COPIES(Game)
        using SceneMap = unordered_map<str, SceneDescriptor>;

    public:
        explicit Game(RenderContext& context);
        ~Game();

        void Initialize(IWindow* window, Viewport* viewport, const Path& workingDir = Path::Current());
        void Shutdown();
        void Update();
        void RenderFrame() const;
        void TransitionScene(const str& name);
        void TransitionScene(const SceneDescriptor& scene);
        void Resize(u32 width, u32 height) const;
        void Reset();
        void Pause();
        void Resume();

        X_NODISCARD Scene* GetActiveScene() const;
        X_NODISCARD bool SceneValid() const;
        X_NODISCARD PostProcessPass* GetPostProcess() const;
        X_NODISCARD ScriptEngine& GetScriptEngine();
        X_NODISCARD RenderSystem* GetRenderSystem() const;
        X_NODISCARD bool IsInitialized() const;
        X_NODISCARD SceneMap& GetSceneMap();

    private:
        friend class XEditor;

        bool mDebugUIEnabled {false};
        bool mIsRunning {false};
        bool mIsPaused {false};
        bool mIsFocused {true};
        bool mIsInitialized {false};
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
        IWindow* mWindow;
        SceneMap mScenes;

        void InitializeEngine();

        void RenderDepthOnly(const SceneState& state) const;
        void ReloadSceneCache();

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