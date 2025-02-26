#include "TonemapEffect.hpp"

#include "Tonemap_CS.h"

namespace x {
    bool TonemapEffect::Initialize() {
        mComputeShader.LoadFromMemory(X_ARRAY_W_SIZE(kTonemap_CSBytes));
        return CreateResources();
    }

    void TonemapEffect::SetExposure(const f32 exposure) {
        mExposure = exposure;
    }

    void TonemapEffect::SetOperator(const TonemapOperator op) {
        mOp = op;
    }

    bool TonemapEffect::CreateResources() {
        return CreateConstantBuffer<TonemapParams>();
    }

    void TonemapEffect::UpdateConstants() {
        D3D11_MAPPED_SUBRESOURCE mapped;
        if (SUCCEEDED(
              mRenderer.GetDeviceContext()->Map(mConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            auto* params = CAST<TonemapParams*>(mapped.pData);

            params->exposure = mExposure;
            params->op       = CAST<u32>(mOp);

            mRenderer.GetDeviceContext()->Unmap(mConstantBuffer.Get(), 0);
        }
    }
}  // namespace x