#include "RenderSystem.hpp"

namespace x {
    RenderSystem::RenderSystem(Renderer& renderer) : _renderer(renderer) {
        Initialize();
    }

    void RenderSystem::Initialize() {
        CreateConstantBuffers();
        CreateLightStructures();
        SetupPipelines();
    }

    void RenderSystem::UpdateSceneConstants(const SceneConstants& constants) {}

    void RenderSystem::CullLights(const vector<LightData::Light>& lights) {}

    void RenderSystem::AddOpaqueMesh(const Mesh& mesh, const Material& material, const XMMATRIX& worldMatrix) {}

    void RenderSystem::AddTransparentMesh(const Mesh& mesh, const Material& material, const XMMATRIX& worldMatrix) {}

    void RenderSystem::Render() {
        auto* context = _renderer.GetContext();

        context->VSSetConstantBuffers(0, 1, _sceneConstantBuffer.GetAddressOf());
        context->PSSetConstantBuffers(0, 1, _sceneConstantBuffer.GetAddressOf());

        context->OMSetDepthStencilState(_opaqueDepthState.Get(), 0);
        context->OMSetBlendState(_opaqueBlendState.Get(), None, 0xFFFFFFFF);

        for (const auto& item : _opaqueQueue) {
            // Setup per-object constants and render
            // TODO: Implement material and mesh rendering
        }

        context->OMSetDepthStencilState(_transparentDepthState.Get(), 0);
        context->OMSetBlendState(_transparentBlendState.Get(), None, 0xFFFFFFFF);

        for (const auto& item : _transparentQueue) {
            // Set per-object constants and render
            // TODO: Implement material and mesh rendering
        }

        _opaqueQueue.clear();
        _transparentQueue.clear();
    }

    void RenderSystem::CreateConstantBuffers() {
        auto* device = _renderer.GetDevice();

        // Create scene constant buffer
        D3D11_BUFFER_DESC cbDesc = {};
        cbDesc.ByteWidth         = sizeof(SceneConstants);
        cbDesc.Usage             = D3D11_USAGE_DYNAMIC;
        cbDesc.BindFlags         = D3D11_BIND_CONSTANT_BUFFER;
        cbDesc.CPUAccessFlags    = D3D11_CPU_ACCESS_WRITE;

        DX_THROW_IF_FAILED(device->CreateBuffer(&cbDesc, nullptr, &_sceneConstantBuffer));

        // Create light buffer
        cbDesc.ByteWidth = sizeof(LightData::Light) * LightData::kMaxLights;
        DX_THROW_IF_FAILED(device->CreateBuffer(&cbDesc, nullptr, &_lightBuffer));
    }

    void RenderSystem::CreateLightStructures() {}

    void RenderSystem::SetupPipelines() {
        auto* device = _renderer.GetDevice();

        D3D11_DEPTH_STENCIL_DESC depthDesc{};
        depthDesc.DepthEnable    = true;
        depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depthDesc.DepthFunc      = D3D11_COMPARISON_LESS;

        DX_THROW_IF_FAILED(device->CreateDepthStencilState(&depthDesc, &_opaqueDepthState));

        // Modified depth state for transparent objects
        depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        DX_THROW_IF_FAILED(device->CreateDepthStencilState(&depthDesc, &_transparentDepthState));

        // Create blend states
        D3D11_BLEND_DESC blendDesc       = {};
        blendDesc.AlphaToCoverageEnable  = false;
        blendDesc.IndependentBlendEnable = false;

        // We only need to set up RT[0] since IndependentBlendEnable is false
        blendDesc.RenderTarget[0].BlendEnable           = false;
        blendDesc.RenderTarget[0].SrcBlend              = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlend             = D3D11_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        DX_THROW_IF_FAILED(device->CreateBlendState(&blendDesc, &_opaqueBlendState));

        // Modified blend state for transparent objects
        blendDesc.RenderTarget[0].BlendEnable           = true;
        blendDesc.RenderTarget[0].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        DX_THROW_IF_FAILED(device->CreateBlendState(&blendDesc, &_transparentBlendState));
    }
}