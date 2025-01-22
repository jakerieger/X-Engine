#include <iostream>
#include "Engine/Game.hpp"

using namespace x;

class SpaceGame final : public IGame {
public:
    SpaceGame() : IGame(GetModuleHandleA(None), "SpaceGame", 1280, 720) {}

    void LoadContent() override {}

    void UnloadContent() override {}

    void Update() override {}

    void Render() override {}

    void OnResize(u32 width, u32 height) override {}
};

int main() {
    SpaceGame game;

    #ifndef NDEBUG
    game.EnableConsole();
    #endif

    game.Run();
    return 0;
}