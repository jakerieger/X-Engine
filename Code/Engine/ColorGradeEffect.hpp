#pragma once

#include "ComputeEffect.hpp"
#include "Math.hpp"

namespace x {
    class ColorGradeEffect : public IComputeEffect {
        struct alignas(16) ColorGradeParams {
            f32 saturation;
            f32 contrast;
            f32 temperature;
            f32 exposureAdjustment;
        };

        f32 mSaturation         = 1.0f;
        f32 mContrast           = 1.0f;
        f32 mTemperature        = 6500.0f;
        f32 mExposureAdjustment = 0.0f;

    public:
        explicit ColorGradeEffect(RenderContext& renderer) : IComputeEffect(renderer) {}

        bool Initialize() override;

        void SetSaturation(f32 saturation);
        void SetContrast(f32 contrast);
        void SetTemperature(f32 temperature);
        void SetExposureAdjustment(f32 exposureAdjustment);

    protected:
        bool CreateResources() override;
        void UpdateConstants() override;
    };
}  // namespace x