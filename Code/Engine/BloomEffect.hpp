#pragma once

#include "ComputeEffect.hpp"

namespace x {
    class BloomEffect : public IComputeEffect {
        struct BloomParams {
            f32 threshold;
            f32 intensity;
            f32 screenWidth;
            f32 screenHeight;
        };

        f32 _threshold;
        f32 _intensity;

    public:
        explicit BloomEffect(Renderer& renderer, f32 threshold = 1.0f, f32 intensity = 1.0f)
            : IComputeEffect(renderer), _threshold(threshold), _intensity(intensity) {}

        bool Initialize() override;

    protected:
        bool CreateResources() override;
        void UpdateConstants() override;
    };
}