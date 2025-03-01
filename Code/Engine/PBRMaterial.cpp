// Author: Jake Rieger
// Created: 2/28/2025.
//

#include "PBRMaterial.hpp"
#include "ShaderManager.hpp"

namespace x {
    PBRMaterial::PBRMaterial(RenderContext& context) : IMaterial(context) {
        mShader = ShaderManager::GetGraphicsShader(kOpaquePBRShaderId);
        CreateBuffers();
    }

    void PBRMaterial::Bind(const TransformMatrices& transforms, const LightState& lights, const Float3 eyePos) const {
        BindShaders();
        BindBuffers();
        UpdateBuffers({transforms, lights, eyePos});

        if (mAlbedoMap.Valid()) { mAlbedoMap->Bind(kAlbedoMapSlot); }
        if (mNormalMap.Valid()) { mNormalMap->Bind(kNormalMapSlot); }
        if (mRoughnessMap.Valid()) { mRoughnessMap->Bind(kRoughnessMapSlot); }
        if (mMetallicMap.Valid()) { mMetallicMap->Bind(kMetallicMapSlot); }
    }

    void PBRMaterial::Unbind() const {
        if (mAlbedoMap.Valid()) { mAlbedoMap->Unbind(kAlbedoMapSlot); }
        if (mNormalMap.Valid()) { mNormalMap->Unbind(kNormalMapSlot); }
        if (mRoughnessMap.Valid()) { mRoughnessMap->Unbind(kRoughnessMapSlot); }
        if (mMetallicMap.Valid()) { mMetallicMap->Unbind(kMetallicMapSlot); }
    }

    void PBRMaterial::SetAlbedoMap(const ResourceHandle<Texture2D>& albedo) {
        mAlbedoMap = albedo;
    }

    void PBRMaterial::SetMetallicMap(const ResourceHandle<Texture2D>& metallic) {
        mMetallicMap = metallic;
    }

    void PBRMaterial::SetRoughnessMap(const ResourceHandle<Texture2D>& roughness) {
        mRoughnessMap = roughness;
    }

    void PBRMaterial::SetNormalMap(const ResourceHandle<Texture2D>& normal) {
        mNormalMap = normal;
    }

    void PBRMaterial::SetTextureMaps(const ResourceHandle<Texture2D>& albedo,
                                     const ResourceHandle<Texture2D>& metallic,
                                     const ResourceHandle<Texture2D>& roughness,
                                     const ResourceHandle<Texture2D>& normal) {
        mAlbedoMap    = albedo;
        mMetallicMap  = metallic;
        mRoughnessMap = roughness;
        mNormalMap    = normal;
    }

    void PBRMaterial::CreateBuffers() {
        mBuffers.Create(mContext);
        // Create additional buffers if needed
    }

    void PBRMaterial::UpdateBuffers(const LitMaterialParameters& params) const {
        mBuffers.Update(mContext, params);
        // Update additional buffers if needed
    }

    void PBRMaterial::BindBuffers() const {
        mBuffers.Bind(mContext);
        // Bind additional buffers if needed
    }
}  // namespace x