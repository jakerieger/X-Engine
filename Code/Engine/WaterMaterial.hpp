// Author: Jake Rieger
// Created: 3/7/2025.
//

#pragma once

#include "Common/Types.hpp"
#include "EngineCommon.hpp"
#include "Material.hpp"

namespace x {
    class WaterMaterial final : public IMaterial {
    public:
        explicit WaterMaterial(RenderContext& context);

        void Bind(const TransformMatrices& transforms, const LightState& lights, const Float3 eyePos) const override;
        void Unbind() const override;

    protected:
        MaterialBuffers mBuffers;

        void CreateBuffers() override;
        void BindBuffers() const override;
        void UpdateBuffers(const MaterialParameters& params) const override;
    };
}  // namespace x
