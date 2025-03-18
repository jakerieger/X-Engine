// Author: Jake Rieger
// Created: 3/18/2025.
//

#pragma once

#include <imgui.h>

namespace x {
    bool SelectableWithHeaders(const char* id,
                               const char* header,
                               const char* subheader,
                               bool selected,
                               ImGuiSelectableFlags flags = 0,
                               ImVec2 size                = ImVec2(300, 36),
                               float textPadding          = 10.0f);
}  // namespace x
