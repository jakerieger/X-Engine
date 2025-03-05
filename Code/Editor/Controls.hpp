// Author: Jake Rieger
// Created: 2/21/2025.
//

#pragma once

#include "Common/Types.hpp"
#include "EditorWindow.hpp"
#include <imgui.h>

namespace x::Editor {
#pragma region Titlebar
    struct WindowState {
        bool isDragging  = false;
        POINT dragStart  = {0, 0};
        ImVec2 windowPos = ImVec2(0, 0);
    };

    static WindowState gWindowState;
    inline constexpr f32 kTitlebarHeight = 32.0f;
    inline constexpr f32 kToolbarHeight  = 36.0f;

    inline void Titlebar(HWND hwnd) {
        constexpr ImVec4 titlebarColor = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, kTitlebarHeight));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, titlebarColor);

        ImGui::Begin("##titlebar",
                     nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                       ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        {
            if (ImGui::IsMouseHoveringRect(ImGui::GetWindowPos(),
                                           ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x - 100,
                                                  ImGui::GetWindowPos().y + kTitlebarHeight))) {
                if (ImGui::IsMouseClicked(0)) {
                    gWindowState.isDragging = true;
                    GetCursorPos(&gWindowState.dragStart);
                    RECT windowRect;
                    GetWindowRect(hwnd, &windowRect);
                    gWindowState.windowPos = ImVec2(CAST<f32>(windowRect.left), CAST<f32>(windowRect.top));
                }
            }

            if (gWindowState.isDragging && ImGui::IsMouseDown(0)) {
                POINT currentPos;
                GetCursorPos(&currentPos);
                int deltaX = currentPos.x - gWindowState.dragStart.x;
                int deltaY = currentPos.y - gWindowState.dragStart.y;

                SetWindowPos(hwnd,
                             nullptr,
                             CAST<int>(gWindowState.windowPos.x) + deltaX,
                             CAST<int>(gWindowState.windowPos.y) + deltaY,
                             0,
                             0,
                             SWP_NOSIZE | SWP_NOZORDER);
            } else {
                gWindowState.isDragging = false;
            }

            // Window title
            ImGui::SetCursorPos(ImVec2(10, (kTitlebarHeight - ImGui::GetFontSize()) * 0.5f));
            ImGui::Text("Custom Titlebar Demo");

            // Window controls
            float buttonSize  = kTitlebarHeight - 8;
            float rightOffset = ImGui::GetWindowSize().x;

            // Close button
            rightOffset -= buttonSize + 4;
            ImGui::SetCursorPos(ImVec2(rightOffset, 4));
            if (ImGui::Button("X", ImVec2(buttonSize, buttonSize))) { PostQuitMessage(0); }

            // Maximize button
            rightOffset -= buttonSize + 4;
            ImGui::SetCursorPos(ImVec2(rightOffset, 4));
            if (ImGui::Button("□", ImVec2(buttonSize, buttonSize))) {
                WINDOWPLACEMENT placement = {sizeof(placement)};
                GetWindowPlacement(hwnd, &placement);
                if (placement.showCmd == SW_MAXIMIZE) ShowWindow(hwnd, SW_RESTORE);
                else ShowWindow(hwnd, SW_MAXIMIZE);
            }

            // Minimize button
            rightOffset -= buttonSize + 4;
            ImGui::SetCursorPos(ImVec2(rightOffset, 4));
            if (ImGui::Button("_", ImVec2(buttonSize, buttonSize))) { ShowWindow(hwnd, SW_MINIMIZE); }
        }
        ImGui::End();

        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    }
#pragma endregion

    inline bool IconButtonToggle(
      ImVec2 size, ID3D11ShaderResourceView* icon, const str& name, bool active, ImVec4 tint = ImVec4(1, 1, 1, 0.5)) {
        const auto* colors = ImGui::GetStyle().Colors;

        ImGui::PushStyleColor(ImGuiCol_Border, active ? colors[ImGuiCol_DockingPreview] : colors[ImGuiCol_WindowBg]);
        ImGui::PushStyleColor(ImGuiCol_Button,
                              active ? ImVec4(colors[ImGuiCol_DockingPreview].x,
                                              colors[ImGuiCol_DockingPreview].y,
                                              colors[ImGuiCol_DockingPreview].z,
                                              0.2f)
                                     : colors[ImGuiCol_WindowBg]);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, colors[ImGuiCol_Button]);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colors[ImGuiCol_Button]);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

        const bool pressed = ImGui::ImageButton(name.c_str(),
                                                (ImTextureID)icon,
                                                size,
                                                ImVec2(0, 0),
                                                ImVec2(1, 1),
                                                ImVec4(0, 0, 0, 0),
                                                tint);

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(4);

        return pressed;
    }

    inline bool
    IconButton(ImVec2 size, ID3D11ShaderResourceView* icon, const str& name, ImVec4 tint = ImVec4(1, 1, 1, 0.5)) {
        const auto* colors = ImGui::GetStyle().Colors;
        ImGui::PushStyleColor(ImGuiCol_Button, colors[ImGuiCol_WindowBg]);
        const bool pressed = ImGui::ImageButton(name.c_str(),
                                                (ImTextureID)icon,
                                                size,
                                                ImVec2(0, 0),
                                                ImVec2(1, 1),
                                                ImVec4(0, 0, 0, 0),
                                                tint);
        ImGui::PopStyleColor();
        return pressed;
    }

    inline void SeparatorVertical(ImVec2 size,
                                  ID3D11ShaderResourceView* icon,
                                  const str& name,
                                  ImVec4 tint = ImVec4(1, 1, 1, 0.25)) {
        ImGui::Image((ImTextureID)icon, size, ImVec2(0, 0), ImVec2(1, 1), tint);
    }

    inline void Toolbar(EditorWindow* parent, TextureManager& textureManager, f32 menuBarHeight) {
        static int selectedTool = 0;
        static bool snapToGrid  = false;

        constexpr ImGuiWindowFlags toolbarFlags = ImGuiWindowFlags_NoTitleBar |            // No title bar needed
                                                  ImGuiWindowFlags_NoScrollbar |           // Disable scrolling
                                                  ImGuiWindowFlags_NoMove |                // Prevent moving
                                                  ImGuiWindowFlags_NoResize |              // Prevent resizing
                                                  ImGuiWindowFlags_NoCollapse |            // Prevent collapsing
                                                  ImGuiWindowFlags_NoSavedSettings |       // Don't save position/size
                                                  ImGuiWindowFlags_NoBringToFrontOnFocus;  // Don't change z-order

        ImGui::SetNextWindowPos(ImVec2(0, menuBarHeight));
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetMainViewport()->Size.x, kToolbarHeight));

        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2, 2));

        if (ImGui::Begin("##Toolbar", nullptr, toolbarFlags)) {
            const auto separatorIcon = textureManager.GetTexture("SeparatorIcon")->srv.Get();

            const auto selectIcon = textureManager.GetTexture("SelectIcon")->srv.Get();
            const auto moveIcon   = textureManager.GetTexture("MoveIcon")->srv.Get();
            const auto rotateIcon = textureManager.GetTexture("RotateIcon")->srv.Get();
            const auto scaleIcon  = textureManager.GetTexture("ScaleIcon")->srv.Get();

            const auto playIcon  = textureManager.GetTexture("PlayIcon")->srv.Get();
            const auto pauseIcon = textureManager.GetTexture("PauseIcon")->srv.Get();
            const auto stopIcon  = textureManager.GetTexture("StopIcon")->srv.Get();

            const auto undoIcon       = textureManager.GetTexture("UndoIcon")->srv.Get();
            const auto redoIcon       = textureManager.GetTexture("RedoIcon")->srv.Get();
            const auto snapToGridIcon = textureManager.GetTexture("SnapToGridIcon")->srv.Get();
            const auto settingsIcon   = textureManager.GetTexture("SettingsIcon")->srv.Get();

            static constexpr auto btnSize = ImVec2(24, 24);

            if (IconButtonToggle(btnSize, selectIcon, "##select", selectedTool == 0)) { selectedTool = 0; }
            ImGui::SameLine();
            if (IconButtonToggle(btnSize, moveIcon, "##move", selectedTool == 1)) { selectedTool = 1; }
            ImGui::SameLine();
            if (IconButtonToggle(btnSize, rotateIcon, "##rotate", selectedTool == 2)) { selectedTool = 2; }
            ImGui::SameLine();
            if (IconButtonToggle(btnSize, scaleIcon, "##scale", selectedTool == 3)) { selectedTool = 3; }

            ImGui::SameLine();
            SeparatorVertical(ImVec2(28, 28), separatorIcon, "##sep1");
            ImGui::SameLine();

            if (IconButton(btnSize, undoIcon, "##undo")) {}
            ImGui::SameLine();
            if (IconButton(btnSize, redoIcon, "##redo")) {}
            ImGui::SameLine();
            if (IconButtonToggle(btnSize, snapToGridIcon, "##snap_to_grid", snapToGrid)) { snapToGrid = !snapToGrid; }

            // Move the next 3 buttons to the window center
            // Total width is (24*3) + (2*3) or (72) + (6) or 78

            auto windowWidth        = ImGui::GetWindowWidth();
            auto middleButtonsWidth = (btnSize.x * 3) + 40;  // Calculated the '+ 40' using photoshop, no clue how it's
                                                             // derived, but I'll be damned if it ain't centered
            auto xOffset = (windowWidth / 2) - (middleButtonsWidth / 2);

            ImGui::SameLine(xOffset);

            if (IconButton(btnSize, playIcon, "##play")) { parent->TogglePlayMode(); }  // 24
            ImGui::SameLine();                                                          // 4
            if (IconButton(btnSize, pauseIcon, "##pause")) {}                           // 24
            ImGui::SameLine();                                                          // 4
            if (IconButton(btnSize, stopIcon, "##stop")) { parent->TogglePlayMode(); }  // 24

            // Move the last settings button to the right
            auto xOffsetFromRight = windowWidth - 36;

            ImGui::SameLine(xOffsetFromRight);
            if (IconButton(btnSize, settingsIcon, "Settings")) {}

            ImGui::End();
        }

        ImGui::PopStyleVar(2);
    }
}  // namespace x::Editor
