// Author: Jake Rieger
// Created: 3/18/2025.
//

#pragma once

#include "Lights.hpp"
#include "Material.hpp"
#include "Texture.hpp"
#include "TransformMatrices.hpp"

namespace x {
    class BasicLitMaterial final : public IMaterial {
    public:
        explicit BasicLitMaterial(RenderContext& context, bool transparent = false);

        void Bind(const TransformMatrices& transforms, const LightState& lights, const Float3 eyePos) const override;
        void Unbind() const override;

    protected:
        MaterialBuffers mBuffers;

        void CreateBuffers() override;
        void BindBuffers() const override;
        void UpdateBuffers(const MaterialParameters& params) const override;
    };
}  // namespace x
