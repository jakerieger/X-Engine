#include "Engine/Game.hpp"
#include "Engine/GeometryBuffer.hpp"
#include "Engine/Shader.hpp"

using namespace x;

struct Vertex {
    XMFLOAT3 position;
    XMFLOAT4 color;
};

class SpaceGame final : public IGame {
    // TODO: These can be abstracted into a Material class
    VertexShader* _vs = None;
    PixelShader* _ps  = None;

    GeometryBuffer<Vertex> _geoBuffer;

public:
    explicit SpaceGame(const HINSTANCE instance) : IGame(instance, "SpaceGame", 1280, 720) {}

    void LoadContent() override {
        const auto shaderFile = R"(C:\Users\conta\Code\SpaceGame\Engine\Shaders\Source\Unlit.hlsl)";

        _vs = new VertexShader(renderer);
        _vs->LoadFromFile(shaderFile);

        _ps = new PixelShader(renderer);
        _ps->LoadFromFile(shaderFile);

        CreateBuffers();
    }

    void UnloadContent() override {
        delete _vs;
        delete _ps;
    }

    void Update() override {}

    void Render() override {
        _vs->Bind();
        _ps->Bind();

        _geoBuffer.Bind(renderer);

        auto* context = renderer.GetContext();
        context->DrawIndexed(_geoBuffer.GetIndexCount(), 0, 0);
    }

    void DrawDebugUI() override {
        ImGui::Begin("Performance");
        ImGui::End();
    }

    void OnResize(u32 width, u32 height) override {}

private:
    void CreateBuffers() {
        constexpr Vertex triangleVertices[] = {
            {XMFLOAT3(0.0f, 0.5f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)}, // Top vertex (red)
            {XMFLOAT3(0.5f, -0.5f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f)}, // Right vertex (green)
            {XMFLOAT3(-0.5f, -0.5f, 0.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)} // Left vertex (blue)
        };

        constexpr u32 triangleIndices[] = {0, 1, 2};

        _geoBuffer.Create(renderer,
                          triangleVertices,
                          sizeof(triangleVertices),
                          triangleIndices,
                          sizeof(triangleIndices));
    }
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