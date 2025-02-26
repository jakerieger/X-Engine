#pragma once

#include "ComputeEffect.hpp"

namespace x {
    enum class TonemapOperator : u32 {
        ACES,
        Reinhard,
        Filmic,
        Linear,
    };

    class TonemapEffect final : public IComputeEffect {
        struct alignas(16) TonemapParams {
            f32 exposure;
            u32 op;
            f32 mPad[2];
        };

        f32 mExposure       = 1.0f;
        TonemapOperator mOp = TonemapOperator::ACES;

    public:
        explicit TonemapEffect(RenderContext& renderer) : IComputeEffect(renderer) {}

        bool Initialize() override;

        void SetExposure(f32 exposure);
        void SetOperator(TonemapOperator op);

    protected:
        bool CreateResources() override;
        void UpdateConstants() override;
    };
}  // namespace x