#pragma once
#include "Material.hpp"

namespace x {
    class MaterialComponent {
        PBRMaterialInstance _materialInstance;

    public:
        MaterialComponent() = default;

        void SetMaterial(const shared_ptr<PBRMaterial>& material) {
            _materialInstance.SetBaseMaterial(material);
        }

        // Binds the current material and its associated data to the pipeline
        void Bind() {}

        // Unbinds the material from the pipeline
        void Unbind() {}
    };
}