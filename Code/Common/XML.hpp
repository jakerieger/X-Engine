// Author: Jake Rieger
// Created: 5/4/2025.
//

#pragma once

#include <span>
#include <rapidxml.hpp>
#include "Typedefs.hpp"
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
            xml = FileReader::ReadText(filename);
            if (xml.empty()) return false;

            const auto buffer = new char[xml.size() + 1];
            std::memcpy(buffer, xml.c_str(), xml.size() + 1);
            doc.parse<rapidxml::parse_non_destructive | rapidxml::parse_normalize_whitespace |
                      rapidxml::parse_validate_closing_tags>(buffer);

            return true;
        } catch (const rapidxml::parse_error& e) {
            X_LOG_ERROR("XML Parse Error: %s", e.what());
            return false;
        } catch (const std::exception& e) {
            X_LOG_ERROR("Exception: %s", e.what());
            return false;
        } catch (...) {
            X_LOG_ERROR("Unknown error occurred");
            return false;
        }
    }

    /// @brief Parse XML from byte array. Data MUST be null-terminated or parsing will fail.
    inline bool ReadBytes(char* data, size_t dataSize, rapidxml::xml_document<>& doc) {
        if (dataSize <= 0) return false;

        try {
            doc.parse<rapidxml::parse_non_destructive | rapidxml::parse_normalize_whitespace |
                      rapidxml::parse_validate_closing_tags>(data);

            return true;
        } catch (const rapidxml::parse_error& e) {
            X_LOG_ERROR("XML Parse Error: %s", e.what());
            return false;
        } catch (const std::exception& e) {
            X_LOG_ERROR("Exception: %s", e.what());
            return false;
        } catch (...) {
            X_LOG_ERROR("Unknown error occurred");
            return false;
        }
    }

    inline bool WriteFile(const Path& filename, const rapidxml::xml_document<>& doc) {
        try {
            str xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
            rapidxml::print(std::back_inserter(xml), doc);
            FileWriter::WriteText(filename, xml);
            return true;
        } catch (...) { return false; }
    }

    template<typename T>
        requires std::is_arithmetic_v<T>
    inline rapidxml::xml_node<>* MakeNumericNode(const char* name,
                                                 const T& value,
                                                 rapidxml::xml_document<>& doc,
                                                 const rapidxml::node_type type = rapidxml::node_element) {
        const str valueStr = X_TOSTR(value);
        rapidxml::xml_node<>* node =
          doc.allocate_node(type, doc.allocate_string(name), doc.allocate_string(valueStr.c_str()));
        return node;
    }

    template<typename T>
        requires std::is_arithmetic_v<T>
    inline rapidxml::xml_attribute<>* MakeNumericAttr(const char* name, const T& value, rapidxml::xml_document<>& doc) {
        const str valueStr = X_TOSTR(value);
        rapidxml::xml_attribute<>* attr =
          doc.allocate_attribute(doc.allocate_string(name), doc.allocate_string(valueStr.c_str()));
        return attr;
    }

    inline rapidxml::xml_node<>* MakeFloat3Node(const char* name, const Float3& value, rapidxml::xml_document<>& doc) {
        rapidxml::xml_node<>* node = doc.allocate_node(rapidxml::node_element, name);
        node->append_attribute(MakeNumericAttr("x", value.x, doc));
        node->append_attribute(MakeNumericAttr("y", value.y, doc));
        node->append_attribute(MakeNumericAttr("z", value.z, doc));
        return node;
    }

    inline rapidxml::xml_node<>* MakeColorNode(const char* name, const Color& value, rapidxml::xml_document<>& doc) {
        rapidxml::xml_node<>* node = doc.allocate_node(rapidxml::node_element, name);
        node->append_attribute(MakeNumericAttr("r", value.R(), doc));
        node->append_attribute(MakeNumericAttr("g", value.G(), doc));
        node->append_attribute(MakeNumericAttr("b", value.B(), doc));
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
