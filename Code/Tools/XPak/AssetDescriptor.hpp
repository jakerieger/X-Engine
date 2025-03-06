// Author: Jake Rieger
// Created: 3/2/2025.
//

#pragma once

#include <yaml-cpp/yaml.h>
#include "Common/Types.hpp"

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

        bool FromFile(const str& filename) {
            YAML::Node root       = YAML::LoadFile(filename);
            const auto& assetNode = root["asset"];
            if (!assetNode.IsDefined()) { return false; }

            mId = assetNode["id"].as<u64>();

            mFilename = assetNode["source"].as<str>();
            if (mFilename.empty()) { return false; }

            return true;
        }

        AssetType GetTypeFromId() const {
            return GetTypeFromId(mId);
        }

        str GetTypeString() const {
            return GetTypeString(GetTypeFromId());
        }

        AssetId GetBaseId() const {
            return mId & kAssetIdBitmask;
        }

        // Returns just the ID bits from the asset ID, omitting the type bits
        static AssetId GetBaseId(AssetId id) {
            return id & kAssetIdBitmask;
        }

        static AssetType GetTypeFromId(AssetId id) {
            const AssetType type = CAST<u8>(id >> 56);
            return type;
        }

        static str GetTypeString(AssetType type) {
            switch (type) {
                case kAssetType_Texture:
                    return "texture";
                case kAssetType_Mesh:
                    return "mesh";
                case kAssetType_Audio:
                    return "audio";
                case kAssetType_Material:
                    return "material";
                case kAssetType_Scene:
                    return "scene";
                case kAssetType_Script:
                    return "script";
                default:
                    return "invalid";
            }
        }
    };
}  // namespace x
