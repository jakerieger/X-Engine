// Author: Jake Rieger
// Created: 3/2/2025.
//

#pragma once

#include "Common/Typedefs.hpp"
#include "AssetDescriptor.hpp"
#include "Common/Filesystem.hpp"

// TODO: Implement collision avoidance

namespace x {
    class AssetGenerator {
    public:
        static u64 GenerateBaseId();
        static AssetId GenerateId(AssetType type);
        static bool GenerateAsset(const Path& assetFile, AssetType type, const Path& outputDir);
    };
}  // namespace x
