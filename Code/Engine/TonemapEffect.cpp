#include "TonemapEffect.hpp"

#include "Tonemap_CS.h"

namespace x {
    bool TonemapEffect::Initialize() {
        _computeShader.LoadFromMemory(X_ARRAY_W_SIZE(kTonemap_CSBytes));
        return CreateResources();
    }

    void TonemapEffect::SetExposure(const f32 exposure) {
        _exposure = exposure;
    }

    void TonemapEffect::SetOperator(const TonemapOperator op) {
        _op = op;
    }

    bool TonemapEffect::CreateResources() {
        return CreateConstantBuffer<TonemapParams>();
    }

    void TonemapEffect::UpdateConstants() {
        D3D11_MAPPED_SUBRESOURCE mapped;
        if (SUCCEEDED(
            _renderer.GetDeviceContext()->Map(_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            auto* params = CAST<TonemapParams*>(mapped.pData);

            params->exposure = _exposure;
            params->op       = CAST<u32>(_op);

            _renderer.GetDeviceContext()->Unmap(_constantBuffer.Get(), 0);
        }
    }
}