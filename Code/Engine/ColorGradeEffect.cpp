#include "ColorGradeEffect.hpp"

#include "ColorGrade_CS.h"

namespace x {
    bool ColorGradeEffect::Initialize() {
        _computeShader.LoadFromMemory(X_ARRAY_W_SIZE(kColorGrade_CSBytes));
        return CreateResources();
    }

    void ColorGradeEffect::SetSaturation(f32 saturation) { _saturation = saturation; }

    void ColorGradeEffect::SetContrast(f32 contrast) { _contrast = contrast; }

    void ColorGradeEffect::SetTemperature(f32 temperature) { _temperature = temperature; }

    void ColorGradeEffect::SetExposureAdjustment(f32 exposureAdjustment) { _exposureAdjustment = exposureAdjustment; }

    bool ColorGradeEffect::CreateResources() { return CreateConstantBuffer<ColorGradeParams>(); }

    void ColorGradeEffect::UpdateConstants() {
        D3D11_MAPPED_SUBRESOURCE mapped;
        if (SUCCEEDED(
            _renderer.GetDeviceContext()->Map(_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            auto* params = CAST<ColorGradeParams*>(mapped.pData);

            params->saturation         = _saturation;
            params->contrast           = _contrast;
            params->temperature        = _temperature;
            params->exposureAdjustment = _exposureAdjustment;

            _renderer.GetDeviceContext()->Unmap(_constantBuffer.Get(), 0);
        }
    }
} // namespace x