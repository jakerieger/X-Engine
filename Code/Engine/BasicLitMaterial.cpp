// Author: Jake Rieger
// Created: 3/18/2025.
//

#include "BasicLitMaterial.hpp"
#include "ShaderManager.hpp"

namespace x {
    BasicLitMaterial::BasicLitMaterial(RenderContext& context, bool transparent) : IMaterial(context, transparent) {
        this->mShader = ShaderManager::GetGraphicsShader(kBasicLitShaderId);
        CreateBuffers();
    }

    void
    BasicLitMaterial::Bind(const TransformMatrices& transforms, const LightState& lights, const Float3 eyePos) const {
        BindShaders();
        BindBuffers();
        UpdateBuffers({transforms, lights, eyePos});
    }

    void BasicLitMaterial::Unbind() const {}

    void BasicLitMaterial::CreateBuffers() {
        mBuffers.Create(mContext, true);
    }

    void BasicLitMaterial::BindBuffers() const {
        mBuffers.Bind(mContext);
    }

    void BasicLitMaterial::UpdateBuffers(const MaterialParameters& params) const {
        mBuffers.Update(mContext, params);
    }
}  // namespace x