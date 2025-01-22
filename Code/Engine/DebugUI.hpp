#pragma once

#include "Vendor/imgui/imgui.h"
#include "Vendor/imgui/backends/imgui_impl_win32.h"
#include "Vendor/imgui/backends/imgui_impl_dx11.h"
#include "Renderer.hpp"

namespace x {
    class DebugUI {
        Renderer& _renderer;

    public:
        explicit DebugUI(const HWND hwnd, Renderer& renderer) : _renderer(renderer) {
            Initialize(hwnd);
        }

        ~DebugUI() {
            Shutdown();
        }

        void BeginFrame() {
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();
        }

        void EndFrame() {
            ImGui::Render();
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        }

    private:
        void Initialize(const HWND hwnd) {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            ImGui::StyleColorsDark();
            ImGui_ImplWin32_Init(hwnd);
            ImGui_ImplDX11_Init(_renderer.GetDevice(), _renderer.GetContext());
        }

        void Shutdown() {
            ImGui_ImplDX11_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
        }
    };
}