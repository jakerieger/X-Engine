// Author: Jake Rieger
// Created: 2/18/2025.
//

#pragma once

#include "Common/Types.hpp"
#include "Engine/Game.hpp"
#include "Engine/Window.hpp"

using namespace x;

class GameWindow final : public Window {
public:
    GameWindow(const str& title, const str& initialScene)
        : Window(title, 1600, 900), _game(x::None, title, 1600, 900), _initialScene(initialScene) {}

    bool Initialize() override {
        // Initialize the base Window first!
        if (!Window::Initialize()) { return false; }

        // Game specific initialization goes here
        // _game.Initialize(this);
    }

private:
    Game _game;
    str _initialScene;
};
