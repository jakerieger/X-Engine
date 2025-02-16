#pragma once

#include "Clock.hpp"
#include "DebugUI.hpp"
#include "DevConsole.hpp"
#include "EngineCommon.hpp"
#include "Input.hpp"
#include "Mouse.hpp"
#include "SceneState.hpp"
#include "Common/Types.hpp"
#include "Platform.hpp"
#include "Volatile.hpp"
#include "RenderSystem.hpp"
#include "Scene.hpp"
#include "ScriptEngine.hpp"

namespace x {
    /// @brief Base interface for implementing a game application.
    /// Hooks up windowing, rendering backend, and input among other things.
    class Game {
        X_CLASS_PREVENT_MOVES_COPIES(Game)

    public:
        explicit Game(HINSTANCE instance, str title, u32 width, u32 height);
        virtual ~Game();

        /// @brief This is the only function that is required to be called on an IGame instance.
        /// Initializes the app, enters into the main loop, and shuts down when the application is closed.
        void Run(const str& initialScene);

        /// @brief Quits a running IGame instance.
        /// This is primarily for internal use but can be called at any point without issue.
        void Quit();

        [[nodiscard]] u32 GetWidth() const;
        [[nodiscard]] u32 GetHeight() const;
        [[nodiscard]] f32 GetAspect() const;

        PostProcessSystem* GetPostProcess() {
            return _renderSystem->GetPostProcess();
        }

    private:
        HINSTANCE _instance;
        HWND _hwnd;
        u32 _currentWidth;
        u32 _currentHeight;
        str _title;
        bool _debugUIEnabled{false};
        bool _isRunning{false};
        bool _isPaused{false};
        Clock _clock;
        unique_ptr<RenderSystem> _renderSystem;
        unique_ptr<Scene> _activeScene;
        unordered_map<str, unique_ptr<PBRMaterial>> _baseMaterials;
        std::unique_ptr<DebugUI> _debugUI;
        vector<Volatile*> _volatiles;
        DevConsole _devConsole;
        RenderContext _renderContext;
        ScriptEngine _scriptEngine;
        Input _input;
        Mouse _mouse;

        void Initialize();
        void Shutdown();
        void Pause();
        void Resume();

        void InitializeWindow();
        void InitializeDX();
        void InitializeEngine();

        void TransitionScene(const str& path);

        void Update();
        void RenderDepthOnly(const SceneState& state);
        void RenderScene(const SceneState& state);
        void RenderFrame();

        LRESULT ResizeHandler(u32 width, u32 height);
        LRESULT MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam);
        static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

        void RegisterVolatile(Volatile* vol) {
            _volatiles.push_back(vol);
        }
    };
}