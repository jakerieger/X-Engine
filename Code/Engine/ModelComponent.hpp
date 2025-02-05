#pragma once

#include "Common/Types.hpp"
#include "Math.hpp"
#include "Model.hpp"
#include "Material.hpp"

namespace x {
    class ModelComponent {
        ModelHandle _modelHandle;
        shared_ptr<PBRMaterial> _materialHandle;
        bool _castsShadows = true;

    public:
        ModelComponent() = default;

        ModelComponent& SetModelHandle(const shared_ptr<ModelData>& model) {
            _modelHandle.SetModelData(model);
            return *this;
        }

        ModelComponent& SetMaterialHandle(const shared_ptr<PBRMaterial>& material) {
            _materialHandle = material;
            return *this;
        }

        ModelComponent& SetCastsShadows(const bool casts) {
            _castsShadows = casts;
            return *this;
        }

        void UpdateMaterialParams(const TransformMatrices& transforms,
                                  const LightState& lights,
                                  const Float3& eyePosition) {
            if (_materialHandle) {
                _materialHandle->UpdateBuffers(transforms, lights, eyePosition);
            }
        }

        void Draw(const bool applyMaterial = true) const {
            if (applyMaterial && _materialHandle) {
                _materialHandle->Apply();
            }

            _modelHandle.Draw();

            if (applyMaterial && _materialHandle) { _materialHandle->Clear(); }
        }
    };
}