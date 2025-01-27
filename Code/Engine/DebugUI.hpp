#pragma once

#include "Vendor/imgui/imgui.h"
#include "Vendor/imgui/backends/imgui_impl_win32.h"
#include "Vendor/imgui/backends/imgui_impl_dx11.h"
#include "Renderer.hpp"
#include <JetBrainsMono.h>

namespace x {
    class DebugUI {
        Renderer& _renderer;
        bool _showFrameGraph = false;
        bool _showDeviceInfo = false;
        bool _showRenderInfo = false;
        ImFont* _font        = None;

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

        void SetShowFramegraph(const bool show) {
            _showFrameGraph = show;
        }

        void SetShowDeviceInfo(const bool show) {
            _showDeviceInfo = show;
        }

        void SetShowRenderInfo(const bool show) {
            _showRenderInfo = show;
        }

        /// Make sure to call this between BeginFrame() and EndFrame()
        void Draw() {
            if (_showFrameGraph) { DrawFrameGraph(); }
            if (_showDeviceInfo) { DrawDeviceInfo(); }
            if (_showRenderInfo) { DrawRenderInfo(); }
        }

    private:
        void Initialize(const HWND hwnd) {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

            _font = io.Fonts->AddFontFromMemoryCompressedTTF(JetBrainsMono_TTF_compressed_data,
                                                             JetBrainsMono_TTF_compressed_size,
                                                             16.0f);

            ImGui::StyleColorsDark();
            ImGui_ImplWin32_Init(hwnd);
            ImGui_ImplDX11_Init(_renderer.GetDevice(), _renderer.GetContext());
        }

        void Shutdown() {
            ImGui_ImplDX11_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
        }

        void DrawFrameGraph() {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(ImVec2(0, viewport->Size.y - 300));
            ImGui::SetNextWindowSize(ImVec2(400, 300));
            if (!ImGui::Begin("##framegraph",
                              None,
                              ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                              ImGuiWindowFlags_NoTitleBar)) {
                ImGui::End();
                return;
            }

            ImGui::Text("Frame Graph");

            ImGui::End();
        }

        void DrawDeviceInfo() {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(ImVec2(viewport->Size.x - 300, 0));
            ImGui::SetNextWindowSize(ImVec2(300, 200));
            if (!ImGui::Begin("##deviceinfo",
                              None,
                              ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                              ImGuiWindowFlags_NoTitleBar)) {
                ImGui::End();
                return;
            }

            ImGui::Text("Device Info");
            ImGui::Text("GPU Vendor: NVidia");
            ImGui::Text("GPU Model: RTX 4070 Ti");
            ImGui::Text("GPU Memory: 12.00 GB");

            ImGui::End();
        }

        void DrawRenderInfo() {
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2(300, 300));
            if (!ImGui::Begin("##renderinfo",
                              None,
                              ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                              ImGuiWindowFlags_NoTitleBar)) {
                ImGui::End();
                return;
            }

            ImGui::Text("Render Info");
            ImGui::Text("Drawcalls: 1");

            ImGui::End();
        }
    };
}