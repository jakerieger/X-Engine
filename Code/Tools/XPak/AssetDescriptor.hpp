// Author: Jake Rieger
// Created: 3/2/2025.
//

#pragma once

#include <yaml-cpp/yaml.h>
#include "Common/Types.hpp"

namespace x {
    static constexpr u16 kAssetType_None     = 0;
    static constexpr u16 kAssetType_Texture  = 1;
    static constexpr u16 kAssetType_Mesh     = 2;
    static constexpr u16 kAssetType_Audio    = 3;
    static constexpr u16 kAssetType_Material = 4;
    static constexpr u16 kAssetType_Scene    = 5;
    static constexpr u16 kAssetType_Invalid  = std::numeric_limits<u16>::max();

    struct AssetDescriptor {
        u64 mId;
        u16 mType;
        str mFilename;

        bool FromFile(const str& filename) {
            YAML::Node root       = YAML::LoadFile(filename);
            const auto& assetNode = root["asset"];
            if (!assetNode.IsDefined()) { return false; }

            mId                   = assetNode["id"].as<u64>();
            const auto typeString = assetNode["type"].as<str>();

            if (typeString == "texture") {
                mType = kAssetType_Texture;
            } else if (typeString == "mesh") {
                mType = kAssetType_Mesh;
            } else if (typeString == "audio") {
                mType = kAssetType_Audio;
            } else if (typeString == "material") {
                mType = kAssetType_Material;
            } else if (typeString == "scene") {
                mType = kAssetType_Scene;
            } else {
                mType = kAssetType_Invalid;
            }

            mFilename = assetNode["filename"].as<str>();
            if (mFilename.empty()) { return false; }

            return true;
        }

        static str TypeToString(u16 type) {
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
                default:
                    return "invalid";
            }
        }
    };
}  // namespace x
