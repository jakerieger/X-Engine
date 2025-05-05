// Author: Jake Rieger
// Created: 3/2/2025.
//

#include "AssetDescriptor.hpp"

namespace x {
    bool AssetDescriptor::FromFile(const Path& filename) {
        rapidxml::xml_document<> doc;
        if (!XML::ReadFile(filename, doc)) { return false; }

        const rapidxml::xml_node<>* assetNode = doc.first_node("Asset");
        const u64 id                          = XML::GetAttrId(assetNode);
        X_ASSERT(id != 0)

        const auto* sourceNode = assetNode->first_node("Source");
        if (!sourceNode) return false;
        const char* sourceValue = sourceNode->value();
        X_ASSERT(!(X_CSTR_EMPTY(sourceValue)))
        const auto sourcePath = Path(sourceValue);
        X_ASSERT(sourcePath.Exists())

        mId       = id;
        mFilename = sourceValue;

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