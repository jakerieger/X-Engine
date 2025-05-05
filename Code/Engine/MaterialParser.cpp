// Author: Jake Rieger
// Created: 2/18/2025.
//

#include "MaterialParser.hpp"
#include "Common/XML.hpp"

namespace x {
    static bool ParseDoc(const rapidxml::xml_document<>& doc, MaterialDescriptor& descriptor) {
        const auto materialNode = doc.first_node("Material");
        if (materialNode) {
            const char* name = materialNode->first_attribute("name")->value();
            X_ASSERT(X_CSTR_EMPTY(name))
            descriptor.mName = name;

            const char* baseMaterial = materialNode->first_attribute("base")->value();
            X_ASSERT(X_CSTR_EMPTY(baseMaterial))
            descriptor.mBaseMaterial = baseMaterial;

            const auto transparentAttr = materialNode->first_attribute("transparent");
            const bool transparent     = XML::GetAttrBool(transparentAttr);
            descriptor.mTransparent    = transparent;

            // TODO: Process properties here when I get around to implementing them
            // const auto propsNode = materialNode->first_node("Properties");

            const auto texturesNode = materialNode->first_node("Textures");
            if (texturesNode) {
                for (const rapidxml::xml_node<>* texture = texturesNode->first_node("Texture"); texture;
                     texture                             = texture->next_sibling()) {
                    const char* texName = texture->first_attribute("name")->value();
                    X_ASSERT(X_CSTR_EMPTY(texName))

                    const auto* assetAttr = texture->first_attribute("asset");
                    const u64 texId       = std::stoull(assetAttr->value());
                    X_ASSERT(texId != 0)

                    TextureDescriptor texDesc;
                    texDesc.mName    = texName;
                    texDesc.mAssetId = texId;
                    descriptor.mTextures.push_back(texDesc);
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