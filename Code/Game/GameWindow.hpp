// Author: Jake Rieger
// Created: 2/18/2025.
//

#pragma once

#include "Common/Types.hpp"
#include "Engine/Game.hpp"
#include "Engine/Viewport.hpp"
#include "Engine/Window.hpp"

using namespace x;

class GameWindow final : public Window {
public:
    GameWindow(const str& title, const str& initialScene)
        : Window(title, 1600, 900), _game(_context), _initialScene(initialScene) {
        AddListener(&_game);  // Let our game instance listen to window events (resize, lose focus, etc.)
    }

    void OnInitialize() override {
        // Simply render to the window viewport
        _game.Initialize(this, _windowViewport.get());
        _game.TransitionScene(_initialScene);
    }

    void Update() override {
        _game.Update();
    }

    void Render() override {
        _windowViewport->AttachViewport();
        _windowViewport->ClearAll();
        _game.RenderFrame();
    }

private:
    Game _game;
    str _initialScene;
};
