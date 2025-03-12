#include "RenderSystem.hpp"
#include "TonemapEffect.hpp"
#include "Material.hpp"

namespace x {
    RenderSystem::RenderSystem(RenderContext& context, Viewport* viewport)
        : mRenderContext(context), mViewport(viewport), mShadowPass(context), mLightPass(context),
          mPostProcessPass(context) {}

    void RenderSystem::Initialize() {
        const auto width  = mViewport->GetWidth();
        const auto height = mViewport->GetHeight();

        // Initialize all our render passes
        mShadowPass.Initialize(width, height);
        mLightPass.Initialize(width, height);
        mPostProcessPass.Initialize(width, height);

        mPostProcessPass.AddEffect<TonemapEffect>()->SetOperator(TonemapOperator::ACES);
    }

    void RenderSystem::OnResize(u32, u32) {
        const auto width  = mViewport->GetWidth();
        const auto height = mViewport->GetHeight();

        mShadowPass.Resize(width, height);
        mLightPass.Resize(width, height);
        mPostProcessPass.Resize(width, height);
    }

    void RenderSystem::SetClearColor(f32 r, f32 g, f32 b, f32 a) {
        mClearColor[0] = r;
        mClearColor[1] = g;
        mClearColor[2] = b;
        mClearColor[3] = a;
    }

    void RenderSystem::BeginShadowPass() {
        mShadowPass.BeginPass();
    }

    void RenderSystem::EndShadowPass(ID3D11ShaderResourceView*& result) const {
        mShadowPass.EndPass(result);
    }

    void RenderSystem::BeginLightPass(ID3D11ShaderResourceView* depthSRV) {
        mLightPass.BeginPass(depthSRV, mClearColor);
    }

    void RenderSystem::EndLightPass(ID3D11ShaderResourceView*& result) const {
        mLightPass.EndPass(result);
    }

    void RenderSystem::ExecutePostProcessPass(ID3D11ShaderResourceView* input) {
        const auto& rtv = mViewport->GetRenderTargetView();
        mViewport->BindRenderTarget();
        mPostProcessPass.Execute(input, rtv.Get());
    }

    void RenderSystem::BlendStateOpaque() const {
        mLightPass.SetOpaqueState();
    }

    void RenderSystem::BlendStateTransparent() const {
        mLightPass.SetTransparentState();
    }

    void RenderSystem::UpdateShadowParams(const Matrix& lightViewProj, const Matrix& world) const {
        mShadowPass.Update(lightViewProj, world);
    }
}  // namespace x