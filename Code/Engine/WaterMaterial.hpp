// Author: Jake Rieger
// Created: 3/7/2025.
//

#pragma once

#include "Common/Types.hpp"
#include "EngineCommon.hpp"
#include "Material.hpp"

namespace x {
    X_ALIGNED_STRUCT WaterParameters {
        f32 mTime {0};
        f32 mWaterDepth {1.0f};
        f32 mTransparency {0.8f};
        Float2 mWindDirection {0.7f, 0.7f};
        f32 mWindStrength {0.4f};
    };

    X_ALIGNED_STRUCT WaveParameters {
        Float4 mWaveAmplitudes[2] {{0.35f, 0.25f, 0.18f, 0.12f}, {0.08f, 0.06f, 0.04f, 0.02f}};
        Float4 mWaveFrequencies[2] {{0.8f, 1.2f, 1.6f, 2.0f}, {2.4f, 2.8f, 3.2f, 3.6f}};
        Float4 mWaveSpeeds[2] {{1.0f, 1.1f, 1.2f, 1.3f}, {1.4f, 1.5f, 1.6f, 1.7f}};
        Float4 mWaveDirectionX[2] {{1.0f, 0.7f, 0.3f, -0.1f}, {-0.5f, -0.8f, -1.0f, -0.6f}};
        Float4 mWaveDirectionZ[2] {{0.0f, 0.7f, 0.9f, 1.0f}, {0.8f, 0.6f, 0.0f, -0.8f}};
        Float4 mWavePhases[2] {{0.0f, 0.5f, 1.0f, 1.5f}, {2.0f, 2.5f, 3.0f, 3.5f}};
        i32 mActiveWaveCount {8};
    };

    class WaterMaterial final : public IMaterial {
    public:
        explicit WaterMaterial(RenderContext& context, bool transparent = false);

        void Bind(const TransformMatrices& transforms, const LightState& lights, const Float3 eyePos) const override;
        void Unbind() const override;

        void SetWaveTime(f32 waveTime);

    protected:
        MaterialBuffers mBuffers;
        ComPtr<ID3D11Buffer> mWaterParamsBuffer;
        ComPtr<ID3D11Buffer> mWaveParamsBuffer;

        WaterParameters mWaterParameters;
        WaveParameters mWaveParameters;

        void CreateBuffers() override;
        void BindBuffers() const override;
        void UpdateBuffers(const MaterialParameters& params) const override;
    };
}  // namespace x
