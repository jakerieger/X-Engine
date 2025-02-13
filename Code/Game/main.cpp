#include "Engine/Game.hpp"
#include "Engine/RasterizerState.hpp"
#include "Common/Timer.hpp"
#include "Common/Filesystem.hpp"
#include "Engine/Scene.hpp"
#include "Engine/ScriptTypeRegistry.hpp"

using namespace x; // engine namespace
using namespace x::Filesystem;

class SpaceGame final : public IGame {
public:
    explicit SpaceGame(const HINSTANCE instance) : IGame(instance, "SpaceGame", 1280, 720) {}

    void LoadContent(Scene* scene) override {
        _renderContext.GetDeviceContext()->RSSetState(RasterizerStates::DefaultSolid.Get());
        scene->Load(R"(Scenes\monke.xscn)");
    }

    void UnloadContent() override {}

    void Update(SceneState& state, const Clock& clock) override {
        IGame::Update(state, clock);
    }

    void DrawDebugUI(SceneState& state) override {}

    void OnResize(u32 width, u32 height) override {}
};


X_MAIN {
    SpaceGame game(X_MODULE_HANDLE);

    #ifndef X_DISTRIBUTION
    game.EnableDebugUI();
    #endif

    game.Run();
}