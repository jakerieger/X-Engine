#pragma once

#include <memory>
#include "Material.hpp"

namespace x::StaticResources {
    extern shared_ptr<PBRMaterial> Material_PBR;

    bool Init(RenderContext& context);
}  // namespace x::StaticResources