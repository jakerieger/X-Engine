#pragma once

#include "Clock.hpp"
#include "DebugUI.hpp"
#include "DevConsole.hpp"
#include "EngineCommon.hpp"
#include "SceneState.hpp"
#include "Common/Types.hpp"
#include "Platform.hpp"
#include "Volatile.hpp"
#include "RenderSystem.hpp"
#include "Scene.hpp"

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

namespace x {
    /// @brief Base interface for implementing a game application.
    /// Hooks up windowing, rendering backend, and input among other things.
    class IGame {
        HINSTANCE _instance;
        HWND _hwnd;
        u32 _currentWidth;
        u32 _currentHeight;
        str _title;
        bool _consoleEnabled{false};
        bool _debugUIEnabled{false};
        std::atomic<bool> _isRunning{false};
        std::atomic<bool> _isPaused{false};
        Clock _clock;
        unique_ptr<RenderSystem> _renderSystem;
        unique_ptr<Scene> _activeScene;
        unordered_map<str, unique_ptr<PBRMaterial>> _baseMaterials;

    public:
        explicit IGame(HINSTANCE instance, str title, u32 width, u32 height);
        virtual ~IGame();

        X_CLASS_PREVENT_MOVES_COPIES(IGame)

        /// @brief This is the only function that is required to be called on an IGame instance.
        /// Initializes the app, enters into the main loop, and shuts down when the application is closed.
        void Run();

        /// @brief Quits a running IGame instance.
        /// This is primarily for internal use but can be called at any point without issue.
        void Quit();

        /// @brief Enables the console window for Win32 apps. Typically enabled for debug builds to show console output.
        bool EnableConsole();

        /// @brief Enables the debug UI (profiler, developer console, etc)
        void EnableDebugUI();

        [[nodiscard]] u32 GetWidth() const;
        [[nodiscard]] u32 GetHeight() const;
        [[nodiscard]] f32 GetAspect() const;

        PostProcessSystem* GetPostProcess() {
            return _renderSystem->GetPostProcess();
        }

        virtual void LoadContent(Scene* scene) = 0;
        virtual void UnloadContent() = 0;
        virtual void Update(SceneState& state, const Clock& clock) = 0;
        virtual void OnResize(u32 width, u32 height) = 0;
        virtual void DrawDebugUI(SceneState& state) {}

        void TestLUA() {
            sol::state lua;
            lua.open_libraries();

            lua.script(R"(
print('Hello from LuaJIT!')
)");
        }

    protected:
        std::unique_ptr<DebugUI> _debugUI;
        vector<Volatile*> _volatiles;
        DevConsole _devConsole;
        RenderContext _renderContext;

        void RegisterVolatile(Volatile* vol) {
            _volatiles.push_back(vol);
        }

    private:
        void Initialize();
        void Shutdown();
        void Pause();
        void Resume();

        void InitializeWindow();
        void InitializeDX();
        void InitializeEngine();

        void RenderDepthOnly(const SceneState& state);
        void RenderScene(const SceneState& state);

        LRESULT ResizeHandler(u32 width, u32 height);
        LRESULT MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam);
        static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    };
}