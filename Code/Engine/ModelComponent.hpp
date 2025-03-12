#pragma once

#include "Common/Types.hpp"
#include "Math.hpp"
#include "Model.hpp"
#include "Material.hpp"
#include "ResourceManager.hpp"

namespace x {
    class ModelComponent {
        ResourceHandle<Model> mModelHandle;
        shared_ptr<IMaterial> mMaterial;
        bool mCastsShadows   = true;
        bool mReceiveShadows = true;

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

        void Draw(RenderContext& context,
                  const TransformMatrices& transforms,
                  const LightState& lights,
                  const Float3& eyePosition) const {
            if (mMaterial) { mMaterial->Bind(transforms, lights, eyePosition); }
            mModelHandle->Draw(context);
            if (mMaterial) { mMaterial->Unbind(); }
        }

        void Draw(RenderContext& context) const {
            mModelHandle->Draw(context);
        }
    };
}  // namespace x