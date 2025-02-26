#include "ColorGradeEffect.hpp"

#include "ColorGrade_CS.h"

namespace x {
    bool ColorGradeEffect::Initialize() {
        mComputeShader.LoadFromMemory(X_ARRAY_W_SIZE(kColorGrade_CSBytes));
        return CreateResources();
    }

    void ColorGradeEffect::SetSaturation(f32 saturation) {
        mSaturation = saturation;
    }

    void ColorGradeEffect::SetContrast(f32 contrast) {
        mContrast = contrast;
    }

    void ColorGradeEffect::SetTemperature(f32 temperature) {
        mTemperature = temperature;
    }

    void ColorGradeEffect::SetExposureAdjustment(f32 exposureAdjustment) {
        mExposureAdjustment = exposureAdjustment;
    }

    bool ColorGradeEffect::CreateResources() {
        return CreateConstantBuffer<ColorGradeParams>();
    }

    void ColorGradeEffect::UpdateConstants() {
        D3D11_MAPPED_SUBRESOURCE mapped;
        if (SUCCEEDED(
              mRenderer.GetDeviceContext()->Map(mConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            auto* params = CAST<ColorGradeParams*>(mapped.pData);

            params->saturation         = mSaturation;
            params->contrast           = mContrast;
            params->temperature        = mTemperature;
            params->exposureAdjustment = mExposureAdjustment;

            mRenderer.GetDeviceContext()->Unmap(mConstantBuffer.Get(), 0);
        }
    }
}  // namespace x