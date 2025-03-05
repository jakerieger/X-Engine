// Author: Jake Rieger
// Created: 2/18/2025.
//

#include "MaterialParser.hpp"
#include <yaml-cpp/yaml.h>

namespace x {
    static MaterialDescriptor ParseFromNode(const YAML::Node& node) {
        MaterialDescriptor material;
        material.name         = node["name"].as<str>();
        material.baseMaterial = node["baseMaterial"].as<str>();
        if (const auto& textures = node["textures"]; textures.IsDefined() && textures.size() > 0) {
            for (const auto& texture : textures) {
                TextureDescriptor textureDescriptor;
                textureDescriptor.name = texture["name"].as<str>();

                // yaml-cpp was having trouble parsing some IDs as uint64's
                // Turns out this was because I was forgetting to add a newline to the yaml file
                // I'm gonna keep this anyways as its more error-tolerant
                auto idStr       = texture["asset"].as<str>();
                const auto idVal = std::stoull(idStr, nullptr, 10);

                textureDescriptor.assetId = idVal;
                material.textures.push_back(textureDescriptor);
            }
        }

        return material;
    }

    MaterialDescriptor MaterialParser::Parse(const str& filename) {
        const YAML::Node root = YAML::LoadFile(filename);
        return ParseFromNode(root);
    }

    MaterialDescriptor MaterialParser::Parse(std::span<const u8> data) {
        const auto content    = RCAST<const char*>(data.data());
        const YAML::Node root = YAML::Load(content);
        return ParseFromNode(root);
    }
}  // namespace x