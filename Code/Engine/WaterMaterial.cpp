// Author: Jake Rieger
// Created: 3/7/2025.
//

#include "WaterMaterial.hpp"
#include "ShaderManager.hpp"

namespace x {
    WaterMaterial::WaterMaterial(RenderContext& context) : IMaterial(context) {
        mShader = ShaderManager::GetGraphicsShader(kWaterShaderId);
        CreateBuffers();
    }

    void WaterMaterial::Bind(const TransformMatrices& transforms, const LightState& lights, const Float3 eyePos) const {
        BindShaders();
        BindBuffers();
        UpdateBuffers({transforms, lights, eyePos});
    }

    void WaterMaterial::Unbind() const {}

    void WaterMaterial::CreateBuffers() {
        mBuffers.Create(mContext, true);
    }

    void WaterMaterial::BindBuffers() const {
        mBuffers.Bind(mContext);
    }

    void WaterMaterial::UpdateBuffers(const MaterialParameters& params) const {
        mBuffers.Update(mContext, params);
    }
}  // namespace x