#include "Engine/Game.hpp"
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

    // TODO: These can be abstracted into a GeometryBuffer class
    ComPtr<ID3D11Buffer> _vertexBuffer;
    ComPtr<ID3D11Buffer> _indexBuffer;
    UINT _stride     = sizeof(Vertex);
    UINT _offset     = 0;
    UINT _indexCount = 0;

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
        _vertexBuffer.Reset();
        _indexBuffer.Reset();
    }

    void Update() override {}

    void Render() override {
        _vs->Bind();
        _ps->Bind();

        auto* context = renderer.GetContext();
        context->IASetVertexBuffers(0, 1, _vertexBuffer.GetAddressOf(), &_stride, &_offset);
        context->IASetIndexBuffer(_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->DrawIndexed(_indexCount, 0, 0);
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

        D3D11_BUFFER_DESC vertexBufferDesc = {};
        vertexBufferDesc.Usage             = D3D11_USAGE_DEFAULT;
        vertexBufferDesc.ByteWidth         = sizeof(triangleVertices);
        vertexBufferDesc.BindFlags         = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA vertexData = {};
        vertexData.pSysMem                = triangleVertices;

        auto* device = renderer.GetDevice();
        DX_THROW_IF_FAILED(device->CreateBuffer(&vertexBufferDesc, &vertexData, &_vertexBuffer));

        // Define triangle indices
        UINT triangleIndices[] = {0, 1, 2};
        _indexCount            = ARRAYSIZE(triangleIndices);

        // Create index buffer
        D3D11_BUFFER_DESC indexBufferDesc = {};
        indexBufferDesc.Usage             = D3D11_USAGE_DEFAULT;
        indexBufferDesc.ByteWidth         = sizeof(triangleIndices);
        indexBufferDesc.BindFlags         = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA indexData = {};
        indexData.pSysMem                = triangleIndices;

        DX_THROW_IF_FAILED(device->CreateBuffer(&indexBufferDesc, &indexData, &_indexBuffer));
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