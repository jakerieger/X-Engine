#pragma once

#include "Common/Types.hpp"
#include "D3D.hpp"
#include "Math.hpp"
#include "Shader.hpp"
#include "Viewport.hpp"
#include "Volatile.hpp"

#include "ShadowPass.hpp"
#include "LightPass.hpp"
#include "PostProcessPass.hpp"

namespace x {
    class RenderSystem final : public Volatile {
    public:
        RenderSystem(RenderContext& context, Viewport* viewport);

        void Initialize();

        void BeginShadowPass();
        void EndShadowPass(ID3D11ShaderResourceView*& result) const;
        void BeginLightPass(ID3D11ShaderResourceView* depthSRV);
        void EndLightPass(ID3D11ShaderResourceView*& result) const;
        void ExecutePostProcessPass(ID3D11ShaderResourceView* input);

        void BlendStateOpaque() const;
        void BlendStateTransparent() const;

        PostProcessPass* GetPostProcess() {
            return &mPostProcessPass;
        }

        void OnResize(u32 width, u32 height) override;
        void SetClearColor(f32 r, f32 g, f32 b, f32 a = 1.0f);

    private:
        friend class Game;

        RenderContext& mRenderContext;
        Viewport* mViewport;
        ShadowPass mShadowPass;
        LightPass mLightPass;
        PostProcessPass mPostProcessPass;
        f32 mClearColor[4] = {0.1f, 0.1f, 0.1f, 1.0f};

        void UpdateShadowParams(const Matrix& lightViewProj, const Matrix& world) const;
    };
}  // namespace x