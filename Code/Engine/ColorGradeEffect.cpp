#include "ColorGradeEffect.hpp"

namespace x {
    bool ColorGradeEffect::Initialize() {
        _computeShader.LoadFromFile(R"(C:\Users\conta\Code\SpaceGame\Engine\Shaders\Source\ColorGrade.hlsl)");
        return CreateResources();
    }

    void ColorGradeEffect::SetSaturation(f32 saturation) { _saturation = saturation; }

    void ColorGradeEffect::SetContrast(f32 contrast) { _contrast = contrast; }

    void ColorGradeEffect::SetTemperature(f32 temperature) { _temperature = temperature; }

    void ColorGradeEffect::SetExposureAdjustment(f32 exposureAdjustment) { _exposureAdjustment = exposureAdjustment; }

    bool ColorGradeEffect::CreateResources() { return CreateConstantBuffer<ColorGradeParams>(); }

    void ColorGradeEffect::UpdateConstants() {
        D3D11_MAPPED_SUBRESOURCE mapped;
        if (SUCCEEDED(_renderer.GetContext()->Map(_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            auto* params = CAST<ColorGradeParams*>(mapped.pData);

            params->saturation         = _saturation;
            params->contrast           = _contrast;
            params->temperature        = _temperature;
            params->exposureAdjustment = _exposureAdjustment;

            _renderer.GetContext()->Unmap(_constantBuffer.Get(), 0);
        }
    }
} // namespace x