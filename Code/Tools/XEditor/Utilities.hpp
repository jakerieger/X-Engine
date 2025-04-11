// Author: Jake Rieger
// Created: 4/11/2025.
//

#pragma once

#include <imgui.h>
#include "Engine/D3D.hpp"
#include "Common/Types.hpp"

namespace x {
    static ImTextureID SrvAsTextureId(ID3D11ShaderResourceView* srv) {
        return RCAST<ImTextureID>(RCAST<void*>(srv));
    }

    static ImVec4 HexToImVec4(const str& hex, const f32 alpha = 1.0f) {
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

    static ImVec4 ColorWithOpacity(const ImVec4 color, const f32 alpha = 1.0f) {
        return ImVec4(color.x, color.y, color.z, alpha);
    }

    static ImU32 ColorToU32(const ImVec4 color) {
        const u32 r         = (u32)(color.x * 255);
        const u32 g         = (u32)(color.y * 255);
        const u32 b         = (u32)(color.z * 255);
        constexpr u32 a     = 255;
        const u32 colorUint = (a << 24) | (b << 16) | (g << 8) | r;
        return colorUint;
    }
}  // namespace x