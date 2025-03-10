// Author: Jake Rieger
// Created: 2/18/2025.
//

#pragma once

#include <span>

#include "Common/Types.hpp"

namespace x {
    struct TextureDescriptor {
        str mName;
        u64 mAssetId;
    };

    struct MaterialDescriptor {
        str mName;
        str mBaseMaterial;
        bool mTransparent {false};
        vector<TextureDescriptor> mTextures;
    };

    class MaterialParser {
    public:
        static MaterialDescriptor Parse(const str& filename);
        static MaterialDescriptor Parse(std::span<const u8> data);
    };
}  // namespace x
