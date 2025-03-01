#include "StaticResources.hpp"

namespace x {
    void StaticResources::Initialize(RenderContext& context) {
        MaterialPBR = std::make_shared<PBRMaterial>(context);
    }
}  // namespace x