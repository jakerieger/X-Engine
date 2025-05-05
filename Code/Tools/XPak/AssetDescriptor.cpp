// Author: Jake Rieger
// Created: 3/2/2025.
//

#include "AssetDescriptor.hpp"

namespace x {
    bool AssetDescriptor::FromFile(const Path& filename) {
        rapidxml::xml_document<> doc;
        if (!XML::ReadFile(filename, doc)) { return false; }

        const auto* assetNode = doc.first_node("Asset");
        mId                   = XML::GetAttrId(assetNode);
        mFilename             = XML::GetNodeStr(assetNode, "Source");

        return true;
    }

    AssetType AssetDescriptor::GetTypeFromId() const {
        return GetTypeFromId(mId);
    }

    str AssetDescriptor::GetTypeString() const {
        return GetTypeString(GetTypeFromId());
    }

    AssetId AssetDescriptor::GetBaseId() const {
        return mId & kAssetIdBitmask;
    }

    AssetId AssetDescriptor::GetBaseId(AssetId id) {
        return id & kAssetIdBitmask;
    }

    AssetType AssetDescriptor::GetTypeFromId(AssetId id) {
        const AssetType type = CAST<u8>(id >> 56);
        return type;
    }

    str AssetDescriptor::GetTypeString(AssetType type) {
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
}  // namespace x