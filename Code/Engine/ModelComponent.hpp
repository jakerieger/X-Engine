#pragma once

#include "Common/Types.hpp"
#include "Math.hpp"
#include "Model.hpp"
#include "Material.hpp"
#include "ResourceManager.hpp"

namespace x {
    class ModelComponent {
        ResourceHandle<Model> mModelHandle;
        PBRMaterialInstance mMaterialInstance;
        bool mCastsShadows = true;

    public:
        ModelComponent() = default;

        ModelComponent& SetModelHandle(const ResourceHandle<Model>& model) {
            mModelHandle = model;
            return *this;
        }

        ModelComponent& SetMaterial(const shared_ptr<PBRMaterial>& material) {
            mMaterialInstance.SetBaseMaterial(material);
            return *this;
        }

        ModelComponent& SetCastsShadows(const bool casts) {
            mCastsShadows = casts;
            return *this;
        }

        PBRMaterialInstance& GetMaterialInstance() {
            return mMaterialInstance;
        }

        void Draw(RenderContext& context,
                  const TransformMatrices& transforms,
                  const LightState& lights,
                  const Float3& eyePosition) const {
            if (mMaterialInstance.GetBaseMaterial()) {
                mMaterialInstance.Bind(transforms, lights, eyePosition);
                mModelHandle->Draw(context);
                mMaterialInstance.Unbind();
            }
        }

        void Draw(RenderContext& context) const {
            mModelHandle->Draw(context);
        }
    };
}  // namespace x