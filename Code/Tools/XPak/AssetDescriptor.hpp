// Author: Jake Rieger
// Created: 3/2/2025.
//

#pragma once

#include "Common/Types.hpp"
#include "Common/Filesystem.hpp"
#include "Common/XML.hpp"

namespace x {
    using AssetType                                = u8;
    using AssetId                                  = u64;
    static constexpr AssetId kAssetIdBitmask       = 0x00FFFFFFFFFFFFFF;
    static constexpr AssetType kAssetType_Invalid  = 0;
    static constexpr AssetType kAssetType_Texture  = 1;
    static constexpr AssetType kAssetType_Mesh     = 2;
    static constexpr AssetType kAssetType_Audio    = 3;
    static constexpr AssetType kAssetType_Material = 4;
    static constexpr AssetType kAssetType_Scene    = 5;
    static constexpr AssetType kAssetType_Script   = 6;

    struct AssetDescriptor {
        u64 mId;
        str mFilename;

        bool FromFile(const Path& filename);

        AssetType GetTypeFromId() const;
        str GetTypeString() const;
        AssetId GetBaseId() const;

        // Returns just the ID bits from the asset ID, omitting the type bits
        static AssetId GetBaseId(AssetId id);
        static AssetType GetTypeFromId(AssetId id);
        static str GetTypeString(AssetType type);
    };
}  // namespace x
