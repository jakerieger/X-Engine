// Author: Jake Rieger
// Created: 3/10/2025.
//

#pragma once

#include "Common/Types.hpp"
#include "Shader.hpp"
#include "Math.hpp"

namespace x {
    class RenderContext;

    class LightPass {
    public:
        explicit LightPass(RenderContext& context) : mContext(context) {}
        void Initialize(u32 width, u32 height);

        void BeginPass(ID3D11ShaderResourceView* depthMap, f32 clearColor[4]);
        void EndPass(ID3D11ShaderResourceView*& result) const;

        void OpaqueState() const;
        void TransparentState() const;

        void Resize(u32 width, u32 height);

    private:
        RenderContext& mContext;
        ComPtr<ID3D11RenderTargetView> mRenderTargetView;
        ComPtr<ID3D11DepthStencilView> mDepthStencilView;
        ComPtr<ID3D11DepthStencilState> mDepthStencilStateOpaque;
        ComPtr<ID3D11DepthStencilState> mDepthStencilStateTransparent;
        ComPtr<ID3D11ShaderResourceView> mShaderResourceView;
        ComPtr<ID3D11Texture2D> mSceneTexture;
        ComPtr<ID3D11SamplerState> mShadowMapSamplerState;
        ComPtr<ID3D11BlendState> mBlendStateOpaque;
        ComPtr<ID3D11BlendState> mBlendStateTransparent;
    };
}  // namespace x
