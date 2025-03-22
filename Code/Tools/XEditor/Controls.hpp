// Author: Jake Rieger
// Created: 3/18/2025.
//

#pragma once

#include <imgui.h>
#include "Common/Types.hpp"

#define X_DROP_TARGET_MESH "DropTarget_Mesh"
#define X_DROP_TARGET_MATERIAL "DropTarget_Material"
#define X_DROP_TARGET_SCRIPT "DropTarget_Script"
#define X_DROP_TARGET_AUDIO "DropTarget_Audio"
#define X_DROP_TARGET_TEXTURE "DropTarget_Texture"

namespace x {
    bool SelectableWithHeaders(const char* id,
                               const char* header,
                               const char* subheader,
                               bool selected,
                               ImGuiSelectableFlags flags = 0,
                               ImVec2 size                = ImVec2(300, 36),
                               float textPadding          = 10.0f);

    bool AssetDropTarget(const char* label,
                         char* buf,
                         size_t bufSize,
                         const char* btnLabel,
                         const char* payloadType,
                         ImGuiInputTextFlags flags = ImGuiInputTextFlags_ReadOnly);

    void CenteredText(const char* text, const ImVec2& containerPos, const ImVec2& containerSize);
}  // namespace x
