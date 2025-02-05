#pragma once

#include "Common/Types.hpp"
#include "D3D.hpp"
#include "Math.hpp"
#include "Model.hpp"
#include "Shader.hpp"
#include "Lights.hpp"
#include "Material.hpp"
#include "Volatile.hpp"
#include "Common/Panic.hpp"

namespace x {
    class Renderer;

    class ShadowPass {
        Renderer& _renderer;
        VertexShader _vertexShader;
        PixelShader _pixelShader;
        ComPtr<ID3D11Buffer> _shadowParamsCB;
        ComPtr<ID3D11DepthStencilView> _depthStencilView;
        ComPtr<ID3D11DepthStencilState> _depthStencilState;
        ComPtr<ID3D11ShaderResourceView> _depthSRV;

        struct alignas(16) ShadowMapParams {
            Matrix lightViewProj;
            Matrix world;
        };

    public:
        explicit ShadowPass(Renderer& renderer);
        void Initialize(u32 width, u32 height);

        void BeginPass();
        ID3D11ShaderResourceView* EndPass();

        void UpdateState(const ShadowMapParams& state);
    };

    class LightPass {
        Renderer& _renderer;
        ComPtr<ID3D11RenderTargetView> _renderTargetView;
        ComPtr<ID3D11DepthStencilView> _depthStencilView;
        ComPtr<ID3D11DepthStencilState> _depthStencilState;
        ComPtr<ID3D11ShaderResourceView> _outputSRV;
        ComPtr<ID3D11Texture2D> _sceneTexture;

    public:
        explicit LightPass(Renderer& renderer) : _renderer(renderer) {}
        void Initialize(u32 width, u32 height);
        void BeginPass(ID3D11ShaderResourceView* depthSRV);
        ID3D11ShaderResourceView* EndPass();
    };

    class RenderSystem final : public Volatile {
    public:
        explicit RenderSystem(Renderer& renderer);
        void Initialize(u32 width, u32 height);

        void BeginShadowPass();
        ID3D11ShaderResourceView* EndShadowPass();

        void BeginLightPass(ID3D11ShaderResourceView* depthSRV);
        ID3D11ShaderResourceView* EndLightPass();

        void UpdateShadowPassParameters(const Matrix& lightViewProj, const Matrix& world);

        void OnResize(u32 width, u32 height) override;

    private:
        ShadowPass _shadowPass;
        LightPass _lightPass;
        u32 _width  = 0;
        u32 _height = 0;
    };
}