// Author: Jake Rieger
// Created: 4/13/2025.
//

#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <utility>
#include "Engine/D3D.hpp"
#include "Common/Types.hpp"

inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) {
    return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y);
}

inline ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs) {
    return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y);
}

inline ImVec2 operator*(const ImVec2& lhs, const ImVec2& rhs) {
    return ImVec2(lhs.x * rhs.x, lhs.y * rhs.y);
}

inline ImVec2 operator/(const ImVec2& lhs, const ImVec2& rhs) {
    return ImVec2(lhs.x / rhs.x, lhs.y / rhs.y);
}

inline ImVec2 operator+(const ImVec2& lhs, const float scalar) {
    return ImVec2(lhs.x + scalar, lhs.y + scalar);
}

inline ImVec2 operator-(const ImVec2& lhs, const float scalar) {
    return ImVec2(lhs.x - scalar, lhs.y - scalar);
}

inline ImVec2 operator*(const ImVec2& lhs, const float scalar) {
    return ImVec2(lhs.x * scalar, lhs.y * scalar);
}

inline ImVec2 operator/(const ImVec2& lhs, const float scalar) {
    return ImVec2(lhs.x / scalar, lhs.y / scalar);
}

namespace x::Gui {
    inline ImVec4 StrToColor(const str& hex, const f32 alpha = 1.0f) {
        // Ensure the string is the correct length
        if (hex.length() != 6) { throw std::invalid_argument("Hex color should be in the format 'RRGGBB'"); }

        ImVec4 color;
        const char red[3]   = {hex[0], hex[1], '\0'};
        const char green[3] = {hex[2], hex[3], '\0'};
        const char blue[3]  = {hex[4], hex[5], '\0'};

        const i32 r = strtol(red, nullptr, 16);
        const i32 g = strtol(green, nullptr, 16);
        const i32 b = strtol(blue, nullptr, 16);

        color.x = (f32)r / 255.0f;
        color.y = (f32)g / 255.0f;
        color.z = (f32)b / 255.0f;
        color.w = alpha;

        return color;
    }

    inline ImVec4 ColorWithOpacity(const ImVec4 color, const f32 alpha = 1.0f) {
        return ImVec4(color.x, color.y, color.z, alpha);
    }

    inline ImU32 ColorToU32(const ImVec4 color) {
        const u32 r         = (u32)(color.x * 255);
        const u32 g         = (u32)(color.y * 255);
        const u32 b         = (u32)(color.z * 255);
        constexpr u32 a     = 255;
        const u32 colorUint = (a << 24) | (b << 16) | (g << 8) | r;
        return colorUint;
    }

    class ScopedStyleVars {
        using StyleVarPair = std::pair<ImGuiStyleVar_, int>;

    public:
        explicit ScopedStyleVars(const vector<StyleVarPair>& vars) : mVars(vars) {
            for (const auto& var : mVars) {
                ImGui::PushStyleVar(var.first, (f32)var.second);
            }
        };

        ~ScopedStyleVars() {
            ImGui::PopStyleVar(mVars.size());
        }

    private:
        vector<StyleVarPair> mVars;
    };

    class ScopedColorVars {
        using ColorVarPair = std::pair<ImGuiCol_, ImVec4>;

    public:
        explicit ScopedColorVars(const vector<ColorVarPair>& vars) : mVars(vars) {
            for (const auto& var : mVars) {
                ImGui::PushStyleColor(var.first, var.second);
            }
        };

        ~ScopedColorVars() {
            ImGui::PopStyleColor(mVars.size());
        }

    private:
        vector<ColorVarPair> mVars;
    };

    class ScopedFont {
    public:
        explicit ScopedFont(ImFont* font) {
            ImGui::PushFont(font);
        }

        ~ScopedFont() {
            ImGui::PopFont();
        }
    };
}  // namespace x::Gui