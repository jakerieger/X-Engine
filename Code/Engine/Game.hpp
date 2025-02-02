#pragma once

#include "Clock.hpp"
#include "DebugUI.hpp"
#include "DevConsole.hpp"
#include "GameState.hpp"
#include "Common/Types.hpp"
#include "Platform.hpp"
#include "PostProcessSystem.hpp"
#include "Renderer.hpp"
#include "Volatile.hpp"

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
        GameState _state;
        Clock _clock;

    public:
        explicit IGame(HINSTANCE instance, str title, u32 width, u32 height);
        virtual ~IGame();

        // Prevent moves or copies
        IGame(const IGame& other)            = delete;
        IGame(IGame&& other)                 = delete;
        IGame& operator=(const IGame& other) = delete;
        IGame& operator=(IGame&& other)      = delete;

        /// @brief This is the only function that is required to be called on an IGame instance.
        /// Initializes the app, enters into the main loop, and shuts down when the application is closed.
        void Run();

        /// @brief Quits a running IGame instance
        void Quit();

        /// @brief Enables the console window for Win32 apps. Typically enabled for debug builds to show console output.
        bool EnableConsole();

        void EnableDebugUI();

        [[nodiscard]] u32 GetWidth() const;

        [[nodiscard]] u32 GetHeight() const;

        [[nodiscard]] f32 GetAspect() const;

        virtual void LoadContent(GameState& state) = 0;
        virtual void UnloadContent() = 0;
        virtual void Update(GameState& state, const Clock& clock) = 0;
        virtual void Render(const GameState& state) = 0;
        virtual void OnResize(u32 width, u32 height) = 0;
        virtual void DrawDebugUI();

    protected:
        Renderer renderer;
        std::unique_ptr<DebugUI> debugUI;
        vector<Volatile*> volatiles;
        DevConsole devConsole;

        void RegisterVolatile(Volatile* vol) {
            volatiles.push_back(vol);
        }

    private:
        void Initialize();
        void Shutdown();
        void Pause();
        void Resume();

        LRESULT ResizeHandler(u32 width, u32 height);
        LRESULT MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam);
        static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    };
}