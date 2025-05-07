// Author: Jake Rieger
// Created: 2/18/2025.
//

#include "MaterialParser.hpp"
#include "Common/XML.hpp"

namespace x {
    static bool ParseDoc(const rapidxml::xml_document<>& doc, MaterialDescriptor& descriptor) {
        const auto materialNode = doc.first_node("Material");
        if (materialNode) {
            descriptor.mName         = XML::GetAttrStr(materialNode->first_attribute("name"));
            descriptor.mBaseMaterial = XML::GetAttrStr(materialNode->first_attribute("base"));

            const auto transparentAttr = materialNode->first_attribute("transparent");
            const bool transparent     = XML::GetAttrBool(transparentAttr);
            descriptor.mTransparent    = transparent;

            // TODO: Process properties here when I get around to implementing them
            // const auto propsNode = materialNode->first_node("Properties");

            const auto texturesNode = materialNode->first_node("Textures");
            if (texturesNode) {
                for (const rapidxml::xml_node<>* texture = texturesNode->first_node("Texture"); texture;
                     texture                             = texture->next_sibling()) {
                    descriptor.mTextures.emplace_back(XML::GetAttrStr(texture->first_attribute("name")),
                                                      std::stoull(texture->first_attribute("asset")->value()));
                }
            }

            return true;
        }
        return false;
    }

    bool MaterialParser::Parse(const Path& filename, MaterialDescriptor& descriptor) {
        if (!filename.Exists()) return false;
        rapidxml::xml_document<> doc;
        if (XML::ReadFile(filename, doc)) { return ParseDoc(doc, descriptor); }
        return false;
    }

    bool MaterialParser::Parse(std::span<const u8> data, MaterialDescriptor& descriptor) {
        if (data.empty()) return false;
        rapidxml::xml_document<> doc;
        vector<u8> buffer(data.begin(), data.end());
        buffer.push_back('\0');
        try {
            doc.parse<0>(RCAST<char*>(&buffer[0]));
            return ParseDoc(doc, descriptor);
        } catch (...) { return false; }
    }
}  // namespace x