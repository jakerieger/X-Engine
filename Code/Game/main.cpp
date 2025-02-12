#include "Engine/Game.hpp"
#include "Engine/RasterizerState.hpp"
#include "Common/Timer.hpp"
#include "Common/Filesystem.hpp"
#include "Engine/Scene.hpp"

using namespace x; // engine namespace
using namespace x::Filesystem;

// TODO: Make this relative to the executable path, this is simply for testing (and because I'm lazy)
/// @brief Returns the filename relative to the game's directory ($SolutionDir/Game)
/// @details Example: The filename 'Scenes/monke.xscn' would return $SolutionDir/Game/Scenes/monke.xscn
static str GamePath(const str& filename) {
    auto filePath    = Path(__FILE__).Parent();
    auto contentPath = filePath / ".." / ".." / "Game";
    return (contentPath / filename).Str();
}

class SpaceGame final : public IGame {
public:
    explicit SpaceGame(const HINSTANCE instance) : IGame(instance, "SpaceGame", 1280, 720) {}

    void LoadContent(Scene* scene) override {
        _renderContext.GetDeviceContext()->RSSetState(RasterizerStates::DefaultSolid.Get());
        scene->Load(GamePath("Scenes/monke.xscn"));

        auto& scriptEngine = ScriptEngine::Get();
        scriptEngine.ExecuteString("print('Hello, Lua!')");
    }

    void UnloadContent() override {}

    void Update(SceneState& state, const Clock& clock) override {}

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