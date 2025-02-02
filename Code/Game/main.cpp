#include "Engine/Game.hpp"
#include "Engine/Model.hpp"
#include "Engine/GenericLoader.hpp"
#include "Engine/Material.hpp"
#include "Engine/RasterizerState.hpp"
#include "Common/Str.hpp"
#include "Engine/Texture.hpp"

#include <SearchTex.h>
#include <AreaTex.h>

using namespace x;

struct Vertex {
    XMFLOAT3 position;
    XMFLOAT4 color;
};

static str ContentPath(const str& filename) {
    const str root = R"(C:\Users\conta\Code\SpaceGame\Engine\Content\)";
    return root + filename;
}

class SpaceGame final : public IGame {
    shared_ptr<PBRMaterial> _material;
    ModelHandle _modelHandle;
    f32 _rotationY = 0.f;
    Matrix _modelMatrix;

public:
    explicit SpaceGame(const HINSTANCE instance) : IGame(instance, "SpaceGame", 1280, 720) {
        _modelMatrix = XMMatrixIdentity();
    }

    void LoadContent(GameState& state) override {
        RasterizerStates::SetupRasterizerStates(renderer);

        const auto starshipFile = ContentPath("Monke.glb");

        GenericLoader loader(renderer);
        const auto modelData = loader.LoadFromFile(starshipFile);

        _modelHandle.SetModelData(modelData);

        if (!_modelHandle.Valid()) {
            throw std::runtime_error("Failed to load model data.");
        }

        _material = make_shared<PBRMaterial>(renderer);
        TextureLoader texLoader(renderer);
        const auto albedo    = texLoader.LoadFromFile2D(ContentPath("Gold_Albedo.dds"));
        const auto normal    = texLoader.LoadFromFile2D(ContentPath("Gold_Normal.dds"));
        const auto metallic  = texLoader.LoadFromFile2D(ContentPath("Gold_Metallic.dds"));
        const auto roughness = texLoader.LoadFromFile2D(ContentPath("Gold_Roughness.dds"));
        _material->SetTextureMaps(albedo, metallic, roughness, normal);

        auto& camera = state.GetMainCamera();
        camera.SetPosition(XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f));

        auto& sun     = state.GetLightState().Sun;
        sun.enabled   = HLSL_TRUE;
        sun.intensity = 5.0f;
        sun.color     = {1.0f, 1.0f, 1.0f, 1.0f};
        sun.direction = {-0.57f, 0.37f, 0.97f, 1.0f};

        renderer.GetContext()->RSSetState(RasterizerStates::DefaultSolid.Get());
    }

    void UnloadContent() override {}

    void Update(GameState& state, const Clock& clock) override {
        _rotationY += CAST<f32>(clock.GetDeltaTime());
        _modelMatrix = XMMatrixRotationY(_rotationY);
    }

    void Render(const GameState& state) override {
        auto view = state.GetMainCamera().GetViewMatrix();
        auto proj = state.GetMainCamera().GetProjectionMatrix();

        TransformMatrices transformMatrices(_modelMatrix,
                                            view,
                                            proj);

        _material->Apply(transformMatrices, state.GetLightState(), state.GetMainCamera().GetPosition());
        _modelHandle.Draw();
    }

    void DrawDebugUI() override {
        // Draw custom debug UI with ImGui
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