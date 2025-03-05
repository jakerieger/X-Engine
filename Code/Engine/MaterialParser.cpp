// Author: Jake Rieger
// Created: 2/18/2025.
//

#include "MaterialParser.hpp"
#include <yaml-cpp/yaml.h>

namespace x {
    MaterialDescriptor MaterialParser::Parse(const str& filename) {
        YAML::Node root = YAML::LoadFile(filename);
        MaterialDescriptor material;

        material.name         = root["name"].as<str>();
        material.baseMaterial = root["baseMaterial"].as<str>();
        if (const auto& textures = root["textures"]; textures.IsDefined() && textures.size() > 0) {
            for (const auto& texture : textures) {
                TextureDescriptor textureDescriptor;
                textureDescriptor.name    = texture["name"].as<str>();
                textureDescriptor.assetId = texture["asset"].as<u64>();
                material.textures.push_back(textureDescriptor);
            }
        }

        return material;
    }
}  // namespace x