#pragma once

#include "Common/Types.hpp"
#include "D3D.hpp"
#include "Math.hpp"
#include "Shader.hpp"
#include "PostProcessSystem.hpp"
#include "Viewport.hpp"
#include "Volatile.hpp"

namespace x {
    class RenderContext;

    class ShadowPass {
        RenderContext& mRenderContext;
        shared_ptr<GraphicsShader> mShader;
        ComPtr<ID3D11Buffer> mShadowParamsCB;
        ComPtr<ID3D11DepthStencilView> mDepthStencilView;
        ComPtr<ID3D11DepthStencilState> mDepthStencilState;
        ComPtr<ID3D11ShaderResourceView> mDepthSRV;

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
        RenderContext& mRenderContext;
        ComPtr<ID3D11RenderTargetView> mRenderTargetView;
        ComPtr<ID3D11DepthStencilView> mDepthStencilView;
        ComPtr<ID3D11DepthStencilState> mDepthStencilState;
        ComPtr<ID3D11ShaderResourceView> mOutputSRV;
        ComPtr<ID3D11Texture2D> mSceneTexture;
        ComPtr<ID3D11SamplerState> mDepthSamplerState;
        ComPtr<ID3D11BlendState> mBlendState;

    public:
        explicit LightPass(RenderContext& context) : mRenderContext(context) {}
        void Initialize(u32 width, u32 height);

        void BeginPass(ID3D11ShaderResourceView* depthSRV, f32 clearColor[4]);
        ID3D11ShaderResourceView* EndPass();

        void Resize(u32 width, u32 height);
    };

    class RenderSystem : public Volatile {
    public:
        RenderSystem(RenderContext& context, Viewport* viewport);

        void Initialize();

        void BeginShadowPass();
        ID3D11ShaderResourceView* EndShadowPass();
        void BeginLightPass(ID3D11ShaderResourceView* depthSRV);
        ID3D11ShaderResourceView* EndLightPass();
        void PostProcessPass(ID3D11ShaderResourceView* input);

        PostProcessSystem* GetPostProcess() {
            return &mPostProcess;
        }

        void OnResize(u32 width, u32 height);

        void SetClearColor(f32 r, f32 g, f32 b, f32 a);

    private:
        friend class Game;

        RenderContext& mRenderContext;
        Viewport* mViewport;
        ShadowPass mShadowPass;
        LightPass mLightPass;
        PostProcessSystem mPostProcess;
        f32 mClearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};

        void UpdateShadowPassParameters(const Matrix& lightViewProj, const Matrix& world);
    };
}  // namespace x