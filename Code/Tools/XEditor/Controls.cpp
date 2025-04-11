// Author: Jake Rieger
// Created: 3/18/2025.
//

#include <imgui.h>
#include <imgui_internal.h>
#include "Controls.hpp"

#include "Utilities.hpp"
#include "Common/FileDialogs.hpp"
#include "Common/Types.hpp"
#include "Engine/EngineCommon.hpp"

ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) {
    return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y);
}

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

    bool AssetDropTarget(const char* label,
                         char* buf,
                         size_t bufSize,
                         ImTextureID btnIcon,
                         const char* payloadType,
                         const std::function<void(const AssetDescriptor&)>& callback,
                         ImGuiInputTextFlags flags) {
        // Begin a named group to keep the input and button together
        ImGui::BeginGroup();

        // Calculate sizes
        float availWidth       = ImGui::GetContentRegionAvail().x;
        float lineHeight       = ImGui::GetTextLineHeight();
        float padding          = ImGui::GetStyle().FramePadding.y * 2.0f;
        float inputHeight      = lineHeight + padding;
        float buttonWidth      = inputHeight;
        const float inputWidth = availWidth - buttonWidth - ImGui::GetStyle().ItemInnerSpacing.x;

        // Use InputText directly with its own width
        ImGui::PushItemWidth(inputWidth);
        ImGui::InputText(label, buf, bufSize, flags);
        ImGui::PopItemWidth();

        // Check for drag-drop on the input field
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && ImGui::BeginDragDropTarget()) {
            const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadType);
            if (payload) { callback(*(AssetDescriptor*)payload->Data); }

            ImGui::EndDragDropTarget();
        }

        // Add button on the same line
        ImGui::SameLine();
        const auto btnId         = std::format("{}_button", label);
        const bool buttonPressed = ImGui::ImageButton(btnId.c_str(), btnIcon, ImVec2(inputHeight - 8, inputHeight - 8));

        // End the group
        ImGui::EndGroup();

        return buttonPressed;
    }

    void CenteredText(const char* text, const ImVec2& containerPos, const ImVec2& containerSize) {
        ImDrawList* drawList  = ImGui::GetWindowDrawList();
        const ImVec2 textSize = ImGui::CalcTextSize(text, nullptr, true);
        const auto textPos    = ImVec2(containerPos.x + (containerSize.x - textSize.x) * 0.5f,
                                    containerPos.y + (containerSize.y - textSize.y) * 0.5f);
        drawList->AddText(textPos, ImGui::GetColorU32(ImGuiCol_Text), text);
        ImGui::Dummy(containerSize);
    }

    bool DragFloatNColored(
      const char* label, f32* v, int components, f32 vSpeed, f32 vMin, f32 vMax, const char* format, f32 power) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;

        ImGuiContext& g = *GImGui;
        bool valueChanged {false};

        ImGui::BeginGroup();
        {
            ImGui::PushID(label);
            ImGui::PushMultiItemsWidths(components, ImGui::CalcItemWidth());

            const ImU32 R = ColorToU32(HexToImVec4("A8310D"));
            const ImU32 G = ColorToU32(HexToImVec4("71A324"));
            const ImU32 B = ColorToU32(HexToImVec4("387CD7"));

            for (int i = 0; i < components; ++i) {
                static const ImU32 colors[] = {R, G, B, 0xBBFFFFFF};

                ImGui::PushID(i);
                valueChanged |= ImGui::DragFloat("##v", &v[i], vSpeed, vMin, vMax, format, power);

                const ImVec2 min      = ImGui::GetItemRectMin();
                const ImVec2 max      = ImGui::GetItemRectMax();
                const f32 spacing     = g.Style.FrameRounding;
                const f32 halfSpacing = spacing * 0.5f;

                window->DrawList->AddLine({min.x + spacing, max.y - halfSpacing},
                                          {max.x - spacing, max.y - halfSpacing},
                                          colors[i],
                                          1);

                ImGui::SameLine(0, g.Style.ItemInnerSpacing.x);
                ImGui::PopID();
                ImGui::PopItemWidth();
            }

            ImGui::PopID();
            ImGui::TextUnformatted(label, ImGui::FindRenderedTextEnd(label));
        }
        ImGui::EndGroup();

        return valueChanged;
    }
}  // namespace x