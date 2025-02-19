// Author: Jake Rieger
// Created: 2/18/2025.
//

#pragma once

#include "Common/Types.hpp"

namespace x {
    struct TextureDescriptor {
        str name;
        str resource;
    };

    struct MaterialDescriptor {
        str name;
        str baseMaterial;
        vector<TextureDescriptor> textures;
    };

    class MaterialParser {
    public:
        static MaterialDescriptor Parse(const str& filename);
    };
}  // namespace x
