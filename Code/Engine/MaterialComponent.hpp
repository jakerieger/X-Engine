#pragma once
#include "Material.hpp"

namespace x {
    class MaterialComponent {
    public:
        MaterialComponent() = default;

        void SetMaterial() {}

        // Binds the current material and its associated data to the pipeline
        void Bind() {}

        // Unbinds the material from the pipeline
        void Unbind() {}
    };
}