// Author: Jake Rieger
// Created: 2/18/2025.
//

#pragma once

#include "Common/Types.hpp"
#include "Engine/Game.hpp"
#include "Engine/Viewport.hpp"
#include "Engine/Window.hpp"

using namespace x;

class GameWindow final : public IWindow {
public:
    GameWindow(const str& title, const str& initialScene)
        : IWindow(title, 1600, 900), mGame(mContext), mInitialScene(initialScene) {
        AddListener(&mGame);  // Let our game instance listen to window events (resize, lose focus, etc.)
    }

    void OnInitialize() override {
        // Simply render to the window viewport
        mGame.Initialize(this, mWindowViewport.get());
        mGame.TransitionScene(mInitialScene);
    }

    void OnUpdate() override {
        mGame.Update();
    }

    void OnRender() override {
        mWindowViewport->AttachViewport();
        mWindowViewport->ClearAll();
        mGame.RenderFrame();
    }

private:
    Game mGame;
    str mInitialScene;
};
