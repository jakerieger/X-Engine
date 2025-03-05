// Author: Jake Rieger
// Created: 2/18/2025.
//

#pragma once

#include <span>

#include "Common/Types.hpp"

namespace x {
    struct TextureDescriptor {
        str name;
        u64 assetId;
    };

    struct MaterialDescriptor {
        str name;
        str baseMaterial;
        vector<TextureDescriptor> textures;
    };

    class MaterialParser {
    public:
        static MaterialDescriptor Parse(const str& filename);
        static MaterialDescriptor Parse(std::span<const u8> data);
    };
}  // namespace x
