#pragma once

#include "Common/Types.hpp"
#include "D3D.hpp"
#include "Math.hpp"
#include "Model.hpp"
#include "Shader.hpp"
#include "Lights.hpp"
#include "Material.hpp"
#include "PostProcessSystem.hpp"
#include "Volatile.hpp"
#include "Common/Panic.hpp"

namespace x {
    class RenderContext;

    class ShadowPass {
        RenderContext& _renderContext;
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
        explicit ShadowPass(RenderContext& context);
        void Initialize(u32 width, u32 height);

        void BeginPass();
        ID3D11ShaderResourceView* EndPass();

        void UpdateState(const ShadowMapParams& state);
        void Resize(u32 width, u32 height);
    };

    class LightPass {
        RenderContext& _renderContext;
        ComPtr<ID3D11RenderTargetView> _renderTargetView;
        ComPtr<ID3D11DepthStencilView> _depthStencilView;
        ComPtr<ID3D11DepthStencilState> _depthStencilState;
        ComPtr<ID3D11ShaderResourceView> _outputSRV;
        ComPtr<ID3D11Texture2D> _sceneTexture;
        ComPtr<ID3D11SamplerState> _depthSamplerState;

    public:
        explicit LightPass(RenderContext& context) : _renderContext(context) {}
        void Initialize(u32 width, u32 height);

        void BeginPass(ID3D11ShaderResourceView* depthSRV);
        ID3D11ShaderResourceView* EndPass();

        void Resize(u32 width, u32 height);
    };

    class RenderSystem final : public Volatile {
    public:
        explicit RenderSystem(RenderContext& context);
        void Initialize(u32 width, u32 height);

        void BeginFrame();
        void EndFrame();

        void BeginShadowPass();
        ID3D11ShaderResourceView* EndShadowPass();
        void BeginLightPass(ID3D11ShaderResourceView* depthSRV);
        ID3D11ShaderResourceView* EndLightPass();
        void PostProcessPass(ID3D11ShaderResourceView* input);

        PostProcessSystem* GetPostProcess() {
            return &_postProcess;
        }

        void OnResize(u32 width, u32 height) override;

    private:
        friend class IGame;

        RenderContext& _renderContext;
        ShadowPass _shadowPass;
        LightPass _lightPass;
        PostProcessSystem _postProcess;

        u32 _width  = 0;
        u32 _height = 0;

        ComPtr<ID3D11RenderTargetView> _renderTargetView;
        ComPtr<ID3D11DepthStencilView> _depthStencilView;
        ComPtr<ID3D11DepthStencilState> _depthStencilState;

        void UpdateShadowPassParameters(const Matrix& lightViewProj, const Matrix& world);
    };
}