#pragma once

#include <format>

#include "Vendor/imgui/imgui.h"
#include "Vendor/imgui/backends/imgui_impl_win32.h"
#include "Vendor/imgui/backends/imgui_impl_dx11.h"
#include "RenderContext.hpp"
#include <JetBrainsMono.h>

namespace x {
    class DebugUI {
        RenderContext& mRenderer;
        bool mShowFrameGraph = false;
        bool mShowDeviceInfo = false;
        bool mShowFrameInfo  = true;
        ImFont* mFont        = None;

    public:
        explicit DebugUI(RenderContext& renderer) : mRenderer(renderer) {
            Initialize();
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

        void SetShowFrameGraph(const bool show) {
            mShowFrameGraph = show;
        }

        void SetShowDeviceInfo(const bool show) {
            mShowDeviceInfo = show;
        }

        void SetShowFrameInfo(const bool show) {
            mShowFrameInfo = show;
        }

        /// Make sure to call this between BeginFrame() and EndFrame()
        void Draw(const RenderContext& renderer, const Clock& clock) {
            if (mShowFrameGraph) { DrawFrameGraph(clock); }
            if (mShowDeviceInfo) { DrawDeviceInfo(renderer.GetDeviceInfo()); }
            if (mShowFrameInfo) { DrawFrameInfo(clock, renderer.GetFrameInfo()); }
        }

    private:
        void Initialize() {
            ImGuiIO& io = ImGui::GetIO();
            (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

            mFont = io.Fonts->AddFontFromMemoryCompressedTTF(JetBrainsMono_TTF_compressed_data,
                                                             JetBrainsMono_TTF_compressed_size,
                                                             16.0f);
        }

        void DrawFrameGraph(const Clock& clock) {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(ImVec2(0, viewport->Size.y - 300));
            ImGui::SetNextWindowSize(ImVec2(400, 300));

            StartWindowTransparent();
            if (!ImGui::Begin("##framegraph",
                              None,
                              ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                                ImGuiWindowFlags_NoTitleBar)) {
                ImGui::End();
                return;
            }
            EndWindowTransparent();

            ImGui::Text("Frame Graph");

            ImGui::End();
        }

        void DrawDeviceInfo(const DeviceInfo& info) {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(ImVec2(viewport->Size.x - 300, 0));
            ImGui::SetNextWindowSize(ImVec2(300, 200));

            StartWindowTransparent();
            if (!ImGui::Begin("##deviceinfo",
                              None,
                              ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                                ImGuiWindowFlags_NoTitleBar)) {
                ImGui::End();
                return;
            }
            EndWindowTransparent();

            ImGui::Text("Device Info");
            ImGui::Text(info.model.c_str());

            constexpr f64 kBytesPerGB = 1024.0 * 1024.0 * 1024.0;
            const auto gpuMemory =
              std::format("GPU Memory: {:.2f} GB", CAST<f32>((CAST<f64>(info.videoMemoryInBytes) / kBytesPerGB)));
            const auto sharedMemory =
              std::format("Shared Memory: {:.2f} GB", CAST<f32>((CAST<f64>(info.sharedMemoryInBytes) / kBytesPerGB)));

            ImGui::Text(gpuMemory.c_str());
            ImGui::Text(sharedMemory.c_str());

            ImGui::End();
        }

        void DrawFrameInfo(const Clock& clock, const FrameInfo& frameInfo) {
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2(300, 300));

            StartWindowTransparent();
            if (!ImGui::Begin("##frameinfo",
                              None,
                              ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                                ImGuiWindowFlags_NoTitleBar)) {
                ImGui::End();
                return;
            }
            EndWindowTransparent();

            ImGui::Text("Frame Info");
            ImGui::Text("==========");

            auto frameTime = std::format("Frame time : {:.2f}ms", clock.GetDeltaTime() * 1000.0f);
            auto frameRate = std::format("Frame rate : {:.0f} FPS", clock.GetFramesPerSecond());
            // auto drawCalls = std::format("Draw Calls : {}", frameInfo.drawCallsPerFrame);
            // auto triangles = std::format("Triangles  : {}", frameInfo.numTriangles);

            ImGui::Text(frameTime.c_str());
            ImGui::Text(frameRate.c_str());
            // ImGui::Text(drawCalls.c_str());
            // ImGui::Text(triangles.c_str());

            ImGui::End();
        }

        void RightAlignText(const vector<str>& lines) {
            // figure out which line is longest and store the length
            // offset the rest of the lines by that amount and window width or something idk
        }

        void StartWindowTransparent() {
            ImGui::PushStyleColor(ImGuiCol_WindowBg, 0x0000000);
            ImGui::PushStyleColor(ImGuiCol_Border, 0x0000000);
        }

        void EndWindowTransparent() {
            ImGui::PopStyleColor(2);
        }
    };
}  // namespace x