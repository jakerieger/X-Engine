#include "RenderPass.hpp"
#include "Renderer.hpp"

namespace x {
    ShadowPass::ShadowPass(Renderer& renderer): _renderer(renderer), _vertexShader(renderer), _pixelShader(renderer) {
        Initialize();
    }

    void ShadowPass::Initialize() {
        const str shadowMapShader = R"(C:\Users\conta\Code\SpaceGame\Engine\Shaders\Source\ShadowPass.hlsl)";
        _vertexShader.LoadFromFile(shadowMapShader);
        _pixelShader.LoadFromFile(shadowMapShader);

        D3D11_BUFFER_DESC desc{};
        desc.ByteWidth      = sizeof(ShadowMapParams);
        desc.Usage          = D3D11_USAGE_DYNAMIC;
        desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        const auto hr = _renderer.GetDevice()->CreateBuffer(&desc, None, &_shadowParamsCB);
        PANIC_IF_FAILED(hr, "Failed to create shadow map constant buffer.")
    }

    void ShadowPass::Begin() {
        _renderer.ClearDepthStencil();
        _vertexShader.Bind();
        _pixelShader.Bind();
        _renderer.GetContext()->VSSetConstantBuffers(0, 1, _shadowParamsCB.GetAddressOf());
    }

    void ShadowPass::End() {
        _renderer.ClearDepthStencil();
        _renderer.ClearColor();
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