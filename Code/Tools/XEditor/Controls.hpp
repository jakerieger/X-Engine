// Author: Jake Rieger
// Created: 3/18/2025.
//

#pragma once

#include <imgui.h>
#include "Common/Types.hpp"

namespace x {
    bool SelectableWithHeaders(const char* id,
                               const char* header,
                               const char* subheader,
                               bool selected,
                               ImGuiSelectableFlags flags = 0,
                               ImVec2 size                = ImVec2(300, 36),
                               float textPadding          = 10.0f);

    void AssetDropTarget(const char* id);

    void CenteredText(const char* text, const ImVec2& containerPos, const ImVec2& containerSize);
}  // namespace x
