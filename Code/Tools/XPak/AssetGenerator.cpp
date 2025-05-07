// Author: Jake Rieger
// Created: 3/2/2025.
//

#include <random>
#include "AssetGenerator.hpp"
#include "Common/XML.hpp"
#include <rapidxml_print.hpp>  // For formatting output

namespace x {
    u64 AssetGenerator::GenerateBaseId() {
        static std::random_device rd;
        static std::mt19937_64 gen(rd());

        const AssetId id = gen() & kAssetIdBitmask;
        return id;
    }

    AssetId AssetGenerator::GenerateId(AssetType type) {
        const u64 baseId = GenerateBaseId();
        const u64 typeId = CAST<u64>(type) << 56;
        const AssetId id = typeId | baseId;
        return id;
    }

    // TODO: Add flags to the asset file, for example streamable or compressed
    bool AssetGenerator::GenerateAsset(const Path& assetFile, AssetType type, const Path& outputDir) {
        using namespace rapidxml;
        xml_document<> doc;

        // Create root asset node
        const auto id         = GenerateId(type);
        xml_node<>* assetNode = doc.allocate_node(node_element, "Asset");
        const auto idStr      = X_TOSTR(id);
        assetNode->append_attribute(doc.allocate_attribute("id", idStr.c_str()));
        doc.append_node(assetNode);

        // Add source node
        const auto sourcePath  = assetFile.RelativeTo(outputDir).Str();
        xml_node<>* sourceNode = doc.allocate_node(node_element, "Source", sourcePath.c_str());
        assetNode->append_node(sourceNode);

        const auto descriptorFile = outputDir / (assetFile.Filename() + ".xasset");

        return XML::WriteFile(descriptorFile, doc);
    }
}  // namespace x