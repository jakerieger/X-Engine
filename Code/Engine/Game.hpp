#pragma once

#include "Common/Types.hpp"
#include "Platform.hpp"

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
        std::atomic<bool> _isRunning{false};

    public:
        explicit IGame(HINSTANCE instance, str title, u32 width, u32 height);
        virtual ~IGame();

        /// @brief This is the only function that is required to be called on an IGame instance.
        /// Initializes the app, enters into the main loop, and shuts down when the application is closed.
        void Run();

        /// @brief Quits a running IGame instance
        void Quit();

        /// @brief Enables the console window for Win32 apps. Typically enabled for debug builds to show console output.
        bool EnableConsole();

        [[nodiscard]] u32 GetWidth() const;

        [[nodiscard]] u32 GetHeight() const;

        [[nodiscard]] f32 GetAspect() const;

        virtual void LoadContent() = 0;
        virtual void UnloadContent() = 0;
        virtual void Update() = 0;
        virtual void Render() = 0;
        virtual void OnResize(u32 width, u32 height) = 0;

    private:
        void Initialize();
        void Shutdown();

        LRESULT ResizeHandler(u32 width, u32 height);
        LRESULT MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam);
        static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    };
}