// Author: Jake Rieger
// Created: 3/11/2025.
//

#include "XEditor.hpp"
#include "Res/resource.h"
#include "Controls.hpp"

#include "Common/FileDialogs.hpp"
#include "Common/WindowsHelpers.hpp"

#include "Engine/SceneParser.hpp"
#include "Engine/EngineCommon.hpp"

#include "XPak/AssetGenerator.hpp"

#include <Inter.h>
#include <JetBrainsMono.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <imgui_internal.h>
#include <yaml-cpp/yaml.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace x {
    static constexpr ImVec2 kViewportSize {1024, 576};  // Default viewport size
    static constexpr f32 kLabelWidth {140.0f};
    static constexpr i32 kNoSelection {-1};
    static constexpr u32 kAssetIconColumnCount {9};

    static EntityId sSelectedEntity {};
    static i32 sSelectedAsset {kNoSelection};

    // TODO: Consider making this a class
    namespace EditorState {
        static Float3 CameraPosition {};
        static Float3 CameraEye {};
        static f32 CameraFovY {};
        static f32 CameraNearZ {};
        static f32 CameraFarZ {};

        static Float3 TransformPosition {};
        static Float3 TransformRotation {};
        static Float3 TransformScale {};

        static bool ModelCastsShadows {false};
        static bool ModelReceivesShadows {false};

        static char CurrentSceneName[256] {0};
    }  // namespace EditorState

    static ImTextureID SrvAsTextureId(ID3D11ShaderResourceView* srv) {
        return RCAST<ImTextureID>(RCAST<void*>(srv));
    }

    static ImVec4 HexToImVec4(const str& hex, const f32 alpha = 1.0f) {
        // Ensure the string is the correct length
        if (hex.length() != 6) { throw std::invalid_argument("Hex color should be in the format 'RRGGBB'"); }

        ImVec4 color;
        const char red[3]   = {hex[0], hex[1], '\0'};
        const char green[3] = {hex[2], hex[3], '\0'};
        const char blue[3]  = {hex[4], hex[5], '\0'};

        const i32 r = strtol(red, nullptr, 16);
        const i32 g = strtol(green, nullptr, 16);
        const i32 b = strtol(blue, nullptr, 16);

        color.x = (f32)r / 255.0f;
        color.y = (f32)g / 255.0f;
        color.z = (f32)b / 255.0f;
        color.w = alpha;

        return color;
    }

    static ImVec4 GetLogEntryColor(const u32 severity) {
        switch (severity) {
            default:
            case X_LOG_SEVERITY_INFO:
                return HexToImVec4("b9b9b9");
            case X_LOG_SEVERITY_WARN:
                return HexToImVec4("ffe100");
            case X_LOG_SEVERITY_ERROR:
                return HexToImVec4("eb529e");
            case X_LOG_SEVERITY_FATAL:
                return ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
            case X_LOG_SEVERITY_DEBUG:
                return HexToImVec4("ebc388");
        }
    }

    bool EditorSession::LoadSession() {
        const auto sessionFile = Path(".session");
        if (sessionFile.Exists()) {
            YAML::Node session = YAML::LoadFile(sessionFile.Str());
            if (session["last_project"].IsDefined()) {
                mLastProjectPath = Path(session["last_project"].as<str>());
                return true;
            }
        }
        return false;
    }

    void EditorSession::SaveSession() const {
        const auto sessionFile = Path(".session");
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "last_project";
        out << YAML::Value << mLastProjectPath.Str();
        out << YAML::EndMap;
        if (!FileWriter::WriteAllText(sessionFile, out.c_str())) { X_LOG_ERROR("Failed to save editor session"); }
    }

    bool EditorTheme::LoadTheme(const str& theme) {
        const auto themeFile = Path("Themes") / (theme + ".yaml");
        if (themeFile.Exists()) {
            YAML::Node themeNode = YAML::LoadFile(themeFile.Str());

            mName             = themeNode["name"].as<str>();
            mWindowBackground = HexToImVec4(themeNode["windowBackground"].as<str>());
            mChildBackground  = HexToImVec4(themeNode["childBackground"].as<str>());
            mFrameBackground  = HexToImVec4(themeNode["frameBackground"].as<str>());
            mAltBackground    = HexToImVec4(themeNode["altBackground"].as<str>());
            mHeaderBackground = HexToImVec4(themeNode["headerBackground"].as<str>());
            mTextHighlight    = HexToImVec4(themeNode["textHighlight"].as<str>());
            mTextPrimary      = HexToImVec4(themeNode["textPrimary"].as<str>());
            mTextSecondary    = HexToImVec4(themeNode["textSecondary"].as<str>());
            mTextDisabled     = HexToImVec4(themeNode["textDisabled"].as<str>());
            mBorder           = HexToImVec4(themeNode["border"].as<str>());
            mError            = HexToImVec4(themeNode["error"].as<str>());
            mWarning          = HexToImVec4(themeNode["warning"].as<str>());
            mSuccess          = HexToImVec4(themeNode["success"].as<str>());
            mLink             = HexToImVec4(themeNode["link"].as<str>());
            mScrollbar        = HexToImVec4(themeNode["scrollbar"].as<str>());
            mPrimary          = HexToImVec4(themeNode["primary"].as<str>());
            mSecondary        = HexToImVec4(themeNode["secondary"].as<str>());
            mBorderRadius     = themeNode["borderRadius"].as<f32>(2.0f);
            mBorderWidth      = themeNode["borderWidth"].as<f32>(1.0f);

            return true;
        }
        return false;
    }

    void EditorTheme::SaveTheme() const {}

    void EditorTheme::Apply() const {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors    = style.Colors;

        style.WindowRounding   = mBorderRadius;
        style.FrameRounding    = mBorderRadius;
        style.WindowBorderSize = mBorderWidth;
        style.FrameBorderSize  = 0.f;
        style.TabRounding      = mBorderRadius;

        const auto windowBackground    = mWindowBackground;
        const auto childBackground     = mChildBackground;
        const auto frameBackground     = mFrameBackground;
        const auto secondaryBackground = mAltBackground;
        const auto headerBackground    = mHeaderBackground;
        const auto textHighlight       = mTextHighlight;
        const auto textPrimary         = mTextPrimary;
        const auto textSecondary       = mTextSecondary;
        const auto textDisabled        = mTextDisabled;
        const auto border              = mBorder;
        const auto error               = mError;
        const auto warning             = mWarning;
        const auto success             = mSuccess;
        const auto link                = mLink;
        const auto scrollbar           = mScrollbar;
        const auto primary             = mPrimary;
        const auto secondary           = mSecondary;

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

    bool EditorSettings::LoadSettings() {
        const auto settingsFile = Path("engine.yaml");
        if (settingsFile.Exists()) {
            YAML::Node settings = YAML::LoadFile(settingsFile.Str());
            mTheme              = settings["theme"].as<str>();
            return true;
        }
        return false;
    }

    void EditorSettings::SaveSettings() const {
        const auto settingsFile = Path("engine.yaml");
        YAML::Emitter out;
        out << YAML::BeginMap;
        {
            out << YAML::Key << "theme";
            out << YAML::Value << mTheme;
        }
        out << YAML::EndMap;
        FileWriter::WriteAllText(settingsFile, out.c_str());
    }

    void XEditor::OnInitialize() {
        SetWindowTitle("XEditor | Untitled");
        this->SetWindowIcon(APPICON);
        LoadEditorIcons();

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

        mWindowViewport->SetClearColor(0.05f, 0.05f, 0.05f, 1.0f);  // Editor background nearly black by default
        mSceneViewport.SetClearColor(Colors::Gray);                 // Viewport background grey by default
        // Resize to 1x1 initially so D3D creation code doesn't fail (0x0 invalid resource size)
        mSceneViewport.Resize(1, 1);

        if (mSession.LoadSession()) { LoadProject(mSession.mLastProjectPath.Str()); }
        if (!mSettings.LoadSettings()) { mSettings.SaveSettings(); }
        mTheme.LoadTheme(mSettings.mTheme);
        mTheme.Apply();

        X_LOG_WARN("Test warning log entry")
        X_LOG_ERROR("Test error log entry")
        X_LOG_DEBUG("Test debug log entry")
    }

    void XEditor::OnResize(u32 width, u32 height) {
        Window::OnResize(width, height);
    }

    void XEditor::OnShutdown() {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    void XEditor::OnUpdate() {
        // mGame.Update(false);
    }

    void XEditor::OnRender() {
        mWindowViewport->ClearAll();

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::PushFont(mFonts["display"]);

        MainMenu();
        const f32 menuBarHeight = ImGui::GetFrameHeight();

        SetupDockspace(menuBarHeight);

        ViewportView();
        SceneSettingsView();
        EntitiesView();
        EntitiesPropertiesView();
        AssetsView();
        LogView();
        AssetPreviewView();

        ImGui::PopFont();

        mWindowViewport->AttachViewport();
        mWindowViewport->BindRenderTarget();
        ImGui::Render();

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    LRESULT XEditor::MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) {
        if (ImGui_ImplWin32_WndProcHandler(mHwnd, msg, wParam, lParam)) return true;
        return Window::MessageHandler(msg, wParam, lParam);
    }

    void XEditor::SaveSceneAsModal() {
        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        if (ImGui::BeginPopupModal("Save Scene As", &mSaveSceneAsOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::SetNextItemWidth(408.0f);
            ImGui::InputText("##scene_name_as", EditorState::CurrentSceneName, sizeof(EditorState::CurrentSceneName));

            ImGui::Dummy(ImVec2(0, 2));

            if (ImGui::Button("OK", ImVec2(200, 0))) {
                if (EditorState::CurrentSceneName[0] != '\0' || strcmp(EditorState::CurrentSceneName, "") == 0) {
                    OnSaveScene(EditorState::CurrentSceneName);
                    mSaveSceneAsOpen = false;
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(200, 0))) {
                mSaveSceneAsOpen = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    struct ComponentOption {
        str mHeader;
        str mSubHeader;
        bool mAvailable {false};
    };

    void XEditor::AddComponentModel() {
        auto& state = mGame.GetActiveScene()->GetState();

        using ComponentMap             = unordered_map<str, str>;
        static ComponentMap components = {
          {"Model", "Renders a 3D model/mesh, making it visible in the scene"},
          {"Behavior", "Custom Lua script that defines unique behaviors and functionality"}};

        static str selectedComponent;
        static constexpr f32 itemHeight = 48.0f;

        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        i32 availableComponents {0};
        if (ImGui::BeginPopupModal("Add Component", &mAddComponentOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
            for (const auto& [name, description] : components) {
                bool available {true};

                if (name == "Model") {
                    if (state.HasComponent<ModelComponent>(sSelectedEntity)) { available = false; }
                } else if (name == "Behavior") {
                    if (state.HasComponent<BehaviorComponent>(sSelectedEntity)) { available = false; }
                }

                if (available) {
                    availableComponents++;
                    const auto id = "##" + name + "_component_option";
                    if (SelectableWithHeaders(id.c_str(),
                                              name.c_str(),
                                              description.c_str(),
                                              selectedComponent == name,
                                              true,
                                              ImVec2(0, itemHeight))) {
                        selectedComponent = name;
                    }
                    ImGui::Dummy(ImVec2(0, 2.f));
                }
            }

            if (availableComponents == 0) {
                const ImVec2 size = ImGui::GetContentRegionAvail();
                CenteredText("No components available", ImGui::GetCursorScreenPos(), ImVec2(size.x, 48));
            }

            // Buttons
            if (ImGui::Button("OK", ImVec2(240, 0))) {
                if (selectedComponent == "Model") {
                    // TODO: This will require a lot more work than simply adding the component since it's tied into
                    // the render system
                    // state.AddComponent<ModelComponent>(sSelectedEntity);
                } else if (selectedComponent == "Behavior") {
                    state.AddComponent<BehaviorComponent>(sSelectedEntity);
                }

                mAddComponentOpen = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(240, 0))) {
                mAddComponentOpen = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    void XEditor::OnOpenProject() {
        const auto filter = "Project (*.xproj)|*.xproj|";
        char filename[MAX_PATH];
        if (Platform::OpenFileDialog(mHwnd,
                                     GetInitialDirectory().CStr(),
                                     filter,
                                     "Open Project File",
                                     filename,
                                     MAX_PATH)) {
            LoadProject(filename);
        }
    }

    void XEditor::OnLoadScene(const str& selectedScene) {
        mGame.TransitionScene(selectedScene);

        auto& state                 = mGame.GetActiveScene()->GetState();
        auto& camera                = state.MainCamera;
        EditorState::CameraPosition = camera.GetPosition();
        EditorState::CameraEye      = camera.GetEye();
        EditorState::CameraFovY     = camera.GetFovY();
        EditorState::CameraNearZ    = camera.GetClipPlanes().first;
        EditorState::CameraFarZ     = camera.GetClipPlanes().second;
        // TODO: Remove the mSceneSettings member, its redundant
        std::strcpy(mSceneSettings.mName, selectedScene.c_str());
        std::strcpy(EditorState::CurrentSceneName, selectedScene.c_str());

        SetWindowTitle(std::format("XEditor | {}", selectedScene));

        sSelectedEntity = mGame.GetActiveScene()->GetState().GetEntities().begin()->first;
    }

    void XEditor::OnImportAsset() {
        const auto filter = "All Supported Asset Files "
                            "(*.dds;*.glb;*.obj;*.fbx;*.lua;*.material;*.scene;*.wav)|*.dds;*.glb;*.obj;*.fbx;*.lua;*."
                            "material;*.scene;*.wav|Texture Files (*.dds)|*.dds|Mesh Files "
                            "(*.glb;*.obj;*.fbx)|*.glb;*.obj;*.fbx|Script Files "
                            "(*.lua)|*.lua|Material Files (*.material)|*.material|Scene Files (*.scene)|*.scene|Audio "
                            "Files (*.wav)|*.wav|";
        char filename[MAX_PATH];
        Path initialDir = GetInitialDirectory();
        if (Platform::OpenFileDialog(mHwnd, GetInitialDirectory().CStr(), filter, "Import Asset", filename, MAX_PATH)) {
            auto assetFile            = Path(filename);
            const AssetType assetType = GetAssetTypeFromFile(assetFile);
            const auto rootContentDir = Path(mLoadedProject.mContentDirectory);
            Path contentDir;
            switch (assetType) {
                case kAssetType_Audio:
                    contentDir = rootContentDir / "Audio";
                    break;
                case kAssetType_Material:
                    contentDir = rootContentDir / "Materials";
                    break;
                case kAssetType_Mesh:
                    contentDir = rootContentDir / "Models";
                    break;
                case kAssetType_Scene:
                    contentDir = rootContentDir / "Scenes";
                    break;
                case kAssetType_Script:
                    contentDir = rootContentDir / "Scripts";
                    break;
                case kAssetType_Texture:
                    contentDir = rootContentDir / "Textures";
                    break;
                case kAssetType_Invalid:
                default:
                    X_LOG_ERROR("Invalid asset type for file '%s'", filename);
                    return;
            }

            if (!contentDir.Exists()) {
                if (!contentDir.Create()) {
                    X_LOG_ERROR("Failed to create directory '%s'", filename);
                    return;
                }
            }

            const auto copiedAssetFile = assetFile.Copy(contentDir);
            assert(copiedAssetFile.Exists());

            if (!AssetGenerator::GenerateAsset(copiedAssetFile, assetType, rootContentDir)) {
                X_LOG_ERROR("Failed to generate asset '%s'", filename);
                return;
            }

            AssetManager::LoadAssets(rootContentDir.Parent());
            ReloadAssetCache();
        }
    }

    void XEditor::ReloadAssetCache() {
        if (mLoadedProject.mLoaded && mGame.IsInitialized()) {
            mAssetDescriptors.clear();
            mAssetDescriptors = AssetManager::GetAssetDescriptors();
        }
    }

    void XEditor::SelectSceneModal() {
        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        if (ImGui::BeginPopupModal("Select Scene", &mSceneSelectorOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
            static str selectedScene;

            u32 index {0};
            for (const auto& [name, desc] : mGame.GetSceneMap()) {
                str id          = "##" + std::to_string(index) + "scene_select";  // ex. ##0_select_scene
                str description = desc.mDescription;
                if (description.length() > 50) { description = description.substr(0, 50) + "..."; }
                if (SelectableWithHeaders(id.c_str(),
                                          name.c_str(),
                                          description.c_str(),
                                          selectedScene == name,
                                          ImGuiSelectableFlags_None,
                                          ImVec2(0, 48))) {
                    selectedScene = name;
                }
                index++;
                ImGui::Dummy(ImVec2(0, 2.f));
            }

            // Buttons
            if (ImGui::Button("OK", ImVec2(200, 0))) {
                if (!selectedScene.empty()) { OnLoadScene(selectedScene); }

                mSceneSelectorOpen = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(200, 0))) {
                mSceneSelectorOpen = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    void XEditor::MainMenu() {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Open Project", "Ctrl+O")) { OnOpenProject(); }
                ImGui::Separator();
                if (ImGui::MenuItem("Open Scene", "Ctrl+Shift+O", false, mLoadedProject.mLoaded)) {
                    mSceneSelectorOpen = true;
                }
                if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
                    // const bool confirmed =
                    //   Platform::ShowAlert(mHwnd,
                    //                       "Save Scene",
                    //                       "You are about to overwrite the current scene. Do you wish to continue?",
                    //                       Platform::AlertSeverity::Question);
                    // if (confirmed == IDYES) {  }
                    OnSaveScene();
                }
                if (ImGui::MenuItem("Save Scene As", "Ctrl+Shift+S")) { mSaveSceneAsOpen = true; }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit", "Alt+F4")) { this->Quit(); }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit")) {
                if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
                if (ImGui::MenuItem("Redo", "Ctrl+Y")) {}
                ImGui::Separator();
                if (ImGui::MenuItem("Project Settings", "Shift+Alt+S")) {}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Assets")) {
                if (ImGui::MenuItem("Import Asset...")) { OnImportAsset(); }
                if (ImGui::MenuItem("Generate Pak File")) {}

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                if (ImGui::MenuItem("Reset Layout")) {
                    // Trigger first-time layout setup again
                    mDockspaceSetup = false;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
        ImGui::PopStyleVar();

        if (mSceneSelectorOpen) { ImGui::OpenPopup("Select Scene"); }
        SelectSceneModal();

        if (mSaveSceneAsOpen) { ImGui::OpenPopup("Save Scene As"); }
        SaveSceneAsModal();
    }

    void XEditor::SceneSettingsView() {
        ImGui::Begin("Scene");
        {
            if (mGame.IsInitialized() && mGame.GetActiveScene()->Loaded()) {
                auto& state = mGame.GetActiveScene()->GetState();

                const f32 width = ImGui::GetContentRegionAvail().x;
                ImGui::Text("Name: ");
                ImGui::SameLine(kLabelWidth);
                ImGui::SetNextItemWidth(width - kLabelWidth);
                ImGui::InputText("##scene_name", mSceneSettings.mName, sizeof(mSceneSettings.mName));

                ImGui::Spacing();
                ImGui::Spacing();

                auto& camera = state.MainCamera;
                if (ImGui::CollapsingHeader("World", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Text("Camera");
                    ImGui::Spacing();
                    ImGui::Text("Position:");
                    ImGui::SameLine(kLabelWidth);
                    ImGui::SetNextItemWidth(width - kLabelWidth);
                    ImGui::DragFloat3("##camera_pos", (f32*)&EditorState::CameraPosition, 0.01f);
                    camera.SetPosition(XMLoadFloat3(&EditorState::CameraPosition));

                    ImGui::Text("Eye:");
                    ImGui::SameLine(kLabelWidth);
                    ImGui::SetNextItemWidth(width - kLabelWidth);
                    ImGui::DragFloat3("##camera_eye", (f32*)&EditorState::CameraEye, 0.01f);
                    camera.SetEye(XMLoadFloat3(&EditorState::CameraEye));

                    ImGui::Text("FOV Y:");
                    ImGui::SameLine(kLabelWidth);
                    ImGui::SetNextItemWidth(width - kLabelWidth);
                    ImGui::InputFloat("##camera_fov", &EditorState::CameraFovY, 0.1f, 1.0f, "%.1f");
                    camera.SetFOV(EditorState::CameraFovY);

                    ImGui::Text("Near Z:");
                    ImGui::SameLine(kLabelWidth);
                    ImGui::SetNextItemWidth(width - kLabelWidth);
                    ImGui::InputFloat("##camera_nearz", &EditorState::CameraNearZ, 0.1f, 100.0f, "%.1f");

                    ImGui::Text("Far Z:");
                    ImGui::SameLine(kLabelWidth);
                    ImGui::SetNextItemWidth(width - kLabelWidth);
                    ImGui::InputFloat("##camera_farz", &EditorState::CameraFarZ, 0.1f, 100.0f, "%.1f");

                    camera.SetClipPlanes(EditorState::CameraNearZ, EditorState::CameraFarZ);

                    ImGui::Spacing();
                    ImGui::Spacing();

                    // Sun
                    auto& sun = state.Lights.mSun;
                    ImGui::Text("Sun");
                    ImGui::Spacing();

                    ImGui::Text("Enabled:");
                    ImGui::SameLine(kLabelWidth);
                    ImGui::Checkbox("##sun_enabled", (bool*)&sun.mEnabled);

                    ImGui::Text("Intensity:");
                    ImGui::SameLine(kLabelWidth);
                    ImGui::SetNextItemWidth(width - kLabelWidth);
                    ImGui::InputFloat("##sun_intensity", &sun.mIntensity, 0.1f, 1.0f, "%.1f");

                    ImGui::Text("Color:");
                    ImGui::SameLine(kLabelWidth);
                    ImGui::SetNextItemWidth(width - kLabelWidth);
                    ImGui::ColorEdit3("##sun_color", (f32*)&sun.mColor);

                    ImGui::Text("Direction:");
                    ImGui::SameLine(kLabelWidth);
                    ImGui::SetNextItemWidth(width - kLabelWidth);
                    ImGui::DragFloat3("##sun_direction", (f32*)&sun.mDirection, 0.01f);

                    ImGui::Text("Casts Shadows:");
                    ImGui::SameLine(kLabelWidth);
                    ImGui::SetNextItemWidth(width - kLabelWidth);
                    ImGui::Checkbox("##sun_casts_shadows", (bool*)&sun.mCastsShadows);
                }
            } else {
                ImGui::Text("No scene loaded. Go to \"File->Open Scene\" to load a scene.");
            }
        }
        ImGui::End();
    }

    void XEditor::EntitiesView() {
        ImGui::Begin("Entities");
        const ImVec2 windowSize = ImGui::GetContentRegionAvail();

        // Define sizes with adjustment for borders and scrollbar appearance
        constexpr f32 buttonHeight    = 24.0f;
        constexpr f32 buttonPadding   = 4.0f;
        constexpr f32 totalButtonArea = buttonHeight + (buttonPadding * 2);
        // Add a small adjustment factor to prevent scrollbar appearance
        constexpr f32 heightAdjustment = 4.0f;
        const f32 listHeight           = windowSize.y - totalButtonArea - heightAdjustment;

        ImGui::PushStyleColor(ImGuiCol_ChildBg, HexToImVec4("17171a"));
        ImGui::BeginChild("##entities_scroll_list", ImVec2(windowSize.x, listHeight), true);
        {
            if (mGame.IsInitialized() && mGame.GetActiveScene()->Loaded()) {
                for (auto& [id, name] : mGame.GetActiveScene()->GetState().GetEntities()) {
                    if (ImGui::Selectable(name.c_str(), id == sSelectedEntity)) {
                        sSelectedEntity = id;
                        std::strcpy(mEntityProperties.mName, name.c_str());
                    }
                }
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();

        ImGui::Dummy(ImVec2(0, buttonPadding));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(buttonPadding, buttonPadding));
        const ImVec2 remainingSpace = ImGui::GetContentRegionAvail();

        static char entityNameBuffer[256] {0};
        static bool openAddEntityPopup {false};
        if (ImGui::Button("Add Entity", ImVec2(remainingSpace.x, buttonHeight))) {
            memset(entityNameBuffer,
                   0,
                   sizeof(entityNameBuffer));  // probably unnecessary since we initialized with null
            ImGui::OpenPopup("Add New Entity");
            openAddEntityPopup = true;
        }
        ImGui::PopStyleVar();

        if (ImGui::BeginPopupModal("Add New Entity", &openAddEntityPopup, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Entity name:");
            ImGui::PushItemWidth(300.0f);
            const bool enterPressed = ImGui::InputText("##new_entity_name",
                                                       entityNameBuffer,
                                                       sizeof(entityNameBuffer),
                                                       ImGuiInputTextFlags_EnterReturnsTrue);
            ImGui::PopItemWidth();
            ImGui::Separator();
            if (ImGui::Button("OK", ImVec2(150, 0)) || enterPressed) {
                if (strlen(entityNameBuffer) > 0) {
                    ImGui::CloseCurrentPopup();
                    OnAddEntity(entityNameBuffer);
                }
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel", ImVec2(150, 0))) { ImGui::CloseCurrentPopup(); }

            ImGui::EndPopup();
        }

        ImGui::End();
    }

    void XEditor::EntitiesPropertiesView() {
        static bool selectAssetOpen {false};
        static AssetDescriptor selectedAsset {};

        ImGui::Begin("Properties");
        const ImVec2 size = ImGui::GetContentRegionAvail();
        {
            if (mGame.IsInitialized() && mGame.GetActiveScene()->Loaded() && sSelectedEntity.Valid()) {
                auto& state = mGame.GetActiveScene()->GetState();

                auto* transform = state.GetComponentMutable<TransformComponent>(sSelectedEntity);
                if (!transform) {
                    X_LOG_ERROR("Entity missing required Transform component")
                    return;
                }

                EditorState::TransformPosition = transform->GetPosition();
                EditorState::TransformRotation = transform->GetRotation();
                EditorState::TransformScale    = transform->GetScale();

                auto* model = state.GetComponentMutable<ModelComponent>(sSelectedEntity);
                if (model) {
                    EditorState::ModelCastsShadows    = model->GetCastsShadows();
                    EditorState::ModelReceivesShadows = model->GetReceiveShadows();
                }
                auto* behavior = state.GetComponentMutable<BehaviorComponent>(sSelectedEntity);

                if (ImGui::CollapsingHeader("General##properties", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Text("Name:");
                    ImGui::SameLine(kLabelWidth);
                    ImGui::SetNextItemWidth(size.x - kLabelWidth);
                    const bool enterPressed = ImGui::InputText("##entity_name_input",
                                                               mEntityProperties.mName,
                                                               sizeof(mEntityProperties.mName),
                                                               ImGuiInputTextFlags_EnterReturnsTrue);
                    if (enterPressed) { state.RenameEntity(sSelectedEntity, mEntityProperties.mName); }
                }

                if (ImGui::CollapsingHeader("Transform##properties", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Text("Position:");
                    ImGui::SameLine(kLabelWidth);
                    ImGui::SetNextItemWidth(size.x - kLabelWidth);
                    ImGui::DragFloat3("##entity_transform_position",
                                      (f32*)&EditorState::TransformPosition,
                                      0.01f,
                                      -FLT_MAX,
                                      FLT_MAX,
                                      "%.3f");
                    transform->SetPosition(EditorState::TransformPosition);

                    ImGui::Text("Rotation:");
                    ImGui::SameLine(kLabelWidth);
                    ImGui::SetNextItemWidth(size.x - kLabelWidth);
                    ImGui::DragFloat3("##entity_transform_rotation",
                                      (f32*)&EditorState::TransformRotation,
                                      0.01f,
                                      -FLT_MAX,
                                      FLT_MAX,
                                      "%.3f");
                    transform->SetRotation(EditorState::TransformRotation);

                    ImGui::Text("Scale:");
                    ImGui::SameLine(kLabelWidth);
                    ImGui::SetNextItemWidth(size.x - kLabelWidth);
                    ImGui::DragFloat3("##entity_transform_scale",
                                      (f32*)&EditorState::TransformScale,
                                      0.01f,
                                      -FLT_MAX,
                                      FLT_MAX,
                                      "%.3f");
                    transform->SetScale(EditorState::TransformScale);

                    transform->Update();
                }

                if (model) {
                    if (ImGui::CollapsingHeader("Model##properties", ImGuiTreeNodeFlags_DefaultOpen)) {
                        ImGui::Text("Mesh (Asset):");
                        ImGui::SameLine(kLabelWidth);

                        const auto& modelAsset =
                          std::ranges::find_if(mAssetDescriptors, [&model](const AssetDescriptor& asset) {
                              return asset.mId == model->GetModelId();
                          });
                        if (modelAsset == mAssetDescriptors.end()) {
                            X_LOG_ERROR("No model asset found for entity '%s'",
                                        state.GetEntities().at(sSelectedEntity).c_str());
                        } else {
                            // const str modelText = std::format("{} (Change)", Path(modelAsset->mFilename).Filename());
                            // if (ImGui::Button(modelText.c_str(), ImVec2(size.x - kLabelWidth, 0))) {
                            //     selectAssetOpen = true;
                            //     if (selectedAsset.mId != 0) model->SetModelId(selectedAsset.mId);
                            //     // TODO: Handle actually switching the asset on the component
                            // }
                            static char buffer[256] {0};
                            const auto currentValue = Path(modelAsset->mFilename).Filename();
                            std::strcpy(buffer, currentValue.c_str());
                            if (AssetDropTarget("##model_drop_target", buffer, sizeof(buffer), "[-]")) {}
                        }

                        ImGui::Text("Material (Asset):");
                        ImGui::SameLine(kLabelWidth);

                        const auto& materialAsset =
                          std::ranges::find_if(mAssetDescriptors, [&model](const AssetDescriptor& asset) {
                              return asset.mId == model->GetMaterialId();
                          });
                        if (materialAsset == mAssetDescriptors.end()) {
                            X_LOG_ERROR("No material asset found for entity '%s'",
                                        state.GetEntities().at(sSelectedEntity).c_str());
                        } else {
                            const str materialText =
                              std::format("{} (Change)", Path(materialAsset->mFilename).Filename());
                            ImGui::Button(materialText.c_str(), ImVec2(size.x - kLabelWidth, 0));
                        }

                        ImGui::Text("Casts Shadows:");
                        ImGui::SameLine(kLabelWidth);
                        ImGui::SetNextItemWidth(size.x - kLabelWidth);
                        ImGui::Checkbox("##entity_model_casts_shadows", &EditorState::ModelCastsShadows);
                        model->SetCastsShadows(EditorState::ModelCastsShadows);

                        ImGui::Text("Receives Shadows:");
                        ImGui::SameLine(kLabelWidth);
                        ImGui::SetNextItemWidth(size.x - kLabelWidth);
                        ImGui::Checkbox("##entity_model_receives_shadows", &EditorState::ModelReceivesShadows);
                        model->SetReceiveShadows(EditorState::ModelReceivesShadows);
                    }
                }

                if (behavior) {
                    if (ImGui::CollapsingHeader("Behavior##properties", ImGuiTreeNodeFlags_DefaultOpen)) {
                        ImGui::Text("Script (Asset):");
                        ImGui::SameLine(kLabelWidth);
                        ImGui::SetNextItemWidth(size.x - kLabelWidth);

                        const auto& scriptAsset =
                          std::ranges::find_if(mAssetDescriptors, [&behavior](const AssetDescriptor& asset) {
                              return asset.mId == behavior->GetScriptId();
                          });
                        if (scriptAsset != mAssetDescriptors.end()) {
                            const str scriptText = std::format("{} (Change)", Path(scriptAsset->mFilename).Filename());
                            ImGui::Button(scriptText.c_str(), ImVec2(size.x - kLabelWidth, 0));
                        }
                    }
                }
            }

            ImGui::Dummy(ImVec2(0, 2.f));
            ImGui::Separator();
            ImGui::Dummy(ImVec2(0, 2.f));

            if (ImGui::Button("Add Component##button", ImVec2(size.x, 0))) { mAddComponentOpen = true; }
        }
        ImGui::End();

        if (selectAssetOpen) { ImGui::OpenPopup("Select Asset"); }
        if (mAddComponentOpen) { ImGui::OpenPopup("Add Component"); }

        if (ImGui::BeginPopupModal("Select Asset", &selectAssetOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Dummy(ImVec2(380, 500));
            ImGui::EndPopup();
        }

        AddComponentModel();
    }

    void XEditor::ViewportView() {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Viewport");
        {
            const ImVec2 contentSize = ImGui::GetContentRegionAvail();
            const auto contentWidth  = CAST<u32>(contentSize.x);
            const auto contentHeight = CAST<u32>(contentSize.y);

            mSceneViewport.Resize(contentWidth, contentHeight);
            mSceneViewport.BindRenderTarget();
            mSceneViewport.ClearAll();
            mSceneViewport.AttachViewport();

            // Render current scene
            if (mLoadedProject.mLoaded && mGame.IsInitialized()) {
                mGame.Resize(contentWidth, contentHeight);
                mGame.RenderFrame();
            }

            auto* srv = mSceneViewport.GetShaderResourceView().Get();
            ImGui::Image(SrvAsTextureId(srv), contentSize);
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    void XEditor::OnSelectedMeshAsset(const AssetDescriptor& descriptor) {
        if (mEditorResources.LoadResource<Model>(descriptor.mId)) {
            const std::optional<ResourceHandle<Model>> modelResource =
              mEditorResources.FetchResource<Model>(descriptor.mId);
            if (modelResource.has_value()) {
                auto model = make_unique<ModelComponent>();
                model->SetModelHandle(*modelResource).SetReceiveShadows(false).SetCastsShadows(false);
                mMeshPreviewer.SetModel(std::move(model));
            }
        } else {
            X_LOG_ERROR("Failed to load mesh asset");
        }
    }

    void XEditor::AssetsView() {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Assets");
        {
            // Assets don't get loaded until the game is initialized (which happens when the project is loaded, but not
            // right away)
            if (mLoadedProject.mLoaded && mProjectRoot.Exists() && mGame.IsInitialized()) {
                // Setup for grid layout
                const auto& assets = mAssetDescriptors;

                constexpr f32 fullWidth = 1920.0f;
                const auto currentWidth = GetWidth();  // Get current client window width, TODO: Change this to the max
                // available width of the assets panel itself
                const f32 columnsCount = std::floorf((kAssetIconColumnCount * currentWidth) / fullWidth);
                // I could calculate this as the floor of the result of the proportion MAX_WIDTH/8  = CURR_WIDTH/x

                const auto& style        = ImGui::GetStyle();
                const f32 windowWidth    = ImGui::GetContentRegionAvail().x;
                const f32 scrollbarWidth = style.ScrollbarSize;
                const f32 availableWidth = windowWidth - scrollbarWidth - style.CellPadding.x - style.FramePadding.x -
                                           style.WindowPadding.x - 28.f;
                const f32 cellWidth =
                  (availableWidth - ImGui::GetStyle().ItemSpacing.x * (columnsCount - 1)) / columnsCount;
                const f32 thumbnailSize = cellWidth * 0.9f;                    // 90% of cell width for the image
                const f32 padding       = (cellWidth - thumbnailSize) / 2.0f;  // Equal padding on both sides

                ImGui::Dummy(ImVec2(0, padding * 2));

                ImGui::PushStyleColor(ImGuiCol_ChildBg, HexToImVec4("17171a"));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));
                if (ImGui::BeginChild("##GridScrollRegion", ImVec2(0, 0), false)) {
                    // Calculate number of rows needed
                    i32 itemCount = CAST<i32>(assets.size());
                    i32 rowCount  = CAST<i32>((itemCount + columnsCount - 1) / columnsCount);  // Ceiling division

                    // For each row
                    for (i32 row = 0; row < rowCount; row++) {
                        // Row container
                        ImGui::BeginGroup();

                        // For each column in the row
                        for (i32 col = 0; col < columnsCount; col++) {
                            i32 itemIndex = CAST<i32>(row * columnsCount + col);

                            // Break if we've displayed all items
                            if (itemIndex >= itemCount) break;

                            // Start new item (except for first item in row)
                            if (col > 0) ImGui::SameLine();

                            const AssetDescriptor& asset = assets[itemIndex];
                            ImGui::PushID(itemIndex);

                            // Begin a group for this cell
                            ImGui::BeginGroup();

                            // Create a selectable that fits the cell size
                            ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));
                            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
                            if (ImGui::Selectable("##cell",
                                                  sSelectedAsset == itemIndex,
                                                  0,
                                                  ImVec2(cellWidth, thumbnailSize + 25))) {
                                // Handle selection
                                sSelectedAsset  = itemIndex;
                                auto descriptor = mAssetDescriptors.at(sSelectedAsset);
                                if (descriptor.GetTypeFromId() == kAssetType_Mesh) { OnSelectedMeshAsset(descriptor); }
                            }
                            ImGui::PopStyleVar(2);

                            // Calculate image position (centered in the cell)
                            f32 imageStartX = ImGui::GetItemRectMin().x + (padding * 1.5f);
                            f32 imageStartY = ImGui::GetItemRectMin().y + (padding);  // Small top margin

                            // Truncate asset thumbnail text if necessary
                            str itemName = Path(asset.mFilename).Filename();
                            if (itemName.length() > 8) { itemName = itemName.substr(0, 10) + "..."; }

                            if (asset.GetTypeFromId() == kAssetType_Texture) {
                                auto thumbnail = mTextureManager.GetTexture(std::to_string(asset.mId));
                                assert(thumbnail.has_value());

                                ImGui::GetWindowDrawList()->AddImage(
                                  SrvAsTextureId(thumbnail->mShaderResourceView.Get()),
                                  ImVec2(imageStartX, imageStartY),
                                  ImVec2(imageStartX + thumbnailSize, imageStartY + thumbnailSize));
                            } else {
                                ID3D11ShaderResourceView* iconSrv {nullptr};

                                switch (asset.GetTypeFromId()) {
                                    case kAssetType_Audio: {
                                        auto icon = mTextureManager.GetTexture("AudioIcon");
                                        assert(icon.has_value());
                                        iconSrv = icon->mShaderResourceView.Get();
                                    } break;
                                    case kAssetType_Material: {
                                        auto icon = mTextureManager.GetTexture("MaterialIcon");
                                        assert(icon.has_value());
                                        iconSrv = icon->mShaderResourceView.Get();
                                    } break;
                                    case kAssetType_Mesh: {
                                        auto icon = mTextureManager.GetTexture("MeshIcon");
                                        assert(icon.has_value());
                                        iconSrv = icon->mShaderResourceView.Get();
                                    } break;
                                    case kAssetType_Scene: {
                                        auto icon = mTextureManager.GetTexture("SceneIcon");
                                        assert(icon.has_value());
                                        iconSrv = icon->mShaderResourceView.Get();
                                    } break;
                                    case kAssetType_Script: {
                                        auto icon = mTextureManager.GetTexture("ScriptIcon");
                                        assert(icon.has_value());
                                        iconSrv = icon->mShaderResourceView.Get();
                                    } break;
                                    default:
                                        break;
                                }

                                ImGui::GetWindowDrawList()->AddImage(
                                  SrvAsTextureId(iconSrv),
                                  ImVec2(imageStartX, imageStartY),
                                  ImVec2(imageStartX + thumbnailSize, imageStartY + thumbnailSize));
                            }

                            // Add text below the image (centered)
                            f32 textWidth  = ImGui::CalcTextSize(itemName.c_str()).x;
                            f32 textStartX = imageStartX + (thumbnailSize - textWidth) * 0.5f;
                            f32 textStartY = imageStartY + thumbnailSize + 4;  // 4 pixels below the image

                            ImGui::GetWindowDrawList()->AddText(ImVec2(textStartX, textStartY),
                                                                ImGui::GetColorU32(ImGuiCol_Text),
                                                                itemName.c_str());

                            // Add hover effect
                            if (ImGui::IsItemHovered()) {
                                ImGui::BeginTooltip();
                                ImGui::Text("%s", asset.mFilename.c_str());
                                ImGui::EndTooltip();
                            }

                            ImGui::EndGroup();
                            ImGui::PopID();
                        }

                        ImGui::EndGroup();
                    }
                }
                ImGui::EndChild();
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();
            }
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    void XEditor::LogView() {
        static bool showInfo {false};
        static bool showWarn {true};
        static bool showError {true};
        static bool showFatal {true};
        static bool showDebug {true};
        static bool autoScroll {true};

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

        ImGui::Begin("Log");
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

            ImGui::PushStyleColor(ImGuiCol_ChildBg, HexToImVec4("17171a"));
            ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

            // Display logs
            std::lock_guard<std::mutex> lock(logger.GetBufferMutex());
            size_t startIndex =
              (logger.GetCurrentEntry() - logger.GetTotalEntries() + logger.kMaxEntries) % logger.kMaxEntries;
            for (size_t i = 0; i < logger.GetTotalEntries(); i++) {
                const size_t index = (startIndex + i) % logger.kMaxEntries;
                const auto& entry  = logger.GetEntries()[index];

                // Apply filters
                if (!ShouldShowSeverity(entry.severity)) continue;

                ImGui::PushStyleColor(ImGuiCol_Text, GetLogEntryColor(entry.severity));
                ImGui::TextUnformatted(std::format("[{}] {}", entry.timestamp, entry.message).c_str());
                ImGui::PopStyleColor();
            }

            // Auto-scroll to bottom
            if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) { ImGui::SetScrollHereY(1.0f); }

            ImGui::Dummy(ImVec2(0, 4));

            ImGui::EndChild();
            ImGui::PopStyleColor();
        }
        ImGui::End();
    }

    void XEditor::AssetPreviewView() {
        ImGui::Begin("Asset Preview", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        {
            if (sSelectedAsset != kNoSelection) {
                const auto& asset = mAssetDescriptors.at(sSelectedAsset);
                auto assetTypeStr = asset.GetTypeString();
                assetTypeStr[0]   = std::toupper(assetTypeStr[0]);

                ImGui::Text("Source: %s", asset.mFilename.c_str());
                ImGui::Text("ID: %llu", asset.mId);
                ImGui::Text("Type: %s", assetTypeStr.c_str());
                ImGui::Separator();
                ImGui::Separator();

                const AssetType assetType = asset.GetTypeFromId();
                if (assetType == kAssetType_Texture) {
                    const auto preview =
                      SrvAsTextureId(mTextureManager.GetTexture(std::to_string(asset.mId))->mShaderResourceView.Get());
                    ImVec2 regionSize = ImGui::GetContentRegionAvail();
                    regionSize.y      = regionSize.x;
                    ImGui::Image(preview, regionSize);
                } else if (assetType == kAssetType_Material) {
                } else if (assetType == kAssetType_Scene) {
                    ImGui::Text("No preview available.");
                } else if (assetType == kAssetType_Mesh) {
                    ImVec2 regionSize = ImGui::GetContentRegionAvail();
                    regionSize.y      = regionSize.x;
                    auto* srv         = mMeshPreviewer.Render(regionSize);
                    ImGui::Image(SrvAsTextureId(srv), regionSize);
                } else if (assetType == kAssetType_Script) {
                }
            }
        }
        ImGui::End();
    }

    void XEditor::GenerateAssetThumbnails() {
        const auto& assets = mAssetDescriptors;
        for (const auto& asset : assets) {
            const auto type = asset.GetTypeFromId();
            switch (type) {
                // Texture thumbnails
                case kAssetType_Texture: {
                    auto textureFile      = Path(mLoadedProject.mContentDirectory) / asset.mFilename;
                    const auto loadResult = mTextureManager.LoadFromDDSFile(textureFile, std::to_string(asset.mId));
                    if (!loadResult) {
                        Platform::ShowAlert(
                          mHwnd,
                          "Error loading texture",
                          ("Failed to load texture asset with id " + std::to_string(asset.mId)).c_str(),
                          Platform::AlertSeverity::Error);
                    }
                } break;
                // Descriptors just use icon files
                // TODO: Generate thumbnails for the rest of these
                // case kAssetType_Scene: {
                // } break;
                // case kAssetType_Material: {
                // } break;
                // case kAssetType_Script: {
                // } break;
                // case kAssetType_Mesh: {
                // } break;
                // case kAssetType_Audio: {
                // } break;
                default:
                    break;
            }
        }
    }

    void XEditor::LoadProject(const str& filename) {
        if (!mLoadedProject.FromFile(filename)) {
            Platform::ShowAlert(mHwnd,
                                "Error loading project",
                                "An error occurred when parsing the selected project file.",
                                Platform::AlertSeverity::Error);
            return;
        }

        mEditorResources.Clear();
        mProjectRoot              = Path(filename).Parent();
        mSession.mLastProjectPath = Path(filename);
        mSession.SaveSession();
        mGame.Initialize(this, &mSceneViewport, mProjectRoot);
        ReloadAssetCache();

        // TODO: Do this asynchronously
        GenerateAssetThumbnails();

        // Load startup scene
        OnLoadScene(mLoadedProject.mStartupScene);
    }

    void XEditor::OnSaveScene(const char* name) {
        const auto* scene = mGame.GetActiveScene();
        SceneDescriptor descriptor;
        SceneParser::StateToDescriptor(scene->GetState(), descriptor, name == nullptr ? scene->GetName() : name);
        if (descriptor.IsValid()) {
            if (name == nullptr) {
                SceneParser::WriteToFile(descriptor, mLoadedScenePath.Str());
            } else {
                const str sceneFileName = str(name) + ".scene";
                const auto scenePath    = Path(mLoadedProject.mContentDirectory) / "Scenes" / sceneFileName;
                SceneParser::WriteToFile(descriptor, scenePath.Str());
                AssetGenerator::GenerateAsset(scenePath, kAssetType_Scene, Path(mLoadedProject.mContentDirectory));
            }

            ReloadAssetCache();
            mGame.ReloadSceneCache();
            OnLoadScene(name);
        } else {
            Platform::ShowAlert(mHwnd,
                                "Error saving scene",
                                "Unable to parse scene state to descriptor.",
                                Platform::AlertSeverity::Error);
        }
    }

    void XEditor::OnAddEntity(const str& name) const {
        auto& state              = mGame.GetActiveScene()->GetState();
        const EntityId newEntity = state.CreateEntity(name);
        if (newEntity.Valid()) { state.AddComponent<TransformComponent>(newEntity); }
    }

    Path XEditor::GetInitialDirectory() const {
        if (mLoadedProject.mLoaded) {
            return Path(mLoadedProject.mContentDirectory).Parent();
        } else {
            return Platform::GetPlatformDirectory(Platform::kPlatformDir_Documents);
        }
    }

    AssetType XEditor::GetAssetTypeFromFile(const Path& path) {
        const auto ext = path.Extension();

        if (ext == "dds") {
            return kAssetType_Texture;
        } else if (ext == "glb" || ext == "obj" || ext == "fbx") {
            return kAssetType_Mesh;
        } else if (ext == "lua") {
            return kAssetType_Script;
        } else if (ext == "material") {
            return kAssetType_Material;
        } else if (ext == "scene") {
            return kAssetType_Scene;
        } else if (ext == "wav") {
            return kAssetType_Audio;
        }

        return kAssetType_Invalid;
    }

    void XEditor::SetupDockspace(const f32 yOffset) {
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

            if (!mDockspaceSetup) {
                mDockspaceSetup = true;

                ImGui::DockBuilderRemoveNode(dockspaceId);
                ImGui::DockBuilderAddNode(dockspaceId, flags | ImGuiDockNodeFlags_DockSpace);
                ImGui::DockBuilderSetNodeSize(dockspaceId, ImGui::GetWindowSize());

                ImGuiID dockMainId = dockspaceId;
                ImGuiID dockRightId =
                  ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Right, 0.24f, nullptr, &dockMainId);
                ImGuiID dockLeftId =
                  ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Left, 0.28f, nullptr, &dockMainId);
                ImGuiID dockBottomId =
                  ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Down, 0.35f, nullptr, &dockMainId);
                ImGuiID dockLeftBottomId =
                  ImGui::DockBuilderSplitNode(dockLeftId, ImGuiDir_Down, 0.5f, nullptr, &dockLeftId);
                ImGuiID dockRightBottomId =
                  ImGui::DockBuilderSplitNode(dockRightId, ImGuiDir_Down, 0.5f, nullptr, &dockRightId);
                ImGuiID dockBottomLeftId =
                  ImGui::DockBuilderSplitNode(dockBottomId, ImGuiDir_Left, 0.5f, nullptr, &dockBottomId);

                ImGui::DockBuilderDockWindow("Scene", dockLeftId);
                ImGui::DockBuilderDockWindow("Entities", dockLeftBottomId);
                ImGui::DockBuilderDockWindow("Properties", dockRightId);
                ImGui::DockBuilderDockWindow("Viewport", dockMainId);
                ImGui::DockBuilderDockWindow("Assets", dockBottomId);
                ImGui::DockBuilderDockWindow("Log", dockBottomId);
                ImGui::DockBuilderDockWindow("Asset Preview", dockRightBottomId);

                ImGui::DockBuilderFinish(imguiViewport->ID);
            }

            ImGui::End();
        }

        ImGui::PopStyleVar(3);
    }

#pragma region Embedded Icon Includes
#include "AssetBrowserIcons.h"
#include "EditorIcons.h"
#pragma endregion

    bool XEditor::LoadEditorIcons() {
        // Asset Browser Icons
        auto result = mTextureManager.LoadFromMemory(AUDIOICON_BYTES, 128, 128, 4, "AudioIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load Audio icon");
            return false;
        }
        result = mTextureManager.LoadFromMemory(MATERIALICON_BYTES, 128, 128, 4, "MaterialIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load Material icon");
            return false;
        }
        result = mTextureManager.LoadFromMemory(MESHICON_BYTES, 128, 128, 4, "MeshIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load Mesh icon");
            return false;
        }
        result = mTextureManager.LoadFromMemory(SCENEICON_BYTES, 128, 128, 4, "SceneIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load Scene icon");
            return false;
        }
        result = mTextureManager.LoadFromMemory(SCRIPTICON_BYTES, 128, 128, 4, "ScriptIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load Script icon");
            return false;
        }
        result = mTextureManager.LoadFromMemory(FOLDERICON_BYTES, 128, 128, 4, "FolderIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load Folder icon");
            return false;
        }
        result = mTextureManager.LoadFromMemory(FOLDERICONEMPTY_BYTES, 128, 128, 4, "FolderEmptyIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load Folder icon");
            return false;
        }

        // Toolbar / Editor Icons
        result = mTextureManager.LoadFromMemory(MOVEICON_BYTES, 24, 24, 4, "MoveIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load Move icon");
            return false;
        }
        result = mTextureManager.LoadFromMemory(PAUSEICON_BYTES, 24, 24, 4, "PauseIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load Pause icon");
            return false;
        }
        result = mTextureManager.LoadFromMemory(PLAYICON_BYTES, 24, 24, 4, "PlayIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load Play icon");
            return false;
        }
        result = mTextureManager.LoadFromMemory(REDOICON_BYTES, 24, 24, 4, "RedoIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load Redo icon");
            return false;
        }
        result = mTextureManager.LoadFromMemory(UNDOICON_BYTES, 24, 24, 4, "UndoIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load Undo icon");
            return false;
        }
        result = mTextureManager.LoadFromMemory(ROTATEICON_BYTES, 24, 24, 4, "RotateIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load Rotate icon");
            return false;
        }
        result = mTextureManager.LoadFromMemory(SCALEICON_BYTES, 24, 24, 4, "ScaleIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load Scale icon");
            return false;
        }
        result = mTextureManager.LoadFromMemory(SELECTICON_BYTES, 24, 24, 4, "SelectIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load Select icon");
            return false;
        }
        result = mTextureManager.LoadFromMemory(SEPARATORICON_BYTES, 24, 24, 4, "SeparatorIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load Separator icon");
            return false;
        }
        result = mTextureManager.LoadFromMemory(SETTINGSICON_BYTES, 24, 24, 4, "SettingsIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load Settings icon");
            return false;
        }
        result = mTextureManager.LoadFromMemory(SNAP_TO_GRIDICON_BYTES, 24, 24, 4, "SnapToGridIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load SnapToGrid icon");
            return false;
        }
        result = mTextureManager.LoadFromMemory(STOPICON_BYTES, 24, 24, 4, "StopIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load Stop icon");
            return false;
        }

        return true;
    }
}  // namespace x