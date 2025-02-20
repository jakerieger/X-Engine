// Author: Jake Rieger
// Created: 2/18/2025.
//

#pragma once

#include "Common/Types.hpp"
#include "Engine/EngineCommon.hpp"
#include "Engine/Game.hpp"
#include "Engine/Window.hpp"

namespace x::Editor {
    class EditorWindow final : public Window {
    public:
        EditorWindow() : Window("XEditor", 1600, 900), _sceneViewport(_context), _game(_context) {}

        void OnInitialize() override;
        void OnResize(u32 width, u32 height) override;
        void OnShutdown() override;

        void Update() override;
        void Render() override;

        LRESULT MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) override;

    private:
        Viewport _sceneViewport;
        Game _game;
        bool _gameRunning = false;

        void HandleOpenScene(const char* filename);
        static void ApplyTheme();
    };
}  // namespace x::Editor
