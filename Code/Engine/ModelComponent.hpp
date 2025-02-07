#pragma once

#include "Common/Types.hpp"
#include "Math.hpp"
#include "Model.hpp"
#include "Material.hpp"

namespace x {
    class ModelComponent {
        ModelHandle _modelHandle;
        PBRMaterialInstance _materialInstance;
        bool _castsShadows = true;

    public:
        ModelComponent() = default;

        ModelComponent& SetModelHandle(const shared_ptr<ModelData>& model) {
            _modelHandle.SetModelData(model);
            return *this;
        }

        ModelComponent& SetMaterialHandle(const shared_ptr<PBRMaterial>& material) {
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

        void DrawWithMaterial(const TransformMatrices& transforms,
                              const LightState& lights,
                              const Float3& eyePosition) const {
            _materialInstance.Bind(transforms, lights, eyePosition);
            _modelHandle.Draw();
            _materialInstance.Unbind();
        }

        void Draw() const {
            _modelHandle.Draw();
        }
    };
}