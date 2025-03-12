// Author: Jake Rieger
// Created: 3/2/2025.
//

#include "AssetGenerator.hpp"
#include <random>
#include <yaml-cpp/yaml.h>

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
    bool AssetGenerator::GenerateAsset(const Path& assetFile, AssetType type) {
        const auto id = GenerateId(type);

        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "asset";
        out << YAML::Value << YAML::BeginMap;
        out << YAML::Key << "id" << YAML::Value << id;
        out << YAML::Key << "source" << YAML::Value << assetFile.Filename();
        out << YAML::EndMap;

        const auto descriptorFile = Path(assetFile.Str() + ".xasset");
        const auto writeResult    = FileWriter::WriteAllText(descriptorFile, out.c_str());

        if (writeResult) {
            printf("Generated Asset:\n  ID: %llu\n  Source: %s\n\n", id, assetFile.Filename().c_str());
        } else {
            printf("Failed to generate Asset:\n  ID: %llu\n\n", id);
        }

        return writeResult;
    }
}  // namespace x