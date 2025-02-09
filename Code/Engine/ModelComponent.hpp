#pragma once

#include "Common/Types.hpp"
#include "Math.hpp"
#include "Model.hpp"
#include "Material.hpp"
#include "ResourceManager.hpp"

namespace x {
    class ModelComponent {
        ResourceHandle<Model> _modelHandle;
        PBRMaterialInstance _materialInstance;
        bool _castsShadows = true;

    public:
        ModelComponent() = default;

        ModelComponent& SetModelHandle(const ResourceHandle<Model>& model) {
            _modelHandle = model;
            return *this;
        }

        ModelComponent& SetMaterial(const shared_ptr<PBRMaterial>& material) {
            _materialInstance.SetBaseMaterial(material);
            return *this;
        }

        ModelComponent& SetCastsShadows(const bool casts) {
            _castsShadows = casts;
            return *this;
        }

        PBRMaterialInstance& GetMaterialInstance() {
            return _materialInstance;
        }

        void Draw(RenderContext& context,
                  const TransformMatrices& transforms,
                  const LightState& lights,
                  const Float3& eyePosition) const {
            if (_materialInstance.GetBaseMaterial()) {
                _materialInstance.Bind(transforms, lights, eyePosition);
                _modelHandle->Draw(context);
                _materialInstance.Unbind();
            }
        }

        void Draw(RenderContext& context) const {
            _modelHandle->Draw(context);
        }
    };
}