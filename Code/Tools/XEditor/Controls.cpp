// Author: Jake Rieger
// Created: 3/18/2025.
//

#include <imgui.h>
#include <imgui_internal.h>
#include "Controls.hpp"

#include "../../Engine/Color.hpp"
#include "ImGuiHelpers.hpp"
#include "Common/FileDialogs.hpp"
#include "Common/Typedefs.hpp"
#include "Engine/EngineCommon.hpp"

namespace x::Gui {
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
        window->DrawList->AddText(ImVec2(header_x, header_y), ImGui::GetColorU32(ImGuiCol_Text), header);

        // Use a slightly different color for the subheader (slightly muted)
        window->DrawList->AddText(ImVec2(subheader_x, subheader_y), ImU32(0xFF666666), subheader);

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
        const bool buttonPressed = ImGui::ImageButton(btnId.c_str(),
                                                      btnIcon,
                                                      ImVec2(inputHeight - 8, inputHeight - 8),
                                                      kUV_0,
                                                      kUV_1,
                                                      Colors::Transparent.ToImVec4(),
                                                      Colors::White75.ToImVec4());

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
        const ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;

        const ImGuiContext& g = *GImGui;
        bool valueChanged {false};

        ImGui::BeginGroup();
        {
            ImGui::PushID(label);
            ImGui::PushMultiItemsWidths(components, ImGui::CalcItemWidth());

            const ImU32 R = Color("#EB3751").ToU32_ABGR();
            const ImU32 G = Color("#83CB10").ToU32_ABGR();
            const ImU32 B = Color("#2F85E6").ToU32_ABGR();

            for (int i = 0; i < components; ++i) {
                static const ImU32 colors[] = {R, G, B, 0xBBFFFFFF};

                ImGui::PushID(i);
                valueChanged |= ImGui::DragFloat("##v", &v[i], vSpeed, vMin, vMax, format, 0);

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

    bool BorderedButton(const char* label, const ImVec2& size) {
        // Define colors
        const ImVec4 normalColorBorder  = Color("#353535").ToImVec4();
        const ImVec4 hoveredColorBorder = Color("#24B7DE").ToImVec4();
        const ImVec4 activeColorBorder  = Color("#24B7DE").ToImVec4();

        ImGui::PushStyleColor(ImGuiCol_Border, normalColorBorder);

        const bool result = ImGui::Button(label, size);

        const bool hovered = ImGui::IsItemHovered();
        const bool active  = ImGui::IsItemActive();

        const ImVec2 min = ImGui::GetItemRectMin();
        const ImVec2 max = ImGui::GetItemRectMax();

        ImGui::PopStyleColor();

        const ImU32 borderColor = ImGui::ColorConvertFloat4ToU32(active    ? activeColorBorder
                                                                 : hovered ? hoveredColorBorder
                                                                           : normalColorBorder);

        ImGui::GetWindowDrawList()->AddRect(min,
                                            max,
                                            borderColor,
                                            ImGui::GetStyle().FrameRounding,
                                            0,  // All corners
                                            ImGui::GetStyle().FrameBorderSize);

        return result;
    }

    bool BorderedButtonWithIcon(const char* label,
                                ImTextureID textureId,
                                const ImVec2& size,
                                const ImVec2& uv0,
                                const ImVec2& uv1,
                                int frame_padding) {
        ImGuiContext& g     = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        if (window->SkipItems) return false;

        // Calculate total button size (icon + padding + text)
        const ImGuiStyle& style = g.Style;

        // Get label size
        ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

        // Total height = button height + spacing + text height
        ImVec2 total_size = ImVec2(size.x, size.y + style.ItemSpacing.y + label_size.y);

        const ImGuiID id = window->GetID(label);
        const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + total_size);

        ImGui::ItemSize(total_size, style.FramePadding.y);
        if (!ImGui::ItemAdd(bb, id)) return false;

        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, 0);

        // Render button (just the icon part)
        ImRect button_bb(window->DC.CursorPos, window->DC.CursorPos + size);

        // Render
        const ImU32 col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_ButtonActive
                                             : hovered         ? ImGuiCol_ButtonHovered
                                                               : ImGuiCol_Button);
        ImGui::RenderNavHighlight(bb, id);
        ImGui::RenderFrame(button_bb.Min, button_bb.Max, col, true, style.FrameRounding);

        // Image is centered in the button
        ImVec2 image_pos = button_bb.Min + (button_bb.Max - button_bb.Min - size) * 0.5f;
        window->DrawList->AddImage(textureId, image_pos, image_pos + size, uv0, uv1, ImGui::GetColorU32(ImGuiCol_Text));

        // Text is centered under the button
        ImVec2 text_pos =
          ImVec2(button_bb.Min.x + (size.x - label_size.x) * 0.5f, button_bb.Max.y + style.ItemSpacing.y);
        window->DrawList->AddText(text_pos, ImGui::GetColorU32(ImGuiCol_Text), label);

        return pressed;
    }

    void SpacingY(const f32 space) {
        ImGui::Dummy(ImVec2(0, space));
    }

    void SpacingX(const f32 space) {
        ImGui::Dummy(ImVec2(space, 0));
    }

    bool PrimaryButton(const char* label, const ImVec2& size) {
        const Color buttonColor("#1a97b8");
        Gui::ScopedColorVars colors({{ImGuiCol_Button, buttonColor.ToImVec4()},
                                     {ImGuiCol_ButtonActive, buttonColor.WithAlpha(0.67f).ToImVec4()},
                                     {ImGuiCol_ButtonHovered, buttonColor.WithAlpha(0.8f).ToImVec4()},
                                     {ImGuiCol_Text, Color(1.0f, 1.0f).ToImVec4()}});
        const bool result = ImGui::Button(label, size);
        return result;
    }

    bool
    ToggleButtonGroup(const char* label, const ImVec2& buttonSize, int* selected, const vector<ImTextureID>& icons) {
        const int previouslySelected = *selected;

        const auto* colors = ImGui::GetStyle().Colors;
        const Color borderNormal(colors[ImGuiCol_Separator]);
        const Color borderHovered(colors[ImGuiCol_SeparatorActive]);
        const Color borderSelected(borderHovered);

        auto ToggleButton = [&](const int index, const ImTextureID icon) {
            const bool isSelected = (*selected == index);

            if (isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Button, colors[ImGuiCol_ButtonActive]);
                ImGui::PushStyleColor(ImGuiCol_Border, borderSelected.ToImVec4());
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, colors[ImGuiCol_Button]);
                ImGui::PushStyleColor(ImGuiCol_Border, borderNormal.ToImVec4());
            }

            ImGui::PushStyleVar(ImGuiCol_ChildBg, 1.0f);

            // Create the image button
            char btnId[16];
            sprintf(btnId, "##btn%d", index);

            const ImVec4 iconTint = isSelected ? Colors::White.ToImVec4() : colors[ImGuiCol_CheckMark];
            if (ImGui::ImageButton(btnId, icon, buttonSize, kUV_0, kUV_1, Colors::Transparent.ToImVec4(), iconTint)) {
                *selected = index;
            }

            ImGui::PopStyleVar();
            ImGui::PopStyleColor(2);

            if (isSelected) {
                ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(),
                                                    ImGui::GetItemRectMax(),
                                                    borderHovered.ToU32_ABGR(),
                                                    ImGui::GetStyle().FrameRounding,
                                                    0,
                                                    1.0f);
            }
        };

        ImGui::PushID(label);
        ImGui::BeginGroup();
        for (int i = 0; i < icons.size(); i++) {
            ToggleButton(i, icons[i]);

            // Add spacing between buttons except after the last one
            if (i < icons.size() - 1) {
                ImGui::SameLine(0, 4.0f);  // 4 pixels spacing between buttons
            }
        }
        ImGui::EndGroup();
        ImGui::PopID();

        return previouslySelected != *selected;
    }
}  // namespace x::Gui