// Author: Jake Rieger
// Created: 5/4/2025.
//

#pragma once

#include <span>
#include <rapidxml.hpp>
#include "Types.hpp"
#include "Engine/Color.hpp"
#include "Engine/EngineCommon.hpp"
#include "Engine/Math.hpp"

namespace rapidxml {
    namespace internal {
        // Forward declarations for internal functions
        template<class OutIt, class Ch>
        OutIt print_children(OutIt out, const xml_node<Ch>* node, int flags, int indent);

        template<class OutIt, class Ch>
        OutIt print_element_node(OutIt out, const xml_node<Ch>* node, int flags, int indent);

        template<class OutIt, class Ch>
        OutIt print_data_node(OutIt out, const xml_node<Ch>* node, int flags, int indent);

        template<class OutIt, class Ch>
        OutIt print_cdata_node(OutIt out, const xml_node<Ch>* node, int flags, int indent);

        template<class OutIt, class Ch>
        OutIt print_declaration_node(OutIt out, const xml_node<Ch>* node, int flags, int indent);

        template<class OutIt, class Ch>
        OutIt print_comment_node(OutIt out, const xml_node<Ch>* node, int flags, int indent);

        template<class OutIt, class Ch>
        OutIt print_doctype_node(OutIt out, const xml_node<Ch>* node, int flags, int indent);

        template<class OutIt, class Ch>
        OutIt print_pi_node(OutIt out, const xml_node<Ch>* node, int flags, int indent);
    }  // namespace internal
}  // namespace rapidxml

#include <rapidxml_print.hpp>

namespace x::XML {
    inline bool ReadFile(const Path& filename, rapidxml::xml_document<>& doc) {
        if (!filename.Exists()) return false;
        str xml;
        try {
            xml = FileReader::ReadAllText(filename);
            if (xml.empty()) return false;

            const auto buffer = new char[xml.size() + 1];
            std::memcpy(buffer, xml.c_str(), xml.size() + 1);
            doc.parse<rapidxml::parse_non_destructive | rapidxml::parse_normalize_whitespace |
                      rapidxml::parse_validate_closing_tags>(buffer);

            return true;
        } catch (const rapidxml::parse_error& e) {
            X_LOG_ERROR("%s", e.what());
            return false;
        } catch (const std::exception& e) {
            X_LOG_ERROR("%s", e.what());
            return false;
        } catch (...) {
            X_LOG_ERROR("Unknown error occurred");
            return false;
        }
    }

    inline bool ReadBytes(std::span<const u8> data, rapidxml::xml_document<>& doc) {
        if (data.empty()) return false;
        vector<u8> bytes(data.begin(), data.end());
        try {
            doc.parse<0>(RCAST<char*>(&bytes[0]));
            return true;
        } catch (const rapidxml::parse_error& e) {
            X_LOG_ERROR("%s", e.what());
            return false;
        } catch (const std::exception& e) {
            X_LOG_ERROR("%s", e.what());
            return false;
        } catch (...) {
            X_LOG_ERROR("Unknown error occurred");
            return false;
        }
    }

    inline bool WriteFile(const Path& filename, const rapidxml::xml_document<>& doc) {
        try {
            std::ofstream file(filename.CStr());
            if (!file.is_open()) return false;
            file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
            file << doc;
            file.close();
            return true;
        } catch (...) { return false; }
    }

    inline rapidxml::xml_node<>* MakeFloat3Node(const char* name, const Float3& value, rapidxml::xml_document<>& doc) {
        rapidxml::xml_node<>* node = doc.allocate_node(rapidxml::node_element, name);
        node->append_attribute(doc.allocate_attribute("x", X_TOCSTR(value.x)));
        node->append_attribute(doc.allocate_attribute("y", X_TOCSTR(value.y)));
        node->append_attribute(doc.allocate_attribute("z", X_TOCSTR(value.z)));
        return node;
    }

    inline rapidxml::xml_node<>* MakeColorNode(const char* name, const Color& value, rapidxml::xml_document<>& doc) {
        rapidxml::xml_node<>* node = doc.allocate_node(rapidxml::node_element, name);
        node->append_attribute(doc.allocate_attribute("r", X_TOCSTR(value.R())));
        node->append_attribute(doc.allocate_attribute("g", X_TOCSTR(value.G())));
        node->append_attribute(doc.allocate_attribute("b", X_TOCSTR(value.B())));
        return node;
    }

#pragma region Attribute Getters
    inline str GetAttrStr(const rapidxml::xml_attribute<>* attr) {
        return str(attr->value(), attr->value_size());
    }

    inline f32 GetAttrFloat(const rapidxml::xml_attribute<>* attr) {
        return attr ? std::stof(attr->value()) : 0.0f;
    }

    inline bool GetAttrBool(const rapidxml::xml_attribute<>* attr) {
        return attr ? X_STRCMP(attr->value(), "true") : false;
    }

    inline Color GetAttrColor(const rapidxml::xml_node<>* node) {
        if (node) {
            const f32 r = GetAttrFloat(node->first_attribute("r"));
            const f32 g = GetAttrFloat(node->first_attribute("g"));
            const f32 b = GetAttrFloat(node->first_attribute("b"));
            return {r, g, b};
        }
        return {};
    }

    inline Float3 GetAttrFloat3(const rapidxml::xml_node<>* node) {
        if (node) {
            const f32 x = GetAttrFloat(node->first_attribute("x"));
            const f32 y = GetAttrFloat(node->first_attribute("y"));
            const f32 z = GetAttrFloat(node->first_attribute("z"));
            return {x, y, z};
        }
        return {};
    }

    inline u64 GetAttrId(const rapidxml::xml_node<>* node) {
        if (node) {
            const auto value = node->first_attribute("id");
            if (value) {
                const u64 id = std::stoull(value->value());
                return id;
            }
        }
        return 0;
    }
#pragma endregion

#pragma region Node Value Getters
    inline str GetNodeStr(const rapidxml::xml_node<>* node) {
        if (node) { return str(node->value(), node->value_size()); }
        return {};
    }

    inline str GetNodeStr(const rapidxml::xml_node<>* parent, const char* childName) {
        const rapidxml::xml_node<>* child = parent->first_node(childName);
        return GetNodeStr(child);
    }

    inline bool GetNodeBool(const rapidxml::xml_node<>* parent, const char* childName) {
        const rapidxml::xml_node<>* child = parent->first_node(childName);
        if (!child || !child->value()) return false;
        return X_STRCMP(GetNodeStr(child).c_str(), "true");
    }

    inline f32 GetNodeF32(const rapidxml::xml_node<>* parent, const char* childName) {
        const rapidxml::xml_node<>* child = parent->first_node(childName);
        if (!child || !child->value()) return std::numeric_limits<f32>::quiet_NaN();
        return std::stof(child->value());
    }

    inline f64 GetNodeF64(const rapidxml::xml_node<>* parent, const char* childName) {
        const rapidxml::xml_node<>* child = parent->first_node(childName);
        if (!child || !child->value()) return std::numeric_limits<f64>::quiet_NaN();
        return std::stod(child->value());
    }
#pragma endregion
}  // namespace x::XML
