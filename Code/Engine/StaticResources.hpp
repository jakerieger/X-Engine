#pragma once

#include <memory>
#include "Material.hpp"
#include "PBRMaterial.hpp"

namespace x {
    class StaticResources {
        friend class Game;
        X_CLASS_PREVENT_MOVES_COPIES(StaticResources)

    public:
        static std::shared_ptr<PBRMaterial> MaterialPBR;

    private:
        static void Initialize(RenderContext& context);
    };
}  // namespace x