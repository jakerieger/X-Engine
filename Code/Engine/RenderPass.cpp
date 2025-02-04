#include "RenderPass.hpp"
#include "Renderer.hpp"

namespace x {
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

    void ShadowPass::Begin() {
        _renderer.GetContext()->OMSetRenderTargets(0, None, _depthStencilView.Get());
        _renderer.GetContext()->OMSetDepthStencilState(_depthStencilState.Get(), 0);

        _renderer.GetContext()->ClearDepthStencilView(_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
        _vertexShader.Bind();
        _pixelShader.Bind();
        _renderer.GetContext()->VSSetConstantBuffers(0, 1, _shadowParamsCB.GetAddressOf());
    }

    void ShadowPass::End() {
        // _renderer.ClearDepthStencil();
        // _renderer.ClearColor();
    }

    void ShadowPass::UpdateParams(const Matrix& lightViewProj, const Matrix& world) {
        D3D11_MAPPED_SUBRESOURCE mapped;
        const auto hr = _renderer.GetContext()->Map(_shadowParamsCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        if (SUCCEEDED(hr)) {
            auto* params          = CAST<ShadowMapParams*>(mapped.pData);
            params->lightViewProj = lightViewProj;
            params->world         = world;

            _renderer.GetContext()->Unmap(_shadowParamsCB.Get(), 0);
        }
    }
}