#pragma once

#include "Common/Types.hpp"
#include "Math.hpp"
#include "Model.hpp"
#include "Material.hpp"
#include "ResourceManager.hpp"

namespace x {
    class ModelComponent {
    public:
        ModelComponent() = default;

        ModelComponent& SetModelHandle(const ResourceHandle<Model>& model) {
            mModelHandle = model;
            return *this;
        }

        ModelComponent& SetMaterial(const shared_ptr<IMaterial>& material) {
            mMaterial = material;
            return *this;
        }

        ModelComponent& SetCastsShadows(const bool casts) {
            mCastsShadows = casts;
            return *this;
        }

        ModelComponent& SetReceiveShadows(const bool receivesShadows) {
            mReceiveShadows = receivesShadows;
            return *this;
        }

        ModelComponent& SetModelId(const u64 id) {
            mModelId = id;
            return *this;
        }

        ModelComponent& SetMaterialId(const u64 materialId) {
            mMaterialId = materialId;
            return *this;
        }

        template<typename T>
        T* GetMaterialAs() {
            return DCAST<T*>(mMaterial.get());
        }

        shared_ptr<IMaterial> GetMaterial() const {
            return mMaterial;
        }

        bool GetCastsShadows() const {
            return mCastsShadows;
        }

        bool GetReceiveShadows() const {
            return mReceiveShadows;
        }

        u64 GetModelId() const {
            return mModelId;
        }

        u64 GetMaterialId() const {
            return mMaterialId;
        }

        void Draw(RenderContext& context,
                  const TransformMatrices& transforms,
                  const LightState& lights,
                  const Float3& eyePosition) const {
            if (mModelId == 0 || mMaterialId == 0 || !mModelHandle.Valid()) { return; }
            
            mMaterial->Bind(transforms, lights, eyePosition);
            mModelHandle->Draw(context);
            mMaterial->Unbind();
        }

        void Draw(RenderContext& context) const {
            if (mModelHandle.Valid() && mModelId != 0) { mModelHandle->Draw(context); }
        }

        bool Valid() const {
            return mModelHandle.Valid();
        }

    private:
        ResourceHandle<Model> mModelHandle;
        shared_ptr<IMaterial> mMaterial;
        bool mCastsShadows   = true;
        bool mReceiveShadows = true;
        u64 mModelId {0};
        u64 mMaterialId {0};
    };
}  // namespace x