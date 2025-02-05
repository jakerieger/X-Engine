#include "RenderPass.hpp"
#include "Renderer.hpp"

namespace x {
    #pragma region RenderSystem
    RenderSystem::RenderSystem(Renderer& renderer) : _shadowPass(renderer), _lightingPass(renderer) {}

    void RenderSystem::Initialize(u32 width, u32 height) {
        OnResize(width, height);

        _shadowPass.Initialize(width, height);
        _lightingPass.Initialize(width, height);
    }

    void RenderSystem::DrawShadowPass() {
        _shadowPass.Draw();
    }

    void RenderSystem::DrawLightingPass() {
        _lightingPass.Draw(_shadowPass.GetDepthSRV());
    }

    void RenderSystem::UpdateShadowParams(const LightState& lights) {
        const Matrix lvp   = XMMatrixTranspose(CalculateLightViewProjection(lights.Sun, 16.0f, 9.0f));
        const Matrix world = XMMatrixTranspose(XMMatrixScaling(1, 1, 1));

        const ShadowPassState state = {.lightViewProj = lvp, .world = world};
        _shadowPass.UpdateState(state);
    }

    void RenderSystem::UpdateLightingParams() {}

    void RenderSystem::RegisterOccluder(const ModelHandle& occluder) {
        _shadowPass.AddOccluder(occluder);
    }

    void RenderSystem::RegisterOpaqueObject(const ModelHandle& object) {
        _lightingPass.AddOpaqueObject(object);
    }

    void RenderSystem::RegisterTransparentObject(const ModelHandle& object) {
        _lightingPass.AddTransparentObject(object);
    }

    void RenderSystem::OnResize(u32 width, u32 height) {
        _width  = width;
        _height = height;
    }
    #pragma endregion

    #pragma region ShadowPass
    ShadowPass::ShadowPass(Renderer& renderer): _renderer(renderer), _vertexShader(renderer),
                                                _pixelShader(renderer) {}

    void ShadowPass::Initialize(u32 width, u32 height) {
        const str shadowMapShader = R"(C:\Users\conta\Code\SpaceGame\Engine\Shaders\Source\ShadowPass.hlsl)";
        _vertexShader.LoadFromFile(shadowMapShader);
        _pixelShader.LoadFromFile(shadowMapShader);

        D3D11_BUFFER_DESC desc{};
        desc.ByteWidth      = sizeof(ShadowMapParams);
        desc.Usage          = D3D11_USAGE_DYNAMIC;
        desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        auto hr = _renderer.GetDevice()->CreateBuffer(&desc, None, &_shadowParamsCB);
        PANIC_IF_FAILED(hr, "Failed to create shadow map constant buffer.")

        D3D11_TEXTURE2D_DESC depthStencilDesc = {};
        depthStencilDesc.Width                = width;
        depthStencilDesc.Height               = height;
        depthStencilDesc.MipLevels            = 1;
        depthStencilDesc.ArraySize            = 1;
        depthStencilDesc.Format               = DXGI_FORMAT_R24G8_TYPELESS;
        depthStencilDesc.SampleDesc.Count     = 1;
        depthStencilDesc.SampleDesc.Quality   = 0;
        depthStencilDesc.Usage                = D3D11_USAGE_DEFAULT;
        depthStencilDesc.BindFlags            = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

        ComPtr<ID3D11Texture2D> depthStencilTexture;
        hr = _renderer.GetDevice()->CreateTexture2D(&depthStencilDesc, None, &depthStencilTexture);
        PANIC_IF_FAILED(hr, "Failed to create Depth Stencil Texture")

        // Create depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
        depthStencilViewDesc.Format                        = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilViewDesc.ViewDimension                 = D3D11_DSV_DIMENSION_TEXTURE2D;
        depthStencilViewDesc.Texture2D.MipSlice            = 0;

        hr = _renderer.GetDevice()->CreateDepthStencilView(depthStencilTexture.Get(),
                                                           &depthStencilViewDesc,
                                                           &_depthStencilView);
        PANIC_IF_FAILED(hr, "Failed to create Depth Stencil View")

        // Create depth stencil state
        D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc = {};
        depthStencilStateDesc.DepthEnable              = TRUE;
        depthStencilStateDesc.DepthWriteMask           = D3D11_DEPTH_WRITE_MASK_ALL;
        depthStencilStateDesc.DepthFunc                = D3D11_COMPARISON_LESS;
        depthStencilStateDesc.StencilEnable            = FALSE;

        hr = _renderer.GetDevice()->CreateDepthStencilState(&depthStencilStateDesc, &_depthStencilState);
        PANIC_IF_FAILED(hr, "Failed to create Depth Stencil State")
    }

    void ShadowPass::Draw() {
        _renderer.GetContext()->OMSetRenderTargets(0, None, _depthStencilView.Get());
        _renderer.GetContext()->OMSetDepthStencilState(_depthStencilState.Get(), 0);
        _renderer.GetContext()->ClearDepthStencilView(_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

        _vertexShader.Bind();
        _pixelShader.Bind();

        _renderer.GetContext()->VSSetConstantBuffers(0, 1, _shadowParamsCB.GetAddressOf());

        for (auto& mesh : _occluders) {
            mesh.Draw();
        }
    }

    void ShadowPass::UpdateState(const ShadowPassState& state) {
        D3D11_MAPPED_SUBRESOURCE mapped;
        const auto hr = _renderer.GetContext()->Map(_shadowParamsCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        if (SUCCEEDED(hr)) {
            auto* params          = CAST<ShadowMapParams*>(mapped.pData);
            params->lightViewProj = state.lightViewProj;
            params->world         = state.world;

            _renderer.GetContext()->Unmap(_shadowParamsCB.Get(), 0);
        }
    }

    void ShadowPass::AddOccluder(const ModelHandle& occluder) {
        _occluders.push_back(occluder);
    }
    #pragma endregion

    #pragma region LightingPass
    void LightingPass::Initialize(u32 width, u32 height) {}

    void LightingPass::Draw(ID3D11ShaderResourceView* depthSRV) {
        _renderer.GetContext()->OMSetRenderTargets(1, _renderTargetView.GetAddressOf(), _depthStencilView.Get());
        _renderer.GetContext()->OMSetDepthStencilState(_depthStencilState.Get(), 0);

        _renderer.GetContext()->ClearRenderTargetView(_renderTargetView.Get(), Colors::Black);
        _renderer.GetContext()->ClearDepthStencilView(_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

        // Iterate over opaque meshes first
        for (auto& object : _opaqueObjects) {
            // TODO: This method needs to apply the material in this pass
            object.Draw();
        }

        for (auto& object : _transparentObjects) {
            object.Draw();
        }
    }

    void LightingPass::UpdateState(const LightingPassState& state) {}

    void LightingPass::AddOpaqueObject(const ModelHandle& object) {
        _opaqueObjects.push_back(object);
    }

    void LightingPass::AddTransparentObject(const ModelHandle& object) {
        _transparentObjects.push_back(object);
    }
    #pragma endregion
}