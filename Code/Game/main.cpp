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
        const auto starshipFile = R"(C:\Users\conta\Documents\3D Assets\Pickle.glb)";
        _starshipHandle         = ModelHandle::LoadGLTF(renderer, starshipFile);
        if (!_starshipHandle.Valid()) {
            throw std::runtime_error("Failed to load model data.");
        }

        _material = make_shared<PBRMaterial>(renderer);

        auto camera = state.GetMainCamera();
        camera.SetPosition(XMVectorSet(0.f, 1.f, -300.f, 1.0f));
    }

    void UnloadContent() override {}

    void Update(GameState& state) override {}

    void Render(const GameState& state) override {
        auto model = XMMatrixScaling(1.0f, 1.0f, 1.0f);

        XMVECTOR eye = XMVectorSet(0.0f, 5.0f, -10.0f, 0.0f);
        XMVECTOR at  = XMVectorSet(0.0f, 2.0f, 0.0f, 0.0f);
        XMVECTOR up  = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        auto view    = XMMatrixLookAtLH(eye, at, up);

        //auto view = state.GetMainCamera().GetViewMatrix();
        auto proj = state.GetMainCamera().GetProjectionMatrix();

        TransformMatrices transformMatrices(model, view, proj);
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