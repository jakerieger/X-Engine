#include "RenderSystem.hpp"
#include "Renderer.hpp"

namespace x {
    #pragma region RenderSystem
    RenderSystem::RenderSystem(Renderer& renderer) : _shadowPass(renderer), _lightPass(renderer) {}

    void RenderSystem::Initialize(u32 width, u32 height) {
        OnResize(width, height);

        _shadowPass.Initialize(width, height);
        _lightPass.Initialize(width, height);
    }

    void RenderSystem::BeginShadowPass() {
        _shadowPass.BeginPass();
    }

    ID3D11ShaderResourceView* RenderSystem::EndShadowPass() {
        return _shadowPass.EndPass();
    }

    void RenderSystem::BeginLightPass(ID3D11ShaderResourceView* depthSRV) {
        _lightPass.BeginPass(depthSRV);
    }

    ID3D11ShaderResourceView* RenderSystem::EndLightPass() {
        return _lightPass.EndPass();
    }

    void RenderSystem::UpdateShadowPassParameters(const Matrix& lightViewProj, const Matrix& world) {
        _shadowPass.UpdateState({lightViewProj, world});
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

        D3D11_TEXTURE2D_DESC depthTexDesc = {};
        depthTexDesc.Width                = width;
        depthTexDesc.Height               = height;
        depthTexDesc.MipLevels            = 1;
        depthTexDesc.ArraySize            = 1;
        depthTexDesc.Format               = DXGI_FORMAT_R32_TYPELESS;
        depthTexDesc.SampleDesc.Count     = 1;
        depthTexDesc.SampleDesc.Quality   = 0;
        depthTexDesc.Usage                = D3D11_USAGE_DEFAULT;
        depthTexDesc.BindFlags            = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

        ComPtr<ID3D11Texture2D> depthStencilTexture;
        hr = _renderer.GetDevice()->CreateTexture2D(&depthTexDesc, None, &depthStencilTexture);
        PANIC_IF_FAILED(hr, "Failed to create Depth Stencil Texture")

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format                          = DXGI_FORMAT_R32_FLOAT; // Format for reading in shader
        srvDesc.ViewDimension                   = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels             = 1;
        srvDesc.Texture2D.MostDetailedMip       = 0;

        hr = _renderer.GetDevice()->CreateShaderResourceView(depthStencilTexture.Get(), &srvDesc, &_depthSRV);
        PANIC_IF_FAILED(hr, "Failed to create Depth SRV")

        // Create depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
        depthStencilViewDesc.Format                        = DXGI_FORMAT_D32_FLOAT;
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

    void ShadowPass::BeginPass() {
        _renderer.GetContext()->OMSetRenderTargets(0, None, _depthStencilView.Get());
        _renderer.GetContext()->OMSetDepthStencilState(_depthStencilState.Get(), 0);
        _renderer.GetContext()->ClearDepthStencilView(_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

        _vertexShader.Bind();
        _pixelShader.Bind();

        _renderer.GetContext()->VSSetConstantBuffers(0, 1, _shadowParamsCB.GetAddressOf());
    }

    ID3D11ShaderResourceView* ShadowPass::EndPass() {
        return _depthSRV.Get();
    }

    void ShadowPass::UpdateState(const ShadowMapParams& state) {
        D3D11_MAPPED_SUBRESOURCE mapped;
        const auto hr = _renderer.GetContext()->Map(_shadowParamsCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        if (SUCCEEDED(hr)) {
            auto* params          = CAST<ShadowMapParams*>(mapped.pData);
            params->lightViewProj = state.lightViewProj;
            params->world         = state.world;

            _renderer.GetContext()->Unmap(_shadowParamsCB.Get(), 0);
        }
    }
    #pragma endregion

    #pragma region LightingPass
    void LightPass::Initialize(u32 width, u32 height) {
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
        auto hr = _renderer.GetDevice()->CreateTexture2D(&depthStencilDesc, None, &depthStencilTexture);
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
        // ==================================================================================================================//
        // ==================================================================================================================//
        D3D11_TEXTURE2D_DESC sceneDesc{};
        sceneDesc.Width            = width;
        sceneDesc.Height           = height;
        sceneDesc.MipLevels        = 1;
        sceneDesc.ArraySize        = 1;
        sceneDesc.Format           = DXGI_FORMAT_R16G16B16A16_FLOAT;
        sceneDesc.SampleDesc.Count = 1;
        sceneDesc.Usage            = D3D11_USAGE_DEFAULT;
        sceneDesc.BindFlags        = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

        hr = _renderer.GetDevice()->CreateTexture2D(&sceneDesc, None, &_sceneTexture);
        PANIC_IF_FAILED(hr, "Failed to create scene texture")

        hr = _renderer.GetDevice()->CreateRenderTargetView(_sceneTexture.Get(), None, &_renderTargetView);
        PANIC_IF_FAILED(hr, "Failed to create render target view.");

        hr = _renderer.GetDevice()->CreateShaderResourceView(_sceneTexture.Get(), None, &_outputSRV);
        PANIC_IF_FAILED(hr, "Failed to create shader resource view")
    }

    void LightPass::BeginPass(ID3D11ShaderResourceView* depthSRV) {
        _renderer.GetContext()->OMSetRenderTargets(1, _renderTargetView.GetAddressOf(), _depthStencilView.Get());
        _renderer.GetContext()->OMSetDepthStencilState(_depthStencilState.Get(), 0);

        _renderer.GetContext()->ClearRenderTargetView(_renderTargetView.Get(), Colors::Black);
        _renderer.GetContext()->ClearDepthStencilView(_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

        _renderer.GetContext()->PSSetShaderResources(5, 1, &depthSRV);
    }

    ID3D11ShaderResourceView* LightPass::EndPass() {
        return _outputSRV.Get();
    }
    #pragma endregion
}