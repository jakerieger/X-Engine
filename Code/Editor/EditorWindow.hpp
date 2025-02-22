// Author: Jake Rieger
// Created: 2/18/2025.
//

#pragma once

#include "PropertiesPanel.hpp"
#include "TextureManager.hpp"
#include "Common/Types.hpp"
#include "Engine/Game.hpp"
#include "Engine/Window.hpp"

namespace x::Editor {
    class EditorWindow final : public Window {
    public:
        EditorWindow()
            : Window("XEditor", 1600, 900), _sceneViewport(_context), _textureManager(_context), _game(_context),
              _propertiesPanel(_game) {}

        void OnInitialize() override;
        void OnResize(u32 width, u32 height) override;
        void OnShutdown() override;

        void Update() override;
        void Render() override;

        LRESULT MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) override;

    private:
        Viewport _sceneViewport;
        TextureManager _textureManager;

        Game _game;
        bool _gameRunning = false;

        Camera _editorCamera;
        Camera _sceneCamera;

        unordered_map<str, EntityId> _entities;
        EntityId _selectedEntity;

        // UI Panels
        PropertiesPanel _propertiesPanel;

        void OpenScene(const char* filename);
        void TogglePlayMode();
        void NewScene();

        static void ApplyTheme();
    };
}  // namespace x::Editor
