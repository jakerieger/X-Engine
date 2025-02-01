#include "Engine/Game.hpp"
#include "Engine/Model.hpp"
#include "Engine/GenericLoader.hpp"
#include "Engine/Material.hpp"
#include "Engine/RasterizerState.hpp"
#include "Common/Str.hpp"
#include "Engine/Texture.hpp"

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
    TextureHandle<Texture2D> _albedoMap;
    TextureHandle<Texture2D> _metallicMap;
    TextureHandle<Texture2D> _roughnessMap;
    // TextureHandle<Texture2D> _aoMap;
    TextureHandle<Texture2D> _normalMap;

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
        _material->SetAlbedo({1.0f, 0.0f, 0.5f});

        auto& camera = state.GetMainCamera();
        camera.SetPosition(XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f));

        auto& sun     = state.GetLightState().Sun;
        sun.intensity = 1.0f;
        sun.color     = {1.0f, 1.0f, 1.0f};
        sun.direction = {-0.57f, 0.57f, -0.57f};

        renderer.GetContext()->RSSetState(RasterizerStates::DefaultSolid.Get());

        // Test texture abstraction(s)
        TextureLoader texLoader(renderer);
        _albedoMap    = texLoader.LoadFromFile2D(ContentPath("Metal_Albedo.dds"));
        _normalMap    = texLoader.LoadFromFile2D(ContentPath("Metal_Normal.dds"));
        _metallicMap  = texLoader.LoadFromFile2D(ContentPath("Metal_Metallic.dds"));
        _roughnessMap = texLoader.LoadFromFile2D(ContentPath("Metal_Roughness.dds"));
    }

    void UnloadContent() override {}

    void Update(GameState& state, const Clock& clock) override {
        _rotationY += CAST<f32>(clock.GetDeltaTime());
        _modelMatrix = XMMatrixRotationY(_rotationY);
    }

    void Render(const GameState& state) override {
        auto view = state.GetMainCamera().GetViewMatrix();
        auto proj = state.GetMainCamera().GetProjectionMatrix();

        TransformMatrices transformMatrices(_modelMatrix, view, proj);

        _albedoMap->Bind(0);
        _metallicMap->Bind(1);
        _roughnessMap->Bind(2);
        _normalMap->Bind(4);

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