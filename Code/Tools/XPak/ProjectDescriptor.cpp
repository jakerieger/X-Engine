// Author: Jake Rieger
// Created: 3/2/2025.
//

#include "ProjectDescriptor.hpp"

namespace x {
    bool ProjectDescriptor::FromFile(const str& filename) {
        YAML::Node root         = YAML::LoadFile(filename);
        const auto& projectNode = root["project"];
        if (!projectNode.IsDefined()) { return false; }

        mName          = projectNode["name"].as<str>();
        mEngineVersion = projectNode["engineVersion"].as<f32>();

        const auto projectDir = Path(filename).Parent();

        // Get directories relative to the project directory
        // .xproj file should sit at root of file structure
        mContentDirectory = projectDir.Join(projectNode["contentDirectory"].as<str>()).Str();

        mLoaded = true;
        return true;
    }

    std::string ProjectDescriptor::ToString() const {
        return std::format("Name: {}\nEngine Version: {}\nContent: {}\n", mName, mEngineVersion, mContentDirectory);
    }
}  // namespace x