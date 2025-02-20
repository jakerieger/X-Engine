// Author: Jake Rieger
// Created: 2/18/2025.
//

#include "EditorWindow.hpp"

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <imgui_internal.h>
#include <Inter.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace x::Editor {
    void EditorWindow::OnInitialize() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        io.Fonts->AddFontFromMemoryCompressedTTF(Inter_compressed_data, Inter_compressed_size, 16.0f);

        ImGui_ImplWin32_Init(_hwnd);
        ImGui_ImplDX11_Init(_context.GetDevice(), _context.GetDeviceContext());

        ImGui::StyleColorsDark();
    }

    void EditorWindow::OnResize(u32 width, u32 height) {}

    void EditorWindow::OnShutdown() {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    void EditorWindow::Update() {}

    void EditorWindow::Render() {
        _windowViewport->AttachViewport();
        _windowViewport->ClearAll();

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        static bool firstTime = true;

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New Scene", "Ctrl+N")) {}
                if (ImGui::MenuItem("Open Scene", "Ctrl+O")) {}
                if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {}
                ImGui::Separator();
                if (ImGui::MenuItem("Exit", "Alt+F4")) { Quit(); }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit")) {
                if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
                if (ImGui::MenuItem("Redo", "Ctrl+Y")) {}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                if (ImGui::MenuItem("Reset Layout")) {
                    // Trigger first-time layout setup again
                    firstTime = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        auto* imguiViewport      = ImGui::GetWindowViewport();
        ImGuiDockNodeFlags flags = ImGuiDockNodeFlags_PassthruCentralNode;
        ImGui::DockSpaceOverViewport(imguiViewport->ID, imguiViewport, flags);

        if (firstTime) {
            firstTime = false;
            ImGui::DockBuilderRemoveNode(imguiViewport->ID);
            ImGui::DockBuilderAddNode(imguiViewport->ID, flags | ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(imguiViewport->ID, imguiViewport->Size);

            ImGuiID dockMainId   = imguiViewport->ID;
            ImGuiID dockRightId  = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Right, 0.2f, None, &dockMainId);
            ImGuiID dockLeftId   = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Left, 0.2f, None, &dockMainId);
            ImGuiID dockBottomId = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Down, 0.2f, None, &dockMainId);

            ImGui::DockBuilderDockWindow("Entities", dockLeftId);
            ImGui::DockBuilderDockWindow("Properties", dockRightId);
            ImGui::DockBuilderDockWindow("Scene", dockMainId);
            ImGui::DockBuilderDockWindow("Editor Log", dockBottomId);

            ImGui::DockBuilderFinish(imguiViewport->ID);
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Scene");
        {
            ImVec2 contentSize = ImGui::GetContentRegionAvail();
            _sceneViewport.Resize((u32)contentSize.x, (u32)contentSize.y);
            _sceneViewport.BindRenderTarget();
            _sceneViewport.ClearRenderTargetView(Colors::CornflowerBlue);
            auto* srv = _sceneViewport.GetShaderResourceView().Get();
            ImGui::Image(ImTextureID((void*)srv), contentSize);
        }
        ImGui::End();
        ImGui::PopStyleVar();

        ImGui::Begin("Entities");
        {}
        ImGui::End();

        ImGui::Begin("Properties");
        {}
        ImGui::End();

        ImGui::Begin("Editor Log");
        {}
        ImGui::End();

        _windowViewport->BindRenderTarget();
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    LRESULT EditorWindow::MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) {
        if (ImGui_ImplWin32_WndProcHandler(_hwnd, msg, wParam, lParam)) return true;
        return Window::MessageHandler(msg, wParam, lParam);
    }
}  // namespace x::Editor
