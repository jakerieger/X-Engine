// Author: Jake Rieger
// Created: 3/2/2025.
//

#include "ProjectDescriptor.hpp"
#include "Common/XML.hpp"

namespace x {
    bool ProjectDescriptor::FromFile(const Path& filename) {
        if (!filename.Exists()) return false;

        rapidxml::xml_document<> doc;
        if (!XML::ReadFile(filename, doc)) return false;

        const auto projectNode = doc.first_node("Project");
        if (!projectNode) return false;

        mName          = XML::GetAttrStr(projectNode->first_attribute("name"));
        mEngineVersion = XML::GetNodeF32(projectNode, "EngineVersion");

        const auto projectRoot = filename.Parent();
        const auto contentPath = projectRoot / XML::GetNodeStr(projectNode, "ContentDirectory");

        mContentDirectory = contentPath.Str();
        mStartupScene     = XML::GetNodeStr(projectNode, "StartupScene");
        mLoaded           = true;

        return true;
    }

    bool ProjectDescriptor::ToFile(const Path& filename) const {
        using namespace rapidxml;
        xml_document<> doc;

        xml_node<>* projectNode = doc.allocate_node(node_element, "Project");
        projectNode->append_attribute(doc.allocate_attribute("name", mName.c_str()));
        doc.append_node(projectNode);

        // Project node
        {
            xml_node<>* engVersionNode = doc.allocate_node(node_element, "EngineVersion", X_TOCSTR(mEngineVersion));
            projectNode->append_node(engVersionNode);

            xml_node<>* contentDirectoryNode =
              doc.allocate_node(node_element, "ContentDirectory", mContentDirectory.c_str());
            projectNode->append_node(contentDirectoryNode);

            xml_node<>* startupSceneNode = doc.allocate_node(node_element, "StartupScene", mStartupScene.c_str());
            projectNode->append_node(startupSceneNode);
        }

        return XML::WriteFile(filename, doc);
    }

    std::string ProjectDescriptor::ToString() const {
        return std::format("Name: {}\nEngine Version: {}\nContent: {}\n", mName, mEngineVersion, mContentDirectory);
    }
}  // namespace x