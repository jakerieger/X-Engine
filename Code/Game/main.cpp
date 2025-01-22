#include "Engine/Game.hpp"

using namespace x;

class SpaceGame final : public IGame {
public:
    explicit SpaceGame(const HINSTANCE instance) : IGame(instance, "SpaceGame", 1280, 720) {}

    void LoadContent() override {}

    void UnloadContent() override {}

    void Update() override {}

    void Render() override {
        renderer.BeginFrame(Colors::Black);

        renderer.EndFrame();
    }

    void OnResize(u32 width, u32 height) override {}
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {
    SpaceGame game(hInstance);

    #ifndef NDEBUG
    game.EnableConsole();
    #endif

    game.Run();
    return 0;
}