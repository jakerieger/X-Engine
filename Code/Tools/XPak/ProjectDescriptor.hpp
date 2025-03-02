// Author: Jake Rieger
// Created: 3/2/2025.
//

#pragma once

#include <format>

#include "Common/Types.hpp"
#include "Common/Filesystem.hpp"
#include <yaml-cpp/yaml.h>

namespace x {
    struct ProjectDescriptor {
        str mName;
        f32 mEngineVersion;
        str mContentDirectory;
        str mScriptsDirectory;
        str mMaterialsDirectory;
        str mScenesDirectory;

        bool FromFile(const str& filename) {
            YAML::Node root         = YAML::LoadFile(filename);
            const auto& projectNode = root["project"];
            if (!projectNode.IsDefined()) { return false; }

            mName          = projectNode["name"].as<str>();
            mEngineVersion = projectNode["engineVersion"].as<f32>();

            const auto& assetDirsNode = projectNode["assetDirectories"];
            if (!assetDirsNode.IsDefined()) { return false; }

            const auto projectDir = Filesystem::Path(filename).Parent();

            // Get directories relative to the project directory
            // .xproj file should sit at root of file structure
            mContentDirectory   = projectDir.Join(assetDirsNode["content"].as<str>()).Str();
            mScriptsDirectory   = projectDir.Join(assetDirsNode["scripts"].as<str>()).Str();
            mMaterialsDirectory = projectDir.Join(assetDirsNode["materials"].as<str>()).Str();
            mScenesDirectory    = projectDir.Join(assetDirsNode["scenes"].as<str>()).Str();

            return true;
        }

        std::string ToString() const {
            return std::format("Name: {}\nEngine Version: {}\nContent: {}\nScripts: {}\nMaterials: {}\nScenes: {}\n",
                               mName,
                               mEngineVersion,
                               mContentDirectory,
                               mScriptsDirectory,
                               mMaterialsDirectory,
                               mScenesDirectory);
        }
    };
}  // namespace x
