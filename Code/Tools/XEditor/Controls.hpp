// Author: Jake Rieger
// Created: 3/18/2025.
//

#pragma once

#include <functional>
#include <imgui.h>
#include "Common/Typedefs.hpp"
#include "XPak/AssetDescriptor.hpp"

#define X_DROP_TARGET_MESH "DropTarget_Mesh"
#define X_DROP_TARGET_MATERIAL "DropTarget_Material"
#define X_DROP_TARGET_SCRIPT "DropTarget_Script"
#define X_DROP_TARGET_AUDIO "DropTarget_Audio"
#define X_DROP_TARGET_TEXTURE "DropTarget_Texture"

namespace x::Gui {
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
                         ImTextureID btnIcon,
                         const char* payloadType,
                         const std::function<void(const AssetDescriptor&)>& callback,
                         ImGuiInputTextFlags flags = ImGuiInputTextFlags_ReadOnly);

    void CenteredText(const char* text, const ImVec2& containerPos, const ImVec2& containerSize);

    bool DragFloatNColored(
      const char* label, f32* v, int components, f32 vSpeed, f32 vMin, f32 vMax, const char* format, f32 power);

    bool BorderedButton(const char* label, const ImVec2& size);

    bool BorderedButtonWithIcon(const char* label,
                                ImTextureID textureId,
                                const ImVec2& size,
                                const ImVec2& uv0 = ImVec2(0, 0),
                                const ImVec2& uv1 = ImVec2(1, 1),
                                int frame_padding = -1);

    void SpacingY(const f32 space);

    void SpacingX(const f32 space);

    bool PrimaryButton(const char* label, const ImVec2& size);

    bool
    ToggleButtonGroup(const char* label, const ImVec2& buttonSize, int* selected, const vector<ImTextureID>& icons);
}  // namespace x::Gui
