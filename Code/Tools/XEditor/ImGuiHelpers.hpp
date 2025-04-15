// Author: Jake Rieger
// Created: 4/13/2025.
//

#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <utility>
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
    class ScopedStyleVars {
        using StyleVarPair = std::pair<ImGuiStyleVar_, int>;

    public:
        explicit ScopedStyleVars(const vector<StyleVarPair>& vars) : mVars(vars) {
            for (const auto& var : mVars) {
                ImGui::PushStyleVar(var.first, var.second);
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