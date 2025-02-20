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
    /// @brief Base interface for implementing a game application.
    /// Hooks up windowing, rendering backend, and input among other things.
    class Game : public EventListener {
        X_CLASS_PREVENT_MOVES_COPIES(Game)

    public:
        explicit Game(RenderContext& context);
        ~Game();

        PostProcessSystem* GetPostProcess() {
            return _renderSystem->GetPostProcess();
        }

        void Initialize(Window* window, Viewport* viewport);
        void Shutdown();
        void Update();
        void RenderFrame();
        void TransitionScene(const str& path);
        void Resize(u32 width, u32 height) const;

    private:
        bool _debugUIEnabled {false};
        bool _isRunning {false};
        bool _isPaused {false};
        bool _isFocused {true};
        Clock _clock;
        unique_ptr<RenderSystem> _renderSystem;
        unique_ptr<Scene> _activeScene;
        std::unique_ptr<DebugUI> _debugUI;
        vector<Volatile*> _volatiles;
        DevConsole _devConsole;
        RenderContext& _renderContext;
        ScriptEngine _scriptEngine;
        Input _input;
        Mouse _mouse;
        Window* _window;

        void Pause();
        void Resume();

        void InitializeEngine();

        void RenderDepthOnly(const SceneState& state) const;
        void RenderScene(const SceneState& state) const;

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