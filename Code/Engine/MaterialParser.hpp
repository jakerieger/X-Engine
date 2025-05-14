// Author: Jake Rieger
// Created: 2/18/2025.
//

#pragma once

#include <span>
#include "Common/Typedefs.hpp"
#include "Common/Filesystem.hpp"

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
        static bool Parse(const Path& filename, MaterialDescriptor& descriptor);
        static bool Parse(std::span<const u8> data, MaterialDescriptor& descriptor);
    };
}  // namespace x
