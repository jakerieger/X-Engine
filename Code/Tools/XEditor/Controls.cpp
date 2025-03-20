// Author: Jake Rieger
// Created: 3/18/2025.
//

#include <imgui.h>
#include <imgui_internal.h>
#include "Controls.hpp"
#include "Common/Types.hpp"

namespace x {
    bool SelectableWithHeaders(const char* id,
                               const char* header,
                               const char* subheader,
                               bool selected,
                               ImGuiSelectableFlags flags,
                               ImVec2 size,
                               float textPadding) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;

        ImGuiContext& g         = *GImGui;
        const ImGuiStyle& style = g.Style;

        // Calculate sizes and positions
        const ImVec2 label_size_header    = ImGui::CalcTextSize(header, NULL, true);
        const ImVec2 label_size_subheader = ImGui::CalcTextSize(subheader, NULL, true);

        // Use the provided size or calculate based on content
        ImVec2 actual_size = size;
        if (size.y <= 0) {
            // Determine how much vertical space we need
            float height = label_size_header.y + label_size_subheader.y + style.ItemSpacing.y;
            // Add padding
            height += style.FramePadding.y * 2;
            actual_size.y = height;
        }

        const ImGuiID item_id = window->GetID(id);
        const ImVec2 pos      = window->DC.CursorPos;

        // If width is auto, use available content width
        if (actual_size.x <= 0) actual_size.x = ImGui::GetContentRegionAvail().x;

        const ImRect bb(pos, ImVec2(pos.x + actual_size.x, pos.y + actual_size.y));
        ImGui::ItemSize(actual_size, style.FramePadding.y);
        if (!ImGui::ItemAdd(bb, item_id)) return false;

        // Detect clicked
        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(bb, item_id, &hovered, &held, flags);

        // Render
        const ImU32 col = ImGui::GetColorU32(selected ? ImGuiCol_FrameBgActive
                                                      : (hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg));
        ImGui::RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);

        // Left align with padding
        const float header_x    = bb.Min.x + textPadding;
        const float subheader_x = bb.Min.x + textPadding;

        // Calculate vertical center position
        float total_text_height = label_size_header.y + style.ItemSpacing.y + label_size_subheader.y;
        float start_y           = bb.Min.y + (bb.Max.y - bb.Min.y - total_text_height) * 0.5f;

        // Position the header at the calculated Y position
        const float header_y = start_y;

        // Position the subheader below the header
        const float subheader_y = header_y + label_size_header.y + style.ItemSpacing.y;

        // Render the header and subheader
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_Text));
        window->DrawList->AddText(ImVec2(header_x, header_y), ImGui::GetColorU32(ImGuiCol_Text), header);

        // Use a slightly different color for the subheader (slightly muted)
        window->DrawList->AddText(ImVec2(subheader_x, subheader_y), ImU32(0xFF999999), subheader);
        ImGui::PopStyleColor();

        return pressed;
    }

    void CenteredText(const char* text, const ImVec2& containerPos, const ImVec2& containerSize) {
        ImDrawList* drawList  = ImGui::GetWindowDrawList();
        const ImVec2 textSize = ImGui::CalcTextSize(text, nullptr, true);
        const auto textPos    = ImVec2(containerPos.x + (containerSize.x - textSize.x) * 0.5f,
                                    containerPos.y + (containerSize.y - textSize.y) * 0.5f);
        drawList->AddText(textPos, ImGui::GetColorU32(ImGuiCol_Text), text);
        ImGui::Dummy(containerSize);
    }
}  // namespace x