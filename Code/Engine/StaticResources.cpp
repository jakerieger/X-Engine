#include "StaticResources.hpp"

namespace x::StaticResources {
    shared_ptr<PBRMaterial> Material_PBR;

    bool Init(RenderContext& context) {
        Material_PBR = PBRMaterial::Create(context);

        if (!Material_PBR) { return false; }

        return true;
    }
}  // namespace x::StaticResources