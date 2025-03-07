// Author: Jake Rieger
// Created: 2/18/2025.
//

#include "Engine/EngineCommon.hpp"
#include "EditorWindow.hpp"
#include "Controls.hpp"
#include "FileDialogs.hpp"
#include "EditorIcons.h"

#include <Inter.h>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <imgui_internal.h>
#include <yaml-cpp/yaml.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace x::Editor {
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
        if (!mTextureManager.LoadFromMemory(MOVE_BYTES, 24, 24, 4, "MoveIcon")) {
            X_LOG_ERROR("Failed to load 'MoveIcon'");
            return false;
        }
        if (!mTextureManager.LoadFromMemory(SELECT_BYTES, 24, 24, 4, "SelectIcon")) {
            X_LOG_ERROR("Failed to load 'SelectIcon'");
            return false;
        }
        if (!mTextureManager.LoadFromMemory(ROTATE_BYTES, 24, 24, 4, "RotateIcon")) {
            X_LOG_ERROR("Failed to load 'RotateIcon'");
            return false;
        }
        if (!mTextureManager.LoadFromMemory(SCALE_BYTES, 24, 24, 4, "ScaleIcon")) {
            X_LOG_ERROR("Failed to load 'ScaleIcon'");
            return false;
        }
        if (!mTextureManager.LoadFromMemory(PLAY_BYTES, 24, 24, 4, "PlayIcon")) {
            X_LOG_ERROR("Failed to load 'PlayIcon'");
            return false;
        }
        if (!mTextureManager.LoadFromMemory(UNDO_BYTES, 24, 24, 4, "UndoIcon")) {
            X_LOG_ERROR("Failed to load 'UndoIcon'");
            return false;
        }
        if (!mTextureManager.LoadFromMemory(REDO_BYTES, 24, 24, 4, "RedoIcon")) {
            X_LOG_ERROR("Failed to load 'RedoIcon'");
            return false;
        }
        if (!mTextureManager.LoadFromMemory(SEPARATOR_BYTES, 24, 24, 4, "SeparatorIcon")) {
            X_LOG_ERROR("Failed to load 'SeparatorIcon'");
            return false;
        }
        if (!mTextureManager.LoadFromMemory(SETTINGS_BYTES, 24, 24, 4, "SettingsIcon")) {
            X_LOG_ERROR("Failed to load 'SettingsIcon'");
            return false;
        }
        if (!mTextureManager.LoadFromMemory(SNAP_TO_GRID_BYTES, 24, 24, 4, "SnapToGridIcon")) {
            X_LOG_ERROR("Failed to load 'SnapToGridIcon'");
            return false;
        }
        if (!mTextureManager.LoadFromMemory(PAUSE_BYTES, 24, 24, 4, "PauseIcon")) {
            X_LOG_ERROR("Failed to load 'PauseIcon'");
            return false;
        }
        if (!mTextureManager.LoadFromMemory(STOP_BYTES, 24, 24, 4, "StopIcon")) {
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
        mDefaultFont           = fontAtlas->AddFontDefault();
        mFonts["display"] =
          fontAtlas->AddFontFromMemoryCompressedTTF(Inter_compressed_data, Inter_compressed_size, 16.0f);
        mFonts["mono"] = fontAtlas->AddFontFromMemoryCompressedTTF(JetBrainsMono_TTF_compressed_data,
                                                                   JetBrainsMono_TTF_compressed_size,
                                                                   16.0f);
        fontAtlas->Build();

        ImGui_ImplWin32_Init(mHwnd);
        ImGui_ImplDX11_Init(mContext.GetDevice(), mContext.GetDeviceContext());

        ApplyTheme();

        // Load icons
        if (!LoadTextures()) { X_PANIC("Failed to load editor textures."); }

        mTextEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
        mTextEditor.SetShowWhitespaces(false);

        // Initialize the engine core
        mWindowViewport->SetClearColor(Colors::Black);
        mSceneViewport.SetClearColor(Colors::CornflowerBlue);
        mSceneViewport.Resize(100,
                              100);  // Give the viewport a default size to prevent DX11 resource creation from failing.
    }

    void EditorWindow::OnResize(u32 width, u32 height) {}

    void EditorWindow::OnShutdown() {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    void EditorWindow::Update() {
        // mGame.Update(!mGameRunning);
        // mEntities = mGame.GetActiveScene()->GetEntities();
        //
        // // Update scene with zero tick
        // if (!InPlayMode()) { mGame.GetActiveScene()->Update(0.f); }
    }

    void EditorWindow::MainMenu() {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New Project", "Ctrl+Shift+N")) {}
                if (ImGui::MenuItem("Open Project", "Ctrl+Shift+O")) {
                    const char* filter = "Project (*.xproj)|*.xproj|";
                    char filename[MAX_PATH];
                    if (OpenFileDialog(mHwnd, nullptr, filter, "Open Project", filename, MAX_PATH)) {
                        OpenProject(filename);
                    }
                }
                if (ImGui::MenuItem("Save Project", "Ctrl+Shift+S")) {}
                ImGui::Separator();
                if (ImGui::MenuItem("New Scene", "Ctrl+N")) { NewScene(); }
                if (ImGui::MenuItem("Open Scene", "Ctrl+O")) {
                    const char* filter = "Scene (*.scene)|*.scene|";
                    char filename[MAX_PATH];
                    if (OpenFileDialog(mHwnd, nullptr, filter, "Open Scene File", filename, MAX_PATH)) {
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
                    mLayoutSetup = false;
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

        if (ImGui::Begin("DockSpace", nullptr, windowFlags)) {
            const ImGuiID dockspaceId = ImGui::GetID("Editor::DockSpace");
            ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), flags);

            if (!mLayoutSetup) {
                mLayoutSetup = true;

                ImGui::DockBuilderRemoveNode(dockspaceId);
                ImGui::DockBuilderAddNode(dockspaceId, flags | ImGuiDockNodeFlags_DockSpace);
                ImGui::DockBuilderSetNodeSize(dockspaceId, ImGui::GetWindowSize());

                ImGuiID dockMainId = dockspaceId;
                ImGuiID dockRightId =
                  ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Right, 0.2f, nullptr, &dockMainId);
                ImGuiID dockLeftId = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Left, 0.2f, nullptr, &dockMainId);
                ImGuiID dockBottomId =
                  ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Down, 0.3f, nullptr, &dockMainId);
                ImGuiID dockRightBottomId =
                  ImGui::DockBuilderSplitNode(dockRightId, ImGuiDir_Down, 0.5f, nullptr, &dockRightId);

                ImGui::DockBuilderDockWindow("Entities", dockLeftId);
                ImGui::DockBuilderDockWindow("Properties", dockRightId);
                ImGui::DockBuilderDockWindow("Scene", dockMainId);
                ImGui::DockBuilderDockWindow("Scripting", dockMainId);
                ImGui::DockBuilderDockWindow("Material", dockMainId);
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

            mSceneViewport.Resize(contentWidth, contentHeight);
            mSceneViewport.BindRenderTarget();
            mSceneViewport.ClearRenderTargetView();
            mSceneViewport.AttachViewport();

            if (mCurrentProject.mLoaded) {
                mGame.Resize(contentWidth, contentHeight);
                mGame.RenderFrame();

                if (mSelectedEntity.value() != 0) {
                    // Draw model for outline buffer
                    auto& state          = mGame.GetActiveScene()->GetState();
                    auto* modelComponent = state.GetComponentMutable<ModelComponent>(mSelectedEntity);
                    if (modelComponent) {}
                }
            }

            auto* srv = mSceneViewport.GetShaderResourceView().Get();
            ImGui::Image(ImTextureID((void*)srv), contentSize);
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    void EditorWindow::ScriptingView() {
        ImGui::Begin("Scripting");
        {}
        ImGui::End();
    }

    void EditorWindow::MaterialView() {
        ImGui::Begin("Material");
        {}
        ImGui::End();
    }

    void EditorWindow::EntitiesView() {
        ImGui::Begin("Entities");
        {}
        ImGui::End();
    }

    void EditorWindow::WorldSettingsView() {
        ImGui::Begin("World Settings");
        {}
        ImGui::End();
    }

    void EditorWindow::PropertiesView() {
        ImGui::Begin("Properties");
        {}
        ImGui::End();
    }

    void EditorWindow::AssetsView() {
        using namespace Filesystem;

        static AssetType selectedType = kAssetType_Invalid;
        ImGui::Begin("Assets");
        {
            const f32 widthAvail  = ImGui::GetContentRegionAvail().x;
            const f32 typeWidth   = 0.2f * widthAvail;
            const f32 assetsWidth = 0.8f * widthAvail;
            ImGui::BeginChild("AssetType", ImVec2(typeWidth, 0));
            {
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
                if (ImGui::Selectable("Texture", selectedType == kAssetType_Texture)) {
                    selectedType = kAssetType_Texture;
                }
                if (ImGui::Selectable("Mesh", selectedType == kAssetType_Mesh)) { selectedType = kAssetType_Mesh; }
                if (ImGui::Selectable("Audio", selectedType == kAssetType_Audio)) { selectedType = kAssetType_Audio; }
                if (ImGui::Selectable("Material", selectedType == kAssetType_Material)) {
                    selectedType = kAssetType_Material;
                }
                if (ImGui::Selectable("Scene", selectedType == kAssetType_Scene)) { selectedType = kAssetType_Scene; }
                if (ImGui::Selectable("Script", selectedType == kAssetType_Script)) {
                    selectedType = kAssetType_Script;
                }
                ImGui::PopStyleVar();
            }
            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::BeginChild("AssetFiles", ImVec2(assetsWidth, 0));
            {
                if (ImGui::BeginTable("AssetsTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY)) {
                    ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("Filename", ImGuiTableColumnFlags_WidthStretch);

                    ImGui::TableHeadersRow();

                    if (mCurrentProject.mLoaded) {
                        switch (selectedType) {
                            case kAssetType_Invalid:
                                break;
                            case kAssetType_Texture: {
                                for (const auto& [id, asset] : mEditorFiles.mAssetDescriptors) {
                                    if (asset.GetTypeFromId() == kAssetType_Texture) {
                                        ImGui::TableNextRow();
                                        ImGui::TableNextColumn();

                                        ImGui::Text(std::to_string(id).c_str());
                                        ImGui::TableNextColumn();
                                        ImGui::Text(asset.mFilename.c_str());
                                    }
                                }
                            } break;
                            case kAssetType_Mesh: {
                                for (const auto& [id, asset] : mEditorFiles.mAssetDescriptors) {
                                    if (asset.GetTypeFromId() == kAssetType_Mesh) {
                                        ImGui::TableNextRow();
                                        ImGui::TableNextColumn();

                                        ImGui::Text(std::to_string(id).c_str());
                                        ImGui::TableNextColumn();
                                        ImGui::Text(asset.mFilename.c_str());
                                    }
                                }
                            } break;
                            case kAssetType_Audio: {
                                for (const auto& [id, asset] : mEditorFiles.mAssetDescriptors) {
                                    if (asset.GetTypeFromId() == kAssetType_Audio) {
                                        ImGui::TableNextRow();
                                        ImGui::TableNextColumn();

                                        ImGui::Text(std::to_string(id).c_str());
                                        ImGui::TableNextColumn();
                                        ImGui::Text(asset.mFilename.c_str());
                                    }
                                }
                            } break;
                            case kAssetType_Material: {
                                for (const auto& [id, asset] : mEditorFiles.mAssetDescriptors) {
                                    if (asset.GetTypeFromId() == kAssetType_Material) {
                                        ImGui::TableNextRow();
                                        ImGui::TableNextColumn();

                                        ImGui::Text(std::to_string(id).c_str());
                                        ImGui::TableNextColumn();
                                        ImGui::Text(asset.mFilename.c_str());
                                    }
                                }
                            } break;
                            case kAssetType_Scene: {
                                for (const auto& [id, asset] : mEditorFiles.mAssetDescriptors) {
                                    if (asset.GetTypeFromId() == kAssetType_Scene) {
                                        ImGui::TableNextRow();
                                        ImGui::TableNextColumn();

                                        ImGui::Text(std::to_string(id).c_str());
                                        ImGui::TableNextColumn();
                                        ImGui::Text(asset.mFilename.c_str());
                                    }
                                }
                            } break;
                            case kAssetType_Script: {
                                for (const auto& [id, asset] : mEditorFiles.mAssetDescriptors) {
                                    if (asset.GetTypeFromId() == kAssetType_Script) {
                                        ImGui::TableNextRow();
                                        ImGui::TableNextColumn();

                                        ImGui::Text(std::to_string(id).c_str());
                                        ImGui::TableNextColumn();
                                        ImGui::Text(asset.mFilename.c_str());
                                    }
                                }
                            } break;
                            default:
                                break;
                        }
                    }

                    ImGui::EndTable();
                }
            }
            ImGui::EndChild();
        }
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

    void EditorWindow::UpdateWindowTitle(const str& title) const {
        const auto windowTitle = std::format("XEditor | {}", title);
        ::SetWindowTextA(mHwnd, windowTitle.c_str());
    }

    void EditorWindow::Render() {
        mWindowViewport->ClearAll();

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::PushFont(mFonts["display"]);

        //============================================================================================================//
        // MENU BAR
        //============================================================================================================//

        MainMenu();
        const f32 menuBarHeight = ImGui::GetFrameHeight();

        //============================================================================================================//
        // TOOL BAR
        //============================================================================================================//

        Toolbar(this, mTextureManager, menuBarHeight);

        //============================================================================================================//
        // EDITOR PANELS
        //============================================================================================================//

        SetupDockspace(menuBarHeight + kToolbarHeight);

        SceneView();
        ScriptingView();
        MaterialView();
        EntitiesView();
        WorldSettingsView();
        PropertiesView();
        AssetsView();
        EditorLogView();

        ImGui::PopFont();

        mWindowViewport->AttachViewport();
        mWindowViewport->BindRenderTarget();
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    LRESULT EditorWindow::MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) {
        if (ImGui_ImplWin32_WndProcHandler(mHwnd, msg, wParam, lParam)) return true;
        return Window::MessageHandler(msg, wParam, lParam);
    }

    void EditorWindow::OpenScene(const char* filename) {
        SceneDescriptor descriptor;
        SceneParser::Parse(filename, descriptor);
        mGame.TransitionScene(descriptor.mName);
    }

    void EditorWindow::TogglePlayMode() {
        if (!mGame.SceneValid()) return;

        if (mGameRunning) {
            // Reset camera back to editor camera
            mGame.GetActiveScene()->ResetToInitialState();
            mGame.GetActiveScene()->Update(0.0f);
            auto& sceneState      = mGame.GetActiveScene()->GetState();
            sceneState.MainCamera = mEditorCamera;

            mGameRunning = false;
        } else {
            // Reset camera back to scene camera
            mGame.GetActiveScene()->Update(0.0f);
            auto& sceneState      = mGame.GetActiveScene()->GetState();
            sceneState.MainCamera = mSceneCamera;

            mGameRunning = true;
        }
    }

    void EditorWindow::NewScene() {
        mGame.GetActiveScene()->Reset();
        mEntities.clear();
    }

    void EditorWindow::OpenProject(const char* filename) {
        using namespace Filesystem;

        mEditorFiles.mAssets.clear();
        mEditorFiles.mAssetDescriptors.clear();

        // Capture project metadata
        mCurrentProject.FromFile(filename);
        UpdateWindowTitle(mCurrentProject.mName);

        // Retrieve file listings for use in editor (scripts, materials, assets)
        const auto assetsDir = Path(mCurrentProject.mContentDirectory);

        for (const auto& entry : assetsDir.Entries()) {
            if (entry.IsFile() && entry.HasExtension() && entry.Extension() == "xasset") {
                AssetDescriptor desc;
                if (!desc.FromFile(entry.Str())) { X_LOG_ERROR("Failed to load asset {}", entry.Str()); }
                mEditorFiles.mAssetDescriptors[desc.mId] = desc;
            }
        }

        static bool hasLoadedProject {false};
        if (!hasLoadedProject) {
            mGame.Initialize(this, &mSceneViewport, Path(filename).Parent());
            hasLoadedProject = true;
        }
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
