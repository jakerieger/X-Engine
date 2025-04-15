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

        mStartupScene = projectNode["startupScene"].as<str>();

        mLoaded = true;
        return true;
    }

    bool ProjectDescriptor::ToFile(const Path& filename) const {
        YAML::Emitter out;

        out << YAML::BeginMap;
        out << YAML::Key << "project" << YAML::Value << YAML::BeginMap;
        {
            out << YAML::Key << "name" << YAML::Value << mName;
            out << YAML::Key << "engineVersion" << YAML::Value << mEngineVersion;
            out << YAML::Key << "contentDirectory" << YAML::Value << mContentDirectory;
            out << YAML::Key << "startupScene" << YAML::Value << mStartupScene;
        }
        out << YAML::EndMap;
        out << YAML::EndMap;

        return FileWriter::WriteAllText(filename, out.c_str());
    }

    std::string ProjectDescriptor::ToString() const {
        return std::format("Name: {}\nEngine Version: {}\nContent: {}\n", mName, mEngineVersion, mContentDirectory);
    }
}  // namespace x