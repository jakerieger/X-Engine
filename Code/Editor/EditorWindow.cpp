// Author: Jake Rieger
// Created: 2/18/2025.
//

#include "Engine/EngineCommon.hpp"
#include "EditorWindow.hpp"
#include "Controls.hpp"
#include "FileDialogs.hpp"

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <imgui_internal.h>
#include <Inter.h>
#include <yaml-cpp/yaml.h>
#include "EditorIcons.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace x::Editor {
    static ImVec4 HexToImVec4(const str& hex,
                              const f32 alpha = 1.0f) {  // Ensure the string is the correct length
        if (hex.length() != 6) { throw std::invalid_argument("Hex color should be in the format 'RRGGBB'"); }

        ImVec4 color;
        const char red[3]   = {hex[0], hex[1], '\0'};
        const char green[3] = {hex[2], hex[3], '\0'};
        const char blue[3]  = {hex[4], hex[5], '\0'};

        const int r = strtol(red, None, 16);
        const int g = strtol(green, None, 16);
        const int b = strtol(blue, None, 16);

        color.x = (f32)r / 255.0f;
        color.y = (f32)g / 255.0f;
        color.z = (f32)b / 255.0f;
        color.w = alpha;

        return color;
    }

    static ImVec4 GetLogEntryColor(const u32 severity) {
        switch (severity) {
            case X_LOG_SEVERITY_INFO:
                return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);  // White
            case X_LOG_SEVERITY_WARN:
                return ImVec4(1.0f, 1.0f, 0.0f, 1.0f);  // Yellow
            case X_LOG_SEVERITY_ERROR:
                return ImVec4(1.0f, 0.4f, 0.4f, 1.0f);  // Red
            case X_LOG_SEVERITY_FATAL:
                return ImVec4(1.0f, 0.0f, 0.0f, 1.0f);  // Bright Red
            case X_LOG_SEVERITY_DEBUG:
                return ImVec4(0.5f, 1.0f, 0.5f, 1.0f);  // Green
            default:
                return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        }
    }

    bool EditorWindow::LoadTextures() {
        if (!_textureManager.LoadFromMemory(MOVE_BYTES, 24, 24, 4, "MoveIcon")) {
            X_LOG_ERROR("Failed to load 'MoveIcon'");
            return false;
        }
        if (!_textureManager.LoadFromMemory(SELECT_BYTES, 24, 24, 4, "SelectIcon")) {
            X_LOG_ERROR("Failed to load 'SelectIcon'");
            return false;
        }
        if (!_textureManager.LoadFromMemory(ROTATE_BYTES, 24, 24, 4, "RotateIcon")) {
            X_LOG_ERROR("Failed to load 'RotateIcon'");
            return false;
        }
        if (!_textureManager.LoadFromMemory(SCALE_BYTES, 24, 24, 4, "ScaleIcon")) {
            X_LOG_ERROR("Failed to load 'ScaleIcon'");
            return false;
        }
        if (!_textureManager.LoadFromMemory(PLAY_BYTES, 24, 24, 4, "PlayIcon")) {
            X_LOG_ERROR("Failed to load 'PlayIcon'");
            return false;
        }
        if (!_textureManager.LoadFromMemory(UNDO_BYTES, 24, 24, 4, "UndoIcon")) {
            X_LOG_ERROR("Failed to load 'UndoIcon'");
            return false;
        }
        if (!_textureManager.LoadFromMemory(REDO_BYTES, 24, 24, 4, "RedoIcon")) {
            X_LOG_ERROR("Failed to load 'RedoIcon'");
            return false;
        }
        if (!_textureManager.LoadFromMemory(SEPARATOR_BYTES, 24, 24, 4, "SeparatorIcon")) {
            X_LOG_ERROR("Failed to load 'SeparatorIcon'");
            return false;
        }
        if (!_textureManager.LoadFromMemory(SETTINGS_BYTES, 24, 24, 4, "SettingsIcon")) {
            X_LOG_ERROR("Failed to load 'SettingsIcon'");
            return false;
        }
        if (!_textureManager.LoadFromMemory(SNAP_TO_GRID_BYTES, 24, 24, 4, "SnapToGridIcon")) {
            X_LOG_ERROR("Failed to load 'SnapToGridIcon'");
            return false;
        }
        if (!_textureManager.LoadFromMemory(PAUSE_BYTES, 24, 24, 4, "PauseIcon")) {
            X_LOG_ERROR("Failed to load 'PauseIcon'");
            return false;
        }
        if (!_textureManager.LoadFromMemory(STOP_BYTES, 24, 24, 4, "StopIcon")) {
            X_LOG_ERROR("Failed to load 'StopIcon'");
            return false;
        }
        return true;
    }

    void EditorWindow::OnInitialize() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        io.IniFilename = "XEditorConfig.ini";

        ImFontAtlas* fontAtlas = io.Fonts;
        _defaultFont           = fontAtlas->AddFontDefault();
        _fonts["display"] =
          fontAtlas->AddFontFromMemoryCompressedTTF(Inter_compressed_data, Inter_compressed_size, 16.0f);
        _fonts["mono"] = fontAtlas->AddFontFromMemoryCompressedTTF(JetBrainsMono_TTF_compressed_data,
                                                                   JetBrainsMono_TTF_compressed_size,
                                                                   16.0f);
        fontAtlas->Build();

        ImGui_ImplWin32_Init(_hwnd);
        ImGui_ImplDX11_Init(_context.GetDevice(), _context.GetDeviceContext());

        ApplyTheme();

        // Load icons
        if (!LoadTextures()) { X_PANIC("Failed to load editor textures."); }

        // Initialize the engine core
        _windowViewport->SetClearColor(Colors::Black);
        _sceneViewport.SetClearColor(Colors::CornflowerBlue);
        _sceneViewport.Resize(100,
                              100);  // Give the viewport a default size to prevent DX11 resource creation from failing.
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

    void EditorWindow::MainMenu() {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New Scene", "Ctrl+N")) { NewScene(); }
                if (ImGui::MenuItem("Open Scene", "Ctrl+O")) {
                    const char* filter = "Scene (*.xscn)|*.xscn|";
                    char filename[MAX_PATH];
                    if (OpenFileDialog(_hwnd, None, filter, "Open Scene File", filename, MAX_PATH)) {
                        OpenScene(filename);
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
                    _layoutSetup = false;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
        ImGui::PopStyleVar();
    }

    void EditorWindow::SetupDockspace(const f32 yOffset) {
        const auto* imguiViewport          = ImGui::GetWindowViewport();
        constexpr ImGuiDockNodeFlags flags = ImGuiDockNodeFlags_PassthruCentralNode;

        ImGui::SetNextWindowPos(ImVec2(0, yOffset));
        ImGui::SetNextWindowSize(ImVec2(imguiViewport->Size.x, imguiViewport->Size.y - yOffset));
        ImGui::SetNextWindowViewport(imguiViewport->ID);

        constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                                                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                                 ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        if (ImGui::Begin("DockSpace", None, windowFlags)) {
            const ImGuiID dockspaceId = ImGui::GetID("Editor::DockSpace");
            ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), flags);

            if (!_layoutSetup) {
                _layoutSetup = true;

                ImGui::DockBuilderRemoveNode(dockspaceId);
                ImGui::DockBuilderAddNode(dockspaceId, flags | ImGuiDockNodeFlags_DockSpace);
                ImGui::DockBuilderSetNodeSize(dockspaceId, ImGui::GetWindowSize());

                ImGuiID dockMainId   = dockspaceId;
                ImGuiID dockRightId  = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Right, 0.2f, None, &dockMainId);
                ImGuiID dockLeftId   = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Left, 0.2f, None, &dockMainId);
                ImGuiID dockBottomId = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Down, 0.3f, None, &dockMainId);
                ImGuiID dockRightBottomId =
                  ImGui::DockBuilderSplitNode(dockRightId, ImGuiDir_Down, 0.5f, None, &dockRightId);

                ImGui::DockBuilderDockWindow("Entities", dockLeftId);
                ImGui::DockBuilderDockWindow("Properties", dockRightId);
                ImGui::DockBuilderDockWindow("Scene", dockMainId);
                ImGui::DockBuilderDockWindow("Assets", dockBottomId);
                ImGui::DockBuilderDockWindow("Editor Log", dockBottomId);
                ImGui::DockBuilderDockWindow("World Settings", dockRightBottomId);

                ImGui::DockBuilderFinish(imguiViewport->ID);
            }

            ImGui::End();
        }

        ImGui::PopStyleVar(3);
    }

    void EditorWindow::SceneView() {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Scene");
        {
            const ImVec2 contentSize = ImGui::GetContentRegionAvail();
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
    }

    void EditorWindow::EntitiesView() {
        ImGui::Begin("Entities");
        {
            for (auto& [name, id] : _entities) {
                if (ImGui::Selectable(name.c_str(), id == _selectedEntity)) {
                    _selectedEntity = id;
                    _propertiesPanel.Update(id);
                }
            }
        }
        ImGui::End();
    }

    void EditorWindow::WorldSettingsView() {
        ImGui::Begin("World Settings");
        {}
        ImGui::End();
    }

    void EditorWindow::PropertiesView() {
        ImGui::Begin("Properties");
        { _propertiesPanel.Draw(_selectedEntity); }
        ImGui::End();
    }

    void EditorWindow::AssetsView() {
        ImGui::Begin("Assets");
        {}
        ImGui::End();
    }

    void EditorWindow::EditorLogView() {
        static bool showInfo   = true;
        static bool showWarn   = true;
        static bool showError  = true;
        static bool showFatal  = true;
        static bool showDebug  = true;
        static bool autoScroll = true;

        auto& logger = GetLogger();

        auto ShouldShowSeverity = [&](const u32 severity) -> bool {
            switch (severity) {
                case X_LOG_SEVERITY_INFO:
                    return showInfo;
                case X_LOG_SEVERITY_WARN:
                    return showWarn;
                case X_LOG_SEVERITY_ERROR:
                    return showError;
                case X_LOG_SEVERITY_FATAL:
                    return showFatal;
                case X_LOG_SEVERITY_DEBUG:
                    return showDebug;
                default:
                    return true;
            }
        };

        ImGui::Begin("Editor Log");
        {
            if (ImGui::Button("Clear")) { logger.ClearEntries(); }
            ImGui::SameLine();
            ImGui::Checkbox("Info", &showInfo);
            ImGui::SameLine();
            ImGui::Checkbox("Warn", &showWarn);
            ImGui::SameLine();
            ImGui::Checkbox("Error", &showError);
            ImGui::SameLine();
            ImGui::Checkbox("Fatal", &showFatal);
            ImGui::SameLine();
            ImGui::Checkbox("Debug", &showDebug);

            ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

            // Display logs
            std::lock_guard<std::mutex> lock(logger.GetBufferMutex());
            size_t startIndex =
              (logger.GetCurrentEntry() - logger.GetTotalEntries() + logger.kMaxEntries) % logger.kMaxEntries;
            for (size_t i = 0; i < logger.GetTotalEntries(); i++) {
                size_t index      = (startIndex + i) % logger.kMaxEntries;
                const auto& entry = logger.GetEntries()[index];

                // Apply filters
                if (!ShouldShowSeverity(entry.severity)) continue;

                ImGui::PushStyleColor(ImGuiCol_Text, GetLogEntryColor(entry.severity));
                ImGui::TextUnformatted(std::format("[{}] {}", entry.timestamp, entry.message).c_str());
                ImGui::PopStyleColor();
            }

            // Auto-scroll to bottom
            if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) { ImGui::SetScrollHereY(1.0f); }

            ImGui::EndChild();
        }
        ImGui::End();
    }

    void EditorWindow::Render() {
        _windowViewport->ClearAll();

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::PushFont(_fonts["display"]);

        //============================================================================================================//
        // MENU BAR
        //============================================================================================================//

        MainMenu();
        const f32 menuBarHeight = ImGui::GetFrameHeight();

        //============================================================================================================//
        // TOOL BAR
        //============================================================================================================//

        Toolbar(this, _textureManager, menuBarHeight);

        //============================================================================================================//
        // EDITOR PANELS
        //============================================================================================================//

        SetupDockspace(menuBarHeight + kToolbarHeight);

        SceneView();
        EntitiesView();
        WorldSettingsView();
        PropertiesView();
        AssetsView();
        EditorLogView();

        ImGui::PopFont();

        _windowViewport->AttachViewport();
        _windowViewport->BindRenderTarget();
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    LRESULT EditorWindow::MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) {
        if (ImGui_ImplWin32_WndProcHandler(_hwnd, msg, wParam, lParam)) return true;
        return Window::MessageHandler(msg, wParam, lParam);
    }

    void EditorWindow::OpenScene(const char* filename) {
        _game.TransitionScene(filename);

        auto& sceneState      = _game.GetActiveScene()->GetState();
        _sceneCamera          = sceneState.MainCamera;  // Cache scene camera
        sceneState.MainCamera = _editorCamera;  // Override scene camera with editor camera while not in play mode

        _propertiesPanel.OnSceneTransition();
    }

    void EditorWindow::TogglePlayMode() {
        if (!_game.SceneValid()) return;

        if (_gameRunning) {
            // Reset camera back to editor camera
            _game.GetActiveScene()->ResetToInitialState();
            _game.GetActiveScene()->Update(0.0f);
            auto& sceneState      = _game.GetActiveScene()->GetState();
            sceneState.MainCamera = _editorCamera;

            _gameRunning = false;
        } else {
            // Reset camera back to scene camera
            _game.GetActiveScene()->Update(0.0f);
            auto& sceneState      = _game.GetActiveScene()->GetState();
            sceneState.MainCamera = _sceneCamera;

            _gameRunning = true;
        }
    }

    void EditorWindow::NewScene() {
        _game.GetActiveScene()->Reset();
        _entities.clear();
    }

    void EditorWindow::ApplyTheme() {
        auto theme = R""(
  Name: Dark
  WindowBackground: 17171a
  ChildBackground: 242324
  FrameBackground: 212127
  SecondaryBackground: 363636
  HeaderBackground: 36363a
  TextHighlight: FFFFFF
  TextPrimary: DADADA
  TextSecondary: B3B3B3
  TextDisabled: 666666
  Border: 1e1e22
  Error: FF0000
  Warning: FFFF00
  Success: 00FF00
  Link: 0000FF
  Scrollbar: 353535
  Primary: 61aced
  Secondary: 585858
  BorderRadius: 2.0
  BorderWidth: 1.0
)"";

        auto themeYaml = YAML::Load(theme);

        const auto borderRadius = themeYaml["BorderRadius"].as<f32>();
        const auto borderWidth  = themeYaml["BorderWidth"].as<f32>();

        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors    = style.Colors;

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

        colors[ImGuiCol_Text]                  = textPrimary;
        colors[ImGuiCol_TextDisabled]          = textDisabled;
        colors[ImGuiCol_WindowBg]              = windowBackground;
        colors[ImGuiCol_ChildBg]               = childBackground;
        colors[ImGuiCol_PopupBg]               = windowBackground;
        colors[ImGuiCol_Border]                = border;
        colors[ImGuiCol_BorderShadow]          = ImVec4(0.f, 0.f, 0.f, 0.f);
        colors[ImGuiCol_FrameBg]               = frameBackground;
        colors[ImGuiCol_FrameBgHovered]        = secondaryBackground;
        colors[ImGuiCol_FrameBgActive]         = secondaryBackground;
        colors[ImGuiCol_TitleBg]               = frameBackground;
        colors[ImGuiCol_TitleBgActive]         = frameBackground;
        colors[ImGuiCol_TitleBgCollapsed]      = frameBackground;
        colors[ImGuiCol_MenuBarBg]             = windowBackground;
        colors[ImGuiCol_ScrollbarBg]           = windowBackground;
        colors[ImGuiCol_ScrollbarGrab]         = scrollbar;
        colors[ImGuiCol_ScrollbarGrabHovered]  = scrollbar;
        colors[ImGuiCol_ScrollbarGrabActive]   = scrollbar;
        colors[ImGuiCol_CheckMark]             = textPrimary;
        colors[ImGuiCol_SliderGrab]            = scrollbar;
        colors[ImGuiCol_SliderGrabActive]      = scrollbar;
        colors[ImGuiCol_Button]                = secondary;
        colors[ImGuiCol_ButtonHovered]         = ImVec4(secondary.x, secondary.y, secondary.z, 0.75f);
        colors[ImGuiCol_ButtonActive]          = ImVec4(secondary.x, secondary.y, secondary.z, 0.60f);
        colors[ImGuiCol_Header]                = secondaryBackground;
        colors[ImGuiCol_HeaderHovered]         = headerBackground;
        colors[ImGuiCol_HeaderActive]          = headerBackground;
        colors[ImGuiCol_Separator]             = border;
        colors[ImGuiCol_SeparatorHovered]      = link;
        colors[ImGuiCol_SeparatorActive]       = link;
        colors[ImGuiCol_ResizeGrip]            = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
        colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        colors[ImGuiCol_Tab]                   = headerBackground;
        colors[ImGuiCol_TabHovered]            = headerBackground;
        colors[ImGuiCol_TabActive]             = headerBackground;
        colors[ImGuiCol_TabUnfocused]          = colors[ImGuiCol_Tab];
        colors[ImGuiCol_TabUnfocusedActive]    = colors[ImGuiCol_TabActive];
        colors[ImGuiCol_TableBorderLight]      = ImVec4(0.f, 0.f, 0.f, 0.f);
        colors[ImGuiCol_TableBorderStrong]     = ImVec4(0.f, 0.f, 0.f, 0.f);
        colors[ImGuiCol_PlotLines]             = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        colors[ImGuiCol_TableHeaderBg]         = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
        colors[ImGuiCol_TableBorderStrong]     = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);  // Prefer using Alpha=1.0 here
        colors[ImGuiCol_TableBorderLight]      = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);  // Prefer using Alpha=1.0 here
        colors[ImGuiCol_TableRowBg]            = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_TableRowBgAlt]         = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
        colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        colors[ImGuiCol_DragDropTarget]        = link;
        colors[ImGuiCol_NavHighlight]          = ImVec4(30.f / 255.f, 30.f / 255.f, 30.f / 255.f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.00f, 0.00f, 0.00f, 0.0f);
        colors[ImGuiCol_DockingPreview]        = primary;
    }
}  // namespace x::Editor
