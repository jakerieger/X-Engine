#include "Engine/Game.hpp"
#include "Engine/GeometryBuffer.hpp"
#include "Engine/Model.hpp"
#include "Engine/FBXLoader.hpp"
#include "Engine/GenericLoader.hpp"
#include "Engine/Material.hpp"
#include "Engine/RasterizerState.hpp"

using namespace x;

struct Vertex {
    XMFLOAT3 position;
    XMFLOAT4 color;
};

class SpaceGame final : public IGame {
    shared_ptr<PBRMaterial> _material;
    ModelHandle _modelHandle;

public:
    explicit SpaceGame(const HINSTANCE instance) : IGame(instance, "SpaceGame", 1280, 720) {}

    void LoadContent(GameState& state) override {
        RasterizerStates::SetupRasterizerStates(renderer);

        const auto starshipFile = R"(C:\Users\conta\Documents\3D Assets\monke.glb)";

        GenericLoader loader(renderer);
        const auto modelData = loader.LoadFromFile(starshipFile);

        _modelHandle.SetModelData(modelData);

        if (!_modelHandle.Valid()) {
            throw std::runtime_error("Failed to load model data.");
        }

        _material = make_shared<PBRMaterial>(renderer);
    }

    void UnloadContent() override {}

    void Update(GameState& state) override {}

    void Render(const GameState& state) override {
        renderer.GetContext()->RSSetState(RasterizerStates::DefaultSolid.Get());

        auto model = XMMatrixScaling(1.0f, 1.0f, 1.0f);

        XMVECTOR eye = XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f);
        XMVECTOR at  = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
        XMVECTOR up  = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        auto view    = XMMatrixLookAtLH(eye, at, up);

        auto proj = state.GetMainCamera().GetProjectionMatrix();

        TransformMatrices transformMatrices(model, view, proj);
        _material->Apply(transformMatrices, state.GetLightState());
        _modelHandle.Draw();
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
    // game.EnableDebugUI();
    #endif

    game.Run();
    return 0;
}