#include "Engine/Game.hpp"

using namespace x;

class SpaceGame final : public IGame {
public:
    SpaceGame() : IGame(GetModuleHandleA(None), "SpaceGame", 1280, 720) {}

    void LoadContent() override {
        SceneConstants constants;
        constants.viewProjection   = XMMatrixIdentity();
        constants.cameraPosition   = XMFLOAT3(0, 0, -5);
        constants.screenDimensions = XMFLOAT2(GetWidth(), GetHeight());
        constants.nearZ            = 0.1f;
        constants.farZ             = 1000.0f;

        renderSystem->UpdateSceneConstants(constants);
    }

    void UnloadContent() override {}

    void Update() override {}

    void Render() override {
        renderer.BeginFrame();

        // Primary forward+ pass
        renderSystem->Render();

        renderer.EndFrame();
    }

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