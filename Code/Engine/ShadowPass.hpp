// Author: Jake Rieger
// Created: 3/10/2025.
//

#pragma once

#include "Shader.hpp"
#include "Common/Typedefs.hpp"
#include "Math.hpp"

namespace x {
    class RenderContext;

    class ShadowPass {
        X_ALIGNED_STRUCT ShadowMapParams {
            Matrix mLightViewProjection;
            Matrix mWorld;
        };

    public:
        explicit ShadowPass(RenderContext& context);
        void Initialize(u32 width, u32 height);

        void BeginPass();
        void EndPass(ID3D11ShaderResourceView*& result) const;

        void Update(const Matrix& lightViewProj, const Matrix& world) const;
        void Resize(u32 width, u32 height);

    private:
        RenderContext& mContext;
        shared_ptr<GraphicsShader> mShader;
        ComPtr<ID3D11Buffer> mConstantBuffer;
        ComPtr<ID3D11DepthStencilView> mDepthStencilView;
        ComPtr<ID3D11DepthStencilState> mDepthStencilState;
        ComPtr<ID3D11ShaderResourceView> mShaderResourceView;
    };
}  // namespace x
