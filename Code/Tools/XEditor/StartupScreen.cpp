// Author: Jake Rieger
// Created: 4/12/2025.
//

#include <Inter.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <imgui_internal.h>

#include "Res/resource.h"
#include "StartupScreen.hpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace x {
    void StartupScreen::OnInitialize() {
        SetWindowTitle("XEditor");
        this->SetWindowIcon(APPICON);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        io.IniFilename = nullptr;  // Disable ui config for now

        ImFontAtlas* fontAtlas = io.Fonts;
        mDefaultFont           = fontAtlas->AddFontDefault();
        mFonts["display"] =
          fontAtlas->AddFontFromMemoryCompressedTTF(Inter_compressed_data, Inter_compressed_size, 16.0f);
        fontAtlas->Build();

        ImGui_ImplWin32_Init(mHwnd);
        ImGui_ImplDX11_Init(mContext.GetDevice(), mContext.GetDeviceContext());

        mWindowViewport->SetClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        // TODO: Implement theming for ALL windows
        // mTheme.LoadTheme(mSettings.mTheme);
        // mTheme.Apply();
    }

    void StartupScreen::OnResize(u32 width, u32 height) {
        IWindow::OnResize(width, height);
    }

    void StartupScreen::OnShutdown() {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    void StartupScreen::OnUpdate() {
        IWindow::OnUpdate();
    }

    void StartupScreen::OnRender() {
        mWindowViewport->ClearAll();

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::PushFont(mFonts["display"]);

        ImGui::Begin("Startup");
        {}
        ImGui::End();

        ImGui::PopFont();

        mWindowViewport->AttachViewport();
        mWindowViewport->BindRenderTarget();
        ImGui::Render();

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    LRESULT StartupScreen::MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) {
        if (ImGui_ImplWin32_WndProcHandler(mHwnd, msg, wParam, lParam)) return true;
        return IWindow::MessageHandler(msg, wParam, lParam);
    }
}  // namespace x