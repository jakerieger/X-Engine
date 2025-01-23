#include "Engine/Game.hpp"
#include "Engine/GeometryBuffer.hpp"
#include "Engine/Model.hpp"
#include "Engine/Shader.hpp"
#include "Engine/Material.hpp"

using namespace x;

struct Vertex {
    XMFLOAT3 position;
    XMFLOAT4 color;
};

class SpaceGame final : public IGame {
    shared_ptr<PBRMaterial> _material;
    ModelHandle _starshipHandle;

public:
    explicit SpaceGame(const HINSTANCE instance) : IGame(instance, "SpaceGame", 1280, 720) {}

    void LoadContent(GameState& state) override {
        const auto starshipFile = R"(C:\Users\conta\Documents\3D Assets\spaceship\source\SpaceShip\StarShip2.obj)";
        _starshipHandle         = ModelHandle::LoadFromFile(renderer, starshipFile);
        if (!_starshipHandle.Valid()) {
            throw std::runtime_error("Failed to load model data.");
        }

        _material = make_shared<PBRMaterial>(renderer);
    }

    void UnloadContent() override {}

    void Update(GameState& state) override {}

    void Render(const GameState& state) override {
        TransformMatrices transformMatrices(XMMatrixIdentity(),
                                            state.GetMainCamera().GetViewMatrix(),
                                            state.GetMainCamera().GetProjectionMatrix());
        _material->Apply(transformMatrices, state.GetLightState());
        _starshipHandle.Draw(state.GetMainCamera(), {});
    }

    void DrawDebugUI() override {
        ImGui::Begin("Performance");
        ImGui::End();
    }

    void OnResize(u32 width, u32 height) override {}
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {
    SpaceGame game(hInstance);

    #ifndef NDEBUG
    if (!game.EnableConsole()) { return EXIT_FAILURE; }
    game.EnableDebugUI();
    #endif

    game.Run();
    return 0;
}