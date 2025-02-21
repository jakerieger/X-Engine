// Author: Jake Rieger
// Created: 2/18/2025.
//

#include "EditorWindow.hpp"
#include "FileDialogs.hpp"

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <imgui_internal.h>
#include <Inter.h>
#include <yaml-cpp/yaml.h>
#include <fstream>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace x::Editor {
    void EditorWindow::OnInitialize() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        io.IniFilename = None;  // Disable ini for now
        io.Fonts->AddFontFromMemoryCompressedTTF(Inter_compressed_data, Inter_compressed_size, 16.0f);

        ImGui_ImplWin32_Init(_hwnd);
        ImGui_ImplDX11_Init(_context.GetDevice(), _context.GetDeviceContext());

        ApplyTheme();

        // Initialize the engine core
        _windowViewport->SetClearColor(Colors::Black);
        _sceneViewport.SetClearColor(Colors::CornflowerBlue);
        _sceneViewport.Resize(100, 100);
        _game.Initialize(this, &_sceneViewport);
    }

    void EditorWindow::OnResize(u32 width, u32 height) {}

    void EditorWindow::OnShutdown() {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    void EditorWindow::Update() {
        _game.Update(!_gameRunning);
        _entities = _game.GetActiveScene()->GetEntities();
    }

    void EditorWindow::Render() {
        _windowViewport->ClearAll();

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        //============================================================================================================//
        //============================================================================================================//

        static bool firstTime = true;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New Scene", "Ctrl+N")) {}
                if (ImGui::MenuItem("Open Scene", "Ctrl+O")) {
                    const char* filter = "Scene (*.xscn)|*.xscn|";
                    char filename[MAX_PATH];
                    if (OpenFileDialog(_hwnd, None, filter, "Open Scene File", filename, MAX_PATH)) {
                        HandleOpenScene(filename);
                    }
                }
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
        ImGui::PopStyleVar();

        //============================================================================================================//
        //============================================================================================================//

        float menuBarHeight           = ImGui::GetFrameHeight();
        ImGuiWindowFlags toolbarFlags = ImGuiWindowFlags_NoTitleBar |            // No title bar needed
                                        ImGuiWindowFlags_NoScrollbar |           // Disable scrolling
                                        ImGuiWindowFlags_NoMove |                // Prevent moving
                                        ImGuiWindowFlags_NoResize |              // Prevent resizing
                                        ImGuiWindowFlags_NoCollapse |            // Prevent collapsing
                                        ImGuiWindowFlags_NoSavedSettings |       // Don't save position/size
                                        ImGuiWindowFlags_NoBringToFrontOnFocus;  // Don't change z-order

        ImGui::SetNextWindowPos(ImVec2(0, menuBarHeight));
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetMainViewport()->Size.x, 36));

        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2, 2));
        if (ImGui::Begin("##Toolbar", nullptr, toolbarFlags)) {
            // Add your toolbar buttons/tools here
            if (ImGui::Button("##transform", ImVec2(28, 28))) {}
            ImGui::SameLine();
            if (ImGui::Button("##rotate", ImVec2(28, 28))) {}
            ImGui::SameLine();
            if (ImGui::Button("##scale", ImVec2(28, 28))) {}
            // ... add more toolbar items

            ImGui::End();
        }
        ImGui::PopStyleVar(2);

        auto* imguiViewport      = ImGui::GetWindowViewport();
        ImGuiDockNodeFlags flags = ImGuiDockNodeFlags_PassthruCentralNode;

        ImGui::SetNextWindowPos(ImVec2(0, menuBarHeight + 36));
        ImGui::SetNextWindowSize(ImVec2(imguiViewport->Size.x, imguiViewport->Size.y - menuBarHeight - 36));
        ImGui::SetNextWindowViewport(imguiViewport->ID);

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                                       ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                       ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                       ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        if (ImGui::Begin("DockSpace", nullptr, windowFlags)) {
            ImGuiID dockspaceId = ImGui::GetID("Editor::DockSpace");
            ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), flags);

            if (firstTime) {
                firstTime = false;
                ImGui::DockBuilderRemoveNode(dockspaceId);
                ImGui::DockBuilderAddNode(dockspaceId, flags | ImGuiDockNodeFlags_DockSpace);
                ImGui::DockBuilderSetNodeSize(dockspaceId, ImGui::GetWindowSize());

                ImGuiID dockMainId   = dockspaceId;
                ImGuiID dockRightId  = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Right, 0.2f, None, &dockMainId);
                ImGuiID dockLeftId   = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Left, 0.2f, None, &dockMainId);
                ImGuiID dockBottomId = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Down, 0.2f, None, &dockMainId);

                ImGui::DockBuilderDockWindow("Entities", dockLeftId);
                ImGui::DockBuilderDockWindow("Properties", dockRightId);
                ImGui::DockBuilderDockWindow("Scene", dockMainId);
                ImGui::DockBuilderDockWindow("Editor Log", dockBottomId);

                ImGui::DockBuilderFinish(imguiViewport->ID);
            }

            ImGui::End();
        }
        ImGui::PopStyleVar(3);

        //============================================================================================================//
        //============================================================================================================//

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Scene");
        {
            ImVec2 contentSize       = ImGui::GetContentRegionAvail();
            const auto contentWidth  = CAST<u32>(contentSize.x);
            const auto contentHeight = CAST<u32>(contentSize.y);

            _sceneViewport.Resize(contentWidth, contentHeight);
            _sceneViewport.BindRenderTarget();
            _sceneViewport.ClearRenderTargetView();
            _sceneViewport.AttachViewport();

            _game.Resize(contentWidth, contentHeight);
            _game.RenderFrame();

            auto* srv = _sceneViewport.GetShaderResourceView().Get();
            ImGui::Image(ImTextureID((void*)srv), contentSize);
        }
        ImGui::End();
        ImGui::PopStyleVar();

        ImGui::Begin("Entities");
        {
            for (auto [name, id] : _entities) {
                if (ImGui::Selectable(name.c_str(), id == _selectedEntity)) { _selectedEntity = id; }
            }
        }
        ImGui::End();

        ImGui::Begin("Properties");
        {}
        ImGui::End();

        ImGui::Begin("Editor Log");
        {}
        ImGui::End();

        _windowViewport->AttachViewport();
        _windowViewport->BindRenderTarget();
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    LRESULT EditorWindow::MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) {
        if (ImGui_ImplWin32_WndProcHandler(_hwnd, msg, wParam, lParam)) return true;
        return Window::MessageHandler(msg, wParam, lParam);
    }

    void EditorWindow::HandleOpenScene(const char* filename) {
        _game.TransitionScene(filename);

        auto& sceneState      = _game.GetActiveScene()->GetState();
        _sceneCamera          = sceneState.MainCamera;  // Cache scene camera
        sceneState.MainCamera = _editorCamera;  // Override scene camera with editor camera while not in play mode
    }

    static ImVec4 HexToImVec4(const str& hex,
                              const f32 alpha = 1.0f) {  // Ensure the string is the correct length
        if (hex.length() != 6) { throw std::invalid_argument("Hex color should be in the format 'RRGGBB'"); }

        ImVec4 color;
        const char red[3]   = {hex[0], hex[1], '\0'};
        const char green[3] = {hex[2], hex[3], '\0'};
        const char blue[3]  = {hex[4], hex[5], '\0'};

        const int r = strtol(red, nullptr, 16);
        const int g = strtol(green, nullptr, 16);
        const int b = strtol(blue, nullptr, 16);

        color.x = (float)r / 255.0f;
        color.y = (float)g / 255.0f;
        color.z = (float)b / 255.0f;
        color.w = alpha;

        return color;
    }

    void EditorWindow::ApplyTheme() {
        auto theme = R""(
  Name: Dark
  WindowBackground: 242324
  ChildBackground: 242324
  FrameBackground: 434243
  SecondaryBackground: 363636
  HeaderBackground: 040404
  TextHighlight: FFFFFF
  TextPrimary: DADADA
  TextSecondary: B3B3B3
  TextDisabled: 666666
  Border: 181818
  Error: FF0000
  Warning: FFFF00
  Success: 00FF00
  Link: 0000FF
  Scrollbar: 353535
  Primary: 6190e6
  Secondary: 585858
  BorderRadius: 0.0
  BorderWidth: 1.0
  Font: Inter-Regular.ttf
)"";

        auto themeYaml = YAML::Load(theme);

        const auto borderRadius = themeYaml["BorderRadius"].as<f32>();
        const auto borderWidth  = themeYaml["BorderWidth"].as<f32>();

        ImGuiStyle& style   = ImGui::GetStyle();
        ImVec4* styleColors = style.Colors;

        style.WindowRounding   = borderRadius;
        style.FrameRounding    = borderRadius;
        style.WindowBorderSize = borderWidth;
        style.FrameBorderSize  = 0.f;
        style.TabRounding      = borderRadius;

        // Load colors
        const auto windowBackground    = HexToImVec4(themeYaml["WindowBackground"].as<str>());
        const auto childBackground     = HexToImVec4(themeYaml["ChildBackground"].as<str>());
        const auto frameBackground     = HexToImVec4(themeYaml["FrameBackground"].as<str>());
        const auto secondaryBackground = HexToImVec4(themeYaml["SecondaryBackground"].as<str>());
        const auto headerBackground    = HexToImVec4(themeYaml["HeaderBackground"].as<str>());
        const auto textHighlight       = HexToImVec4(themeYaml["TextHighlight"].as<str>());
        const auto textPrimary         = HexToImVec4(themeYaml["TextPrimary"].as<str>());
        const auto textSecondary       = HexToImVec4(themeYaml["TextSecondary"].as<str>());
        const auto textDisabled        = HexToImVec4(themeYaml["TextDisabled"].as<str>());
        const auto border              = HexToImVec4(themeYaml["Border"].as<str>());
        const auto error               = HexToImVec4(themeYaml["Error"].as<str>());
        const auto warning             = HexToImVec4(themeYaml["Warning"].as<str>());
        const auto success             = HexToImVec4(themeYaml["Success"].as<str>());
        const auto link                = HexToImVec4(themeYaml["Link"].as<str>());
        const auto scrollbar           = HexToImVec4(themeYaml["Scrollbar"].as<str>());
        const auto primary             = HexToImVec4(themeYaml["Primary"].as<str>());
        const auto secondary           = HexToImVec4(themeYaml["Secondary"].as<str>());

        styleColors[ImGuiCol_Text]                 = textPrimary;
        styleColors[ImGuiCol_TextDisabled]         = textDisabled;
        styleColors[ImGuiCol_WindowBg]             = windowBackground;
        styleColors[ImGuiCol_ChildBg]              = childBackground;
        styleColors[ImGuiCol_PopupBg]              = windowBackground;
        styleColors[ImGuiCol_Border]               = border;
        styleColors[ImGuiCol_BorderShadow]         = ImVec4(0.f, 0.f, 0.f, 0.f);
        styleColors[ImGuiCol_FrameBg]              = frameBackground;
        styleColors[ImGuiCol_FrameBgHovered]       = secondaryBackground;
        styleColors[ImGuiCol_FrameBgActive]        = secondaryBackground;
        styleColors[ImGuiCol_TitleBg]              = frameBackground;
        styleColors[ImGuiCol_TitleBgActive]        = frameBackground;
        styleColors[ImGuiCol_TitleBgCollapsed]     = frameBackground;
        styleColors[ImGuiCol_MenuBarBg]            = windowBackground;
        styleColors[ImGuiCol_ScrollbarBg]          = windowBackground;
        styleColors[ImGuiCol_ScrollbarGrab]        = scrollbar;
        styleColors[ImGuiCol_ScrollbarGrabHovered] = scrollbar;
        styleColors[ImGuiCol_ScrollbarGrabActive]  = scrollbar;
        styleColors[ImGuiCol_CheckMark]            = textPrimary;
        styleColors[ImGuiCol_SliderGrab]           = scrollbar;
        styleColors[ImGuiCol_SliderGrabActive]     = scrollbar;
        styleColors[ImGuiCol_Button]               = secondary;
        styleColors[ImGuiCol_ButtonHovered]        = ImVec4(secondary.x, secondary.y, secondary.z, 0.75f);
        styleColors[ImGuiCol_ButtonActive]         = ImVec4(secondary.x, secondary.y, secondary.z, 0.60f);
        styleColors[ImGuiCol_Header]               = secondaryBackground;
        styleColors[ImGuiCol_HeaderHovered]        = headerBackground;
        styleColors[ImGuiCol_HeaderActive]         = headerBackground;
        styleColors[ImGuiCol_Separator]            = border;
        styleColors[ImGuiCol_SeparatorHovered]     = link;
        styleColors[ImGuiCol_SeparatorActive]      = link;
        styleColors[ImGuiCol_ResizeGrip]           = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
        styleColors[ImGuiCol_ResizeGripHovered]    = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        styleColors[ImGuiCol_ResizeGripActive]     = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        styleColors[ImGuiCol_Tab]                  = headerBackground;
        styleColors[ImGuiCol_TabHovered]           = headerBackground;
        styleColors[ImGuiCol_TabActive]            = headerBackground;
        styleColors[ImGuiCol_TabUnfocused]         = styleColors[ImGuiCol_Tab];
        styleColors[ImGuiCol_TabUnfocusedActive]   = styleColors[ImGuiCol_TabActive];
        styleColors[ImGuiCol_TableBorderLight]     = ImVec4(0.f, 0.f, 0.f, 0.f);
        styleColors[ImGuiCol_TableBorderStrong]    = ImVec4(0.f, 0.f, 0.f, 0.f);
        styleColors[ImGuiCol_PlotLines]            = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        styleColors[ImGuiCol_PlotLinesHovered]     = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        styleColors[ImGuiCol_PlotHistogram]        = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        styleColors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        styleColors[ImGuiCol_TableHeaderBg]        = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
        styleColors[ImGuiCol_TableBorderStrong]    = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);  // Prefer using Alpha=1.0 here
        styleColors[ImGuiCol_TableBorderLight]     = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);  // Prefer using Alpha=1.0 here
        styleColors[ImGuiCol_TableRowBg]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        styleColors[ImGuiCol_TableRowBgAlt]        = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
        styleColors[ImGuiCol_TextSelectedBg]       = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        styleColors[ImGuiCol_DragDropTarget]       = link;
        styleColors[ImGuiCol_NavHighlight]         = ImVec4(30.f / 255.f, 30.f / 255.f, 30.f / 255.f, 1.00f);
        styleColors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        styleColors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        styleColors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.00f, 0.00f, 0.00f, 0.0f);
    }
}  // namespace x::Editor
