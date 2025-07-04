// Author: Jake Rieger
// Created: 3/11/2025.
//
// This file contains the bulk of the relevant code for the editor. If your editor/IDE supports regions, code folding
// should be enabled for each section of the file.
//
// All assets used by the editor (icons, fonts, etc.) are embedded in the app itself. Header files ending in .h instead
// of .hpp contain byte arrays for the asset(s).
//
// You can Ctrl+F with "View_" or "Modal_" to match functions for those UI elements. Event handlers for UI input actions
// begin with "On", i.e. "OnSaveProject", and can be found the way.

// Vendor includes
#include <Inter.h>
#include <JetBrainsMono.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <imgui_internal.h>
#include <yaml-cpp/yaml.h>

// Local includes
#include "XEditor.hpp"
#include "Res/resource.h"
#include "Controls.hpp"
#include "ImGuiHelpers.hpp"
#include "Common/FileDialogs.hpp"
#include "Common/WindowsHelpers.hpp"
#include "Engine/SceneParser.hpp"
#include "Engine/EngineCommon.hpp"
#include "Tools/XPak/AssetGenerator.hpp"

// Embedded resources
#include "AssetBrowserIcons.h"
#include "ToolbarIcons.h"
#include "Logos.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace x {
#pragma region Global Constants
    static constexpr f32 kLabelWidth {140.0f};
    static constexpr i32 kNoSelection {-1};
    static constexpr u32 kAssetIconColumnCount {11};
    static constexpr f32 kToolbarHeight {36.0f};
    static constexpr f32 kStatusBarHeight {24.0f};
#pragma endregion

#pragma region UI State
    static EntityId sSelectedEntity {};
    static i32 sSelectedAsset {kNoSelection};

    // TODO: Consider making this a class
    namespace EditorState {
        static Float3 TransformPosition {};
        static Float3 TransformRotation {};
        static Float3 TransformScale {};

        static bool ModelCastsShadows {false};
        static bool ModelReceivesShadows {false};

        static char CurrentSceneName[256] {0};
    }  // namespace EditorState
#pragma endregion

#pragma region Helpers
    static ImVec4 GetLogEntryColor(const u32 severity) {
        switch (severity) {
            default:
            case X_LOG_SEVERITY_INFO:
                return Colors::White.WithAlpha(0.45f).ToImVec4();
            case X_LOG_SEVERITY_WARN:
                return Color("#ffe100").ToImVec4();
            case X_LOG_SEVERITY_ERROR:
                return Color("#eb529e").ToImVec4();
            case X_LOG_SEVERITY_FATAL:
                return {1.0f, 0.0f, 0.0f, 1.0f};
            case X_LOG_SEVERITY_DEBUG:
                return Color("#ebc388").ToImVec4();
        }
    }
#pragma endregion

#pragma region EditorSession
    bool EditorSession::LoadSession() {
        const auto sessionFile = Path::Current() / "session.yaml";
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
        const auto sessionFile = Path::Current() / "session.yaml";
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "last_project";
        out << YAML::Value << mLastProjectPath.Str();
        out << YAML::EndMap;
        if (!FileWriter::WriteText(sessionFile, out.c_str())) { X_LOG_ERROR("Failed to save editor session"); }
    }
#pragma endregion

#pragma region EditorTheme
    bool EditorTheme::LoadTheme(const str& theme) {
        const auto themeFile = Path::Current() / "Themes" / (theme + ".yaml");
        if (themeFile.Exists()) {
            YAML::Node themeNode = YAML::LoadFile(themeFile.Str());

            mName             = themeNode["name"].as<str>();
            mWindowBackground = Color(themeNode["windowBackground"].as<str>());
            mMenuBackground   = Color(themeNode["menuBackground"].as<str>());
            mTabHeader        = Color(themeNode["tabHeader"].as<str>());
            mPanelBackground  = Color(themeNode["panelBackground"].as<str>());
            mButtonBackground = Color(themeNode["buttonBackground"].as<str>());
            mInputBackground  = Color(themeNode["inputBackground"].as<str>());
            mHeaderBackground = Color(themeNode["headerBackground"].as<str>());
            mTextHighlight    = Color(themeNode["textHighlight"].as<str>());
            mTextPrimary      = Color(themeNode["textPrimary"].as<str>());
            mTextSecondary    = Color(themeNode["textSecondary"].as<str>());
            mSelected         = Color(themeNode["selected"].as<str>());
            mIcon             = Color(themeNode["icon"].as<str>());
            mBorder           = Color(themeNode["border"].as<str>());
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

        colors[ImGuiCol_BorderShadow]          = ImVec4(0.f, 0.f, 0.f, 0.f);
        colors[ImGuiCol_Border]                = mBorder.ToImVec4();
        colors[ImGuiCol_ButtonActive]          = mButtonBackground.WithAlpha(0.67f).ToImVec4();
        colors[ImGuiCol_ButtonHovered]         = mButtonBackground.WithAlpha(0.8f).ToImVec4();
        colors[ImGuiCol_Button]                = mButtonBackground.ToImVec4();
        colors[ImGuiCol_CheckMark]             = mIcon.ToImVec4();
        colors[ImGuiCol_ChildBg]               = mPanelBackground.ToImVec4();
        colors[ImGuiCol_DockingPreview]        = mSelected.ToImVec4();
        colors[ImGuiCol_DragDropTarget]        = mSelected.ToImVec4();
        colors[ImGuiCol_FrameBgActive]         = mInputBackground.Brightness(0.4f).ToImVec4();
        colors[ImGuiCol_FrameBgHovered]        = mInputBackground.Brightness(0.7f).ToImVec4();
        colors[ImGuiCol_FrameBg]               = mInputBackground.ToImVec4();
        colors[ImGuiCol_HeaderActive]          = mHeaderBackground.WithAlpha(0.67f).ToImVec4();
        colors[ImGuiCol_HeaderHovered]         = mHeaderBackground.WithAlpha(0.8f).ToImVec4();
        colors[ImGuiCol_Header]                = mHeaderBackground.ToImVec4();
        colors[ImGuiCol_MenuBarBg]             = mMenuBackground.ToImVec4();
        colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.00f, 0.00f, 0.00f, 0.5f);
        colors[ImGuiCol_NavHighlight]          = ImVec4(30.f / 255.f, 30.f / 255.f, 30.f / 255.f, 1.00f);
        colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        colors[ImGuiCol_PlotLines]             = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        colors[ImGuiCol_PopupBg]               = mWindowBackground.ToImVec4();
        colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        colors[ImGuiCol_ResizeGrip]            = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
        colors[ImGuiCol_ScrollbarBg]           = mMenuBackground.ToImVec4();
        colors[ImGuiCol_ScrollbarGrabActive]   = mIcon.ToImVec4();
        colors[ImGuiCol_ScrollbarGrabHovered]  = mIcon.ToImVec4();
        colors[ImGuiCol_ScrollbarGrab]         = mIcon.ToImVec4();
        colors[ImGuiCol_SeparatorActive]       = mSelected.ToImVec4();
        colors[ImGuiCol_SeparatorHovered]      = mSelected.ToImVec4();
        colors[ImGuiCol_Separator]             = Color("#4e4e4e").ToImVec4();
        colors[ImGuiCol_SliderGrabActive]      = mIcon.ToImVec4();
        colors[ImGuiCol_SliderGrab]            = mIcon.ToImVec4();
        colors[ImGuiCol_TabActive]             = mHeaderBackground.ToImVec4();
        colors[ImGuiCol_TabHovered]            = mHeaderBackground.ToImVec4();
        colors[ImGuiCol_TabUnfocusedActive]    = colors[ImGuiCol_TabActive];
        colors[ImGuiCol_TabUnfocused]          = colors[ImGuiCol_Tab];
        colors[ImGuiCol_Tab]                   = mTabHeader.ToImVec4();
        colors[ImGuiCol_TableBorderLight]      = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);  // Prefer using Alpha=1.0 here
        colors[ImGuiCol_TableBorderLight]      = ImVec4(0.f, 0.f, 0.f, 0.f);
        colors[ImGuiCol_TableBorderStrong]     = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);  // Prefer using Alpha=1.0 here
        colors[ImGuiCol_TableBorderStrong]     = ImVec4(0.f, 0.f, 0.f, 0.f);
        colors[ImGuiCol_TableHeaderBg]         = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
        colors[ImGuiCol_TableRowBgAlt]         = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
        colors[ImGuiCol_TableRowBg]            = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_TextDisabled]          = mTextSecondary.ToImVec4();
        colors[ImGuiCol_TextSelectedBg]        = mSelected.WithAlpha(0.5f).ToImVec4();
        colors[ImGuiCol_Text]                  = mTextHighlight.ToImVec4();
        colors[ImGuiCol_TitleBgActive]         = mTabHeader.ToImVec4();
        colors[ImGuiCol_TitleBgCollapsed]      = mTabHeader.ToImVec4();
        colors[ImGuiCol_TitleBg]               = mTabHeader.ToImVec4();
        colors[ImGuiCol_WindowBg]              = mWindowBackground.ToImVec4();
    }
#pragma endregion

#pragma region EditorSettings
    bool EditorSettings::LoadSettings() {
        const auto settingsFile = Path::Current() / "engine.yaml";
        if (settingsFile.Exists()) {
            YAML::Node settings = YAML::LoadFile(settingsFile.Str());
            mTheme              = settings["theme"].as<str>();
            return true;
        }
        return false;
    }

    void EditorSettings::SaveSettings() const {
        const auto settingsFile = Path::Current() / "engine.yaml";
        YAML::Emitter out;
        out << YAML::BeginMap;
        {
            out << YAML::Key << "theme";
            out << YAML::Value << mTheme;
        }
        out << YAML::EndMap;
        FileWriter::WriteText(settingsFile, out.c_str());
    }
#pragma endregion

#pragma region Window Events
    void XEditor::OnInitialize() {
        SetWindowTitle("XEditor | Untitled");
        this->SetWindowIcon(APPICON);
        LoadEditorIcons();

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_NavEnableKeyboard;

        io.IniFilename = nullptr;  // Disable ui config for now

        ImFontAtlas* fontAtlas = io.Fonts;
        mDefaultFont           = fontAtlas->AddFontDefault();

        mFonts["display"] =
          fontAtlas->AddFontFromMemoryCompressedTTF(Inter_compressed_data, Inter_compressed_size, 16.0f);

        mFonts["display_20"] =
          fontAtlas->AddFontFromMemoryCompressedTTF(Inter_compressed_data, Inter_compressed_size, 20.0f);

        mFonts["display_26"] =
          fontAtlas->AddFontFromMemoryCompressedTTF(Inter_compressed_data, Inter_compressed_size, 26.0f);

        mFonts["title"] =
          fontAtlas->AddFontFromMemoryCompressedTTF(Inter_compressed_data, Inter_compressed_size, 128.0f);

        mFonts["mono"] = fontAtlas->AddFontFromMemoryCompressedTTF(JetBrainsMono_TTF_compressed_data,
                                                                   JetBrainsMono_TTF_compressed_size,
                                                                   16.0f);
        fontAtlas->Build();

        ImGui_ImplWin32_Init(mHwnd);
        ImGui_ImplDX11_Init(mContext.GetDevice(), mContext.GetDeviceContext());

        mWindowViewport->SetClearColor(Colors::Black);  // Editor background nearly black by default
        mSceneViewport.SetClearColor(Colors::Grey);     // Viewport background grey by default
        // Resize to 1x1 initially so D3D creation code doesn't fail (0x0 invalid resource size)
        mSceneViewport.Resize(1, 1);

        if (mSession.LoadSession()) { LoadProject(mSession.mLastProjectPath.Str()); }
        if (!mSettings.LoadSettings()) { mSettings.SaveSettings(); }
        mTheme.LoadTheme(mSettings.mTheme);
        mTheme.Apply();

        mMeshPreviewer.SetClearColor(mTheme.mWindowBackground);

        RegisterEditorShortcuts();
    }

    void XEditor::OnResize(u32 width, u32 height) {
        IWindow::OnResize(width, height);
    }

    void XEditor::OnShutdown() {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    void XEditor::OnUpdate() {
        // mGame.Update(false);
        mShortcutManager.ProcessShortcuts();
    }

    void XEditor::View_Alerts() {
        if (mAlertDialogOpen) {
            ImGui::OpenPopup("##alert");
            mAlertDialogOpen = false;
        }

        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, {0.5f, 0.5f});
        // ImGui::SetNextWindowSize({600, 0}, ImGuiCond_Appearing);
        if (ImGui::BeginPopupModal("##alert", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("%s", mAlertMessage);

            if (ImGui::Button("OK", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                mAlertDialogOpen = false;
            }

            ImGui::EndPopup();
        }
    }

    void XEditor::OnRender() {
        mWindowViewport->ClearAll();

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::PushFont(mFonts["display"]);

        View_MainMenu();
        const f32 menuBarHeight = ImGui::GetFrameHeight();

        if (HasSession() && mLoadedProject.mLoaded) {
            View_Toolbar(menuBarHeight);
            const f32 yOffset = menuBarHeight + kToolbarHeight;

            SetupDockspace(yOffset);

            if (mShowViewport) View_Viewport();
            if (mShowSceneSettings) View_SceneSettings();
            if (mShowEntities) View_Entities();
            if (mShowEntityProperties) View_EntityProperties();
            if (mShowAssetBrowser) View_AssetBrowser();
            if (mShowLog) View_Log();
            if (mShowAssetPreview) View_AssetPreview();
            if (mShowPostProcessing) View_PostProcessing();
            if (mShowMaterial) View_Material();

            View_StatusBar();
        } else {
            View_StartupScreen(menuBarHeight);
        }

        View_Modals();
        View_Alerts();

        ImGui::PopFont();

        mWindowViewport->AttachViewport();
        mWindowViewport->BindRenderTarget();
        ImGui::Render();

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Execute all the actions that needed to be deferred until rendering has completed
        if (!mPostRenderQueue.IsEmpty()) { mPostRenderQueue.Execute(); }
    }

    LRESULT XEditor::MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) {
        if (ImGui_ImplWin32_WndProcHandler(mHwnd, msg, wParam, lParam)) return true;
        return IWindow::MessageHandler(msg, wParam, lParam);
    }

    bool XEditor::HasSession() {
        return (Path::Current() / "session.yaml").Exists();
    }
#pragma endregion

#pragma region Editor Modals
    void XEditor::Modal_SaveSceneAs() {
        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, {0.5f, 0.5f});

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {10.0f, 10.0f});
        if (ImGui::BeginPopupModal("Save Scene As", &mSaveSceneAsOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
            {
                Gui::ScopedFont font(mFonts["display_26"]);
                ImGui::Text("Save scene as");
            }
            {
                Gui::ScopedColorVars colors({{ImGuiCol_Separator, Color("#4e4e4e").ToImVec4()}});
                ImGui::Separator();
            }

            Gui::SpacingY(10.0f);

            ImGui::SetNextItemWidth(408.0f);
            ImGui::InputText("##scene_name_as", EditorState::CurrentSceneName, sizeof(EditorState::CurrentSceneName));

            Gui::SpacingY(10.0f);

            if (Gui::PrimaryButton("OK", {200, 0})) {
                // If the name isn't empty and is different from the previous name, save the scene
                if (!X_CSTR_EMPTY(EditorState::CurrentSceneName) &&
                    std::strcmp(EditorState::CurrentSceneName, GetCurrentScene()->GetName().c_str()) != 0) {
                    OnSaveScene(EditorState::CurrentSceneName);
                    mSaveSceneAsOpen = false;
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", {200, 0})) {
                mSaveSceneAsOpen = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        ImGui::PopStyleVar();
    }

    void XEditor::Modal_AddComponent() {
        auto& state = GetSceneState();

        using ComponentMap             = unordered_map<str, str>;
        static ComponentMap components = {
          {"Model", "Renders a 3D model/mesh, making it visible in the scene"},
          {"Behavior", "Custom Lua script that defines unique behaviors and functionality"},
          {"Camera", "Captures and displays a view of the game world to the player"},
        };

        static str selectedComponent;
        static constexpr f32 itemHeight = 48.0f;

        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, {0.5f, 0.5f});
        i32 availableComponents {0};
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {10.0f, 10.0f});
        if (ImGui::BeginPopupModal("Add Component", &mAddComponentOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
            if (ImGui::IsWindowAppearing()) { selectedComponent = ""; }

            {
                Gui::ScopedFont font(mFonts["display_26"]);
                ImGui::Text("Add a component");
            }
            {
                Gui::ScopedColorVars colors({{ImGuiCol_Separator, Color("#4e4e4e").ToImVec4()}});
                ImGui::Separator();
            }

            Gui::SpacingY(10.0f);

            for (const auto& [name, description] : components) {
                bool available {true};

                if (name == "Model") {
                    if (state.HasComponent<ModelComponent>(sSelectedEntity)) { available = false; }
                } else if (name == "Behavior") {
                    if (state.HasComponent<BehaviorComponent>(sSelectedEntity)) { available = false; }
                } else if (name == "Camera") {
                    if (state.HasComponent<CameraComponent>(sSelectedEntity)) { available = false; }
                }

                if (available) {
                    availableComponents++;
                    const auto id = "##" + name + "_component_option";
                    if (Gui::SelectableWithHeaders(id.c_str(),
                                                   name.c_str(),
                                                   description.c_str(),
                                                   selectedComponent == name,
                                                   true,
                                                   {0, itemHeight})) {
                        selectedComponent = name;
                    }
                    ImGui::Dummy({0, 2.f});
                }
            }

            if (availableComponents == 0) {
                const ImVec2 size = ImGui::GetContentRegionAvail();
                Gui::CenteredText("No components available", ImGui::GetCursorScreenPos(), {size.x, 48});
            }

            Gui::SpacingY(10.0f);

            // Buttons
            if (Gui::PrimaryButton("OK", {240, 0})) {
                if (selectedComponent == "Model") {
                    auto& model = state.AddComponent<ModelComponent>(sSelectedEntity);
                    model.SetModelId(0);
                    model.SetMaterialId(0);
                    // Fixes the bug for some reason ?
                    // Just fucking call Scene::Update any time I run in to a bug now I guess, always seems to fix it
                    GetCurrentScene()->Update(0.0f);
                } else if (selectedComponent == "Behavior") {
                    state.AddComponent<BehaviorComponent>(sSelectedEntity);
                } else if (selectedComponent == "Camera") {
                    const auto* transform = state.GetComponent<TransformComponent>(sSelectedEntity);
                    X_ASSERT(transform);
                    state.AddComponent<CameraComponent>(sSelectedEntity, transform);
                }

                mAddComponentOpen = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", {240, 0})) {
                mAddComponentOpen = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        ImGui::PopStyleVar();
    }

    void XEditor::Modal_SelectAsset() {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {10.0f, 10.0f});
        if (ImGui::BeginPopupModal("Select Asset", &mSelectAssetOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
            {
                Gui::ScopedFont font(mFonts["display_26"]);
                ImGui::Text("Select asset");
            }
            {
                Gui::ScopedColorVars colors({{ImGuiCol_Separator, Color("#4e4e4e").ToImVec4()}});
                ImGui::Separator();
            }

            Gui::SpacingY(10.0f);

            // Collect all available assets of the current filter type
            vector<AssetDescriptor> availableAssets;
            std::ranges::copy_if(
              mAssetDescriptors,
              std::back_inserter(availableAssets),
              [this](const AssetDescriptor& desc) { return desc.GetTypeFromId() == mSelectAssetFilter; });

            // Assets list
            static u64 selectedAssetId {0};
            for (const auto& desc : availableAssets) {
                const str filename = Path(desc.mFilename).Filename();
                const str path     = desc.mFilename;

                if (Gui::SelectableWithHeaders(std::format("##{}_asset_select", desc.mId).c_str(),
                                               filename.c_str(),
                                               path.c_str(),
                                               desc.mId == selectedAssetId,
                                               0,
                                               {408, 48})) {
                    selectedAssetId = desc.mId;
                }
            }

            Gui::SpacingY(10.0f);

            // OK and Cancel buttons
            if (Gui::PrimaryButton("OK", {200, 0})) {
                mSelectAssetOpen = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", {200, 0})) {
                mSelectAssetOpen = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
        ImGui::PopStyleVar();
    }

    void XEditor::Modal_About() {
        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, {0.5f, 0.5f});
        if (ImGui::BeginPopupModal("About", &mAboutOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
            const auto banner     = mTextureManager.GetTexture("AboutBanner");
            const auto* bannerSrv = banner->mShaderResourceView.Get();
            ImGui::Image((ImTextureID)bannerSrv, {800, 300});
            ImGui::EndPopup();
        }
    }

    void XEditor::Modal_NewProject() {
        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, {0.5f, 0.5f});
        ImGui::SetNextWindowSize({600, 0});
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {20.0f, 20.0f});

        constexpr size_t locationSize = 512;
        static char nameBuffer[256] {0};
        static char locationBuffer[locationSize] {0};
        static str projectRoot;

        const char* engineVersions[] = {"XENGINE 1.0.0"};
        static int selectedVersion   = 0;

        if (ImGui::BeginPopupModal("New Project", &mNewProjectOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
            if (ImGui::IsWindowAppearing()) {
                std::memset(nameBuffer, 0, std::size(nameBuffer));
                std::memset(locationBuffer, 0, std::size(locationBuffer));

                const Path documentsDir = Platform::GetPlatformDirectory(Platform::kPlatformDir_Documents);
                const Path projectsRoot = documentsDir / "XENGINE Projects";

                std::strcpy(locationBuffer, projectsRoot.CStr());
                projectRoot = projectsRoot.Str();
            }

            {
                Gui::ScopedFont font(mFonts["display_26"]);
                ImGui::Text("Create a new project");
            }
            {
                Gui::ScopedColorVars colors({{ImGuiCol_Separator, Color("#4e4e4e").ToImVec4()}});
                ImGui::Separator();
            }

            Gui::SpacingY(20.0f);

            ImGui::Text("Engine Version");
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::Combo("##engine_version", &selectedVersion, engineVersions, 1)) {
                // TODO: Do nothing for now I guess
            }

            Gui::SpacingY(10.0f);

            const f32 locationLen = ImGui::CalcTextSize("Location").x;
            const f32 nameLen     = ImGui::CalcTextSize("Name").x;
            const f32 nameOffset  = locationLen - nameLen;

            Gui::SpacingX(nameOffset - ImGui::GetStyle().ItemSpacing.x);
            ImGui::SameLine();
            ImGui::Text("Name");
            ImGui::SameLine(90);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::InputText("##project_name", nameBuffer, std::size(nameBuffer))) {
                const auto nameSize = std::strlen(nameBuffer);
                if (nameSize > 0 && nameSize + projectRoot.size() < std::size(locationBuffer)) {
                    const Path projPath = Path(projectRoot) / nameBuffer;
                    std::strcpy(locationBuffer, projPath.CStr());
                }
            }

            Gui::SpacingY(2.0f);

            ImGui::Text("Location");
            ImGui::SameLine(90);
            constexpr f32 selectLocationButtonWidth = 24;
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - selectLocationButtonWidth);
            ImGui::InputText("##project_location",
                             locationBuffer,
                             std::size(locationBuffer),
                             ImGuiInputTextFlags_ReadOnly);
            ImGui::SameLine();

            const ImTextureID folderIcon = mTextureManager.GetTextureID("OpenFolderIcon");
            if (ImGui::ImageButton("##select_location",
                                   folderIcon,
                                   {16, 16},
                                   Gui::kUV_0,
                                   Gui::kUV_1,
                                   Colors::Transparent.ToImVec4(),
                                   Colors::White75.ToImVec4())) {
                char path[locationSize] {0};
                if (Platform::SelectFolderDialog(mHwnd, "Select Project Directory", path, locationSize)) {
                    std::memset(nameBuffer,
                                0,
                                std::size(nameBuffer));  // Clear name buffer to avoid issues with string concat
                    std::strncpy(locationBuffer, path, locationSize);
                    projectRoot = path;
                }
            }

            Gui::SpacingY(20.0f);

            constexpr f32 cancelButtonWidth = 100;
            constexpr f32 createButtonWidth = 140;
            constexpr f32 buttonHeight      = 24;
            const f32 itemSpacing           = ImGui::GetStyle().ItemSpacing.x * 2;
            const f32 offset = ImGui::GetContentRegionAvail().x - (cancelButtonWidth + createButtonWidth + itemSpacing);

            Gui::SpacingX(offset);
            ImGui::SameLine();
            if (ImGui::Button("Cancel##new_project", {cancelButtonWidth, buttonHeight})) {
                mNewProjectOpen = false;
                //
            }
            ImGui::SameLine();
            if (Gui::PrimaryButton("Create Project##new_project", {createButtonWidth, buttonHeight})) {
                if (OnCreateProject(nameBuffer, locationBuffer, engineVersions[selectedVersion])) {
                    mNewProjectOpen = false;
                }
            }

            ImGui::EndPopup();
        }
        ImGui::PopStyleVar();
    }

    void XEditor::Modal_ProjectSettings() {
        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, {0.5f, 0.5f});
        ImGui::SetNextWindowSize({800, 600}, ImGuiCond_Appearing);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {10.0f, 10.0f});

        static str selectedCategory = "General";
        vector<str> categories      = {"General", "Audio", "Assets", "Physics", "Rendering"};

        if (ImGui::BeginPopupModal("Project Settings",
                                   &mProjectSettingsOpen,
                                   ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize)) {
            {
                Gui::ScopedFont font(mFonts["display_26"]);
                ImGui::Text("Project Settings");
            }
            ImGui::Separator();
            Gui::SpacingY(10.0f);

            // Settings categories
            {
                if (ImGui::BeginChild("##settings_categories", {240.0f, 600.0f - 44.0f})) {
                    for (const auto& category : categories) {
                        const bool selected = selectedCategory == category;
                        if (ImGui::Selectable(category.c_str(), selected)) { selectedCategory = category; }
                    }
                    ImGui::EndChild();
                }
            }
            ImGui::SameLine();
            // Category options
            {
                if (ImGui::BeginChild("##category_options", {800.0f - 240.0f - 10.0f, 600.0f - 44.0f})) {
                    ImGui::EndChild();
                }
            }

            Gui::SpacingY(10.0f);
            constexpr f32 cancelButtonWidth = 100;
            constexpr f32 saveButtonWidth   = 140;
            constexpr f32 buttonHeight      = 24;
            const f32 itemSpacing           = ImGui::GetStyle().ItemSpacing.x * 2;
            const f32 offset = ImGui::GetContentRegionAvail().x - (cancelButtonWidth + saveButtonWidth + itemSpacing);

            Gui::SpacingX(offset);
            ImGui::SameLine();
            if (ImGui::Button("Cancel##project_settings", {cancelButtonWidth, buttonHeight})) {
                mProjectSettingsOpen = false;
                //
            }
            ImGui::SameLine();
            if (Gui::PrimaryButton("Save##project_settings", {saveButtonWidth, buttonHeight})) {
                mProjectSettingsOpen = false;
                //
            }

            ImGui::EndPopup();
        }
        ImGui::PopStyleVar();
    }

    void XEditor::Modal_CreateMaterial() {
        static i32 selectedType            = 0;
        static const char* materialTypes[] = {"Standard Lit", "Basic Lit"};
        char nameBuffer[256] {0};

        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, {0.5f, 0.5f});
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {10.0f, 10.0f});
        if (ImGui::BeginPopupModal("Create Material", &mCreateMaterialOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
            {
                Gui::ScopedFont font(mFonts["display_26"]);
                ImGui::Text("Create a new material");
            }
            {
                Gui::ScopedColorVars colors({{ImGuiCol_Separator, Color("#4e4e4e").ToImVec4()}});
                ImGui::Separator();
            }

            Gui::SpacingY(10.0f);

            ImGui::Text("Material Type");
            ImGui::SetNextItemWidth(408);
            if (ImGui::Combo("##material_type_combo", &selectedType, materialTypes, std::size(materialTypes))) {
                selectedType = selectedType % std::size(materialTypes);
            }
            ImGui::SetNextItemWidth(408);
            ImGui::InputText("##material_name", nameBuffer, sizeof(nameBuffer));

            Gui::SpacingY(10.0f);

            if (Gui::PrimaryButton("OK", {200, 0})) {
                mCreateMaterialOpen = false;
                if (!mShowMaterial) { mShowMaterial = true; }
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", {200, 0})) { mCreateMaterialOpen = false; }

            ImGui::EndPopup();
        }
        ImGui::PopStyleVar();
    }

    void XEditor::Modal_SelectScene() {
        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, {0.5f, 0.5f});
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {10.0f, 10.0f});
        if (ImGui::BeginPopupModal("Select Scene", &mSceneSelectorOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
            {
                Gui::ScopedFont font(mFonts["display_26"]);
                ImGui::Text("Select scene");
            }
            {
                Gui::ScopedColorVars colors({{ImGuiCol_Separator, Color("#4e4e4e").ToImVec4()}});
                ImGui::Separator();
            }

            Gui::SpacingY(10.0f);

            static str selectedScene;

            u32 index {0};
            for (const auto& [name, desc] : mGame.GetSceneMap()) {
                str id          = "##" + X_TOSTR(index) + "scene_select";  // ex. ##0_select_scene
                str description = desc.mDescription;
                if (description.length() > 50) { description = description.substr(0, 50) + "..."; }
                if (Gui::SelectableWithHeaders(id.c_str(),
                                               name.c_str(),
                                               description.c_str(),
                                               selectedScene == name,
                                               ImGuiSelectableFlags_None,
                                               {0, 48})) {
                    selectedScene = name;
                }
                index++;
                ImGui::Dummy({0, 2.f});
            }

            Gui::SpacingY(10.0f);

            // Buttons
            if (Gui::PrimaryButton("OK", {200, 0})) {
                if (!selectedScene.empty()) { OnLoadScene(selectedScene); }

                mSceneSelectorOpen = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", {200, 0})) {
                mSceneSelectorOpen = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        ImGui::PopStyleVar();
    }

    void XEditor::Modal_AddEntity() {
        static char entityName[256] {0};

        if (ImGui::BeginPopupModal("Add New Entity", &mAddEntityOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
            if (ImGui::IsWindowAppearing()) {
                const auto numEntities = GetEntities().size();
                const str name         = std::format("Entity{}", numEntities + 1);
                std::strcpy(entityName, name.c_str());
            }

            {
                Gui::ScopedFont font(mFonts["display_26"]);
                ImGui::Text("Add a new entity");
            }
            {
                Gui::ScopedColorVars colors({{ImGuiCol_Separator, Color("#4e4e4e").ToImVec4()}});
                ImGui::Separator();
            }

            Gui::SpacingY(10.0f);

            ImGui::Text("Name:");
            ImGui::PushItemWidth(408.0f);
            const bool enterPressed = ImGui::InputText("##new_entity_name",
                                                       entityName,
                                                       sizeof(entityName),
                                                       ImGuiInputTextFlags_EnterReturnsTrue);
            ImGui::PopItemWidth();

            Gui::SpacingY(10.0f);

            if (Gui::PrimaryButton("OK", {200, 0}) || enterPressed) {
                if (std::strlen(entityName) > 0) {
                    OnAddEntity(entityName);
                    mAddEntityOpen = false;
                }
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel", {200, 0})) { mAddEntityOpen = false; }

            ImGui::EndPopup();
        }
    }
#pragma endregion

#pragma region Editor Views
    void XEditor::View_MainMenu() {
        {
            Gui::ScopedStyleVars styles({{ImGuiStyleVar_WindowBorderSize, 0.0f}});
            Gui::ScopedColorVars colors({{ImGuiCol_Text, mTheme.mTextPrimary.ToImVec4()},
                                         {ImGuiCol_PopupBg, mTheme.mInputBackground.ToImVec4()},
                                         {ImGuiCol_Separator, Colors::White.WithAlpha(0.1f).ToImVec4()}});
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("New Project", "Ctrl+N")) { mNewProjectOpen = true; }
                    if (ImGui::MenuItem("Open Project", "Ctrl+O")) { OnOpenProject(); }
                    ImGui::Separator();
                    if (ImGui::MenuItem("New Scene", "Ctrl+Shift+N", false, mLoadedProject.mLoaded)) {}
                    if (ImGui::MenuItem("Open Scene", "Ctrl+Shift+O", false, mLoadedProject.mLoaded)) {
                        mSceneSelectorOpen = true;
                    }
                    if (ImGui::MenuItem("Save Scene", "Ctrl+S", false, mLoadedProject.mLoaded)) { OnSaveScene(); }
                    if (ImGui::MenuItem("Save Scene As", "Ctrl+Shift+S", false, mLoadedProject.mLoaded)) {
                        mSaveSceneAsOpen = true;
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Exit", "Alt+F4")) { this->Quit(); }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Edit")) {
                    if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
                    if (ImGui::MenuItem("Redo", "Ctrl+Y")) {}
                    ImGui::Separator();
                    if (ImGui::MenuItem("Project Settings", "Ctrl+Alt+S")) { mProjectSettingsOpen = true; }
                    if (ImGui::MenuItem("Preferences", "Ctrl+Alt+P")) {}
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Assets")) {
                    if (ImGui::BeginMenu("Create")) {
                        if (ImGui::MenuItem("Material")) { mCreateMaterialOpen = true; }
                        if (ImGui::MenuItem("Script")) {}
                        if (ImGui::MenuItem("Texture")) {}
                        ImGui::EndMenu();
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Import Asset")) { OnImportAsset(); }
                    if (ImGui::MenuItem("Import Engine Content")) { OnImportEngineContent(); }

                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("View")) {
                    if (ImGui::MenuItem("Scene")) { mShowSceneSettings = !mShowSceneSettings; }
                    if (ImGui::MenuItem("Entities")) { mShowEntities = !mShowEntities; }
                    if (ImGui::MenuItem("Properties")) { mShowEntityProperties = !mShowEntityProperties; }
                    if (ImGui::MenuItem("Viewport")) { mShowViewport = !mShowViewport; }
                    if (ImGui::MenuItem("Asset Browser")) { mShowAssetBrowser = !mShowAssetBrowser; }
                    if (ImGui::MenuItem("Log")) { mShowLog = !mShowLog; }
                    if (ImGui::MenuItem("Asset Preview")) { mShowAssetPreview = !mShowAssetPreview; }
                    if (ImGui::MenuItem("Post Processing")) { mShowPostProcessing = !mShowPostProcessing; }
                    if (ImGui::MenuItem("Material")) { mShowMaterial = !mShowMaterial; }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Window")) {
                    if (ImGui::BeginMenu("Layouts")) {
                        // TODO: Add default layouts here
                        if (ImGui::MenuItem("Reset")) { OnResetWindow(); }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Help")) {
                    if (ImGui::MenuItem("About")) { mAboutOpen = true; }
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }
        }
    }

    void XEditor::View_Toolbar(const f32 menuBarHeight) {
        constexpr ImGuiWindowFlags toolbarFlags = ImGuiWindowFlags_NoTitleBar |            // No title bar needed
                                                  ImGuiWindowFlags_NoScrollbar |           // Disable scrolling
                                                  ImGuiWindowFlags_NoMove |                // Prevent moving
                                                  ImGuiWindowFlags_NoResize |              // Prevent resizing
                                                  ImGuiWindowFlags_NoCollapse |            // Prevent collapsing
                                                  ImGuiWindowFlags_NoSavedSettings |       // Don't save position/size
                                                  ImGuiWindowFlags_NoBringToFrontOnFocus;  // Don't change z-order

        ImGui::SetNextWindowPos({0, menuBarHeight});
        ImGui::SetNextWindowSize({ImGui::GetMainViewport()->Size.x, kToolbarHeight});

        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {2, 2});

        const ImVec4 background = ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, background);
        ImGui::PushStyleColor(ImGuiCol_Button, {0, 0, 0, 0});

        if (ImGui::Begin("##Toolbar", nullptr, toolbarFlags)) {
            static constexpr ImVec2 btnSize = {24, 24};

            // Calculate offset in order to center all the buttons
            // Honestly, most of this is guess work. It appears centered to me, but it's probably not *exactly* centered
            const ImVec2 toolbarSize   = ImGui::GetContentRegionAvail();
            constexpr int numButtons   = 12;  // 3 groups of 4
            constexpr f32 buttonsWidth = btnSize.x * numButtons;
            const f32 buttonsSpacing =
              ((numButtons + 8) * ImGui::GetStyle().ItemSpacing.x) + (ImGui::GetStyle().WindowPadding.x * 2);
            const f32 totalButtonsWidth =
              buttonsWidth + buttonsSpacing + 120.0f;  // 40 comes from the two 20 unit spacers
            const f32 centerX       = toolbarSize.x / 2.0f;
            const f32 buttonsCenter = totalButtonsWidth / 2.0f;
            const f32 offset        = centerX - buttonsCenter;

            Gui::SpacingX(offset);
            ImGui::SameLine();

            // VIEWPORT/EDITOR ACTIONS
            {
                const auto undoIcon = (ImTextureID)(mTextureManager.GetTexture("UndoIcon")->mShaderResourceView.Get());
                const auto redoIcon = (ImTextureID)(mTextureManager.GetTexture("RedoIcon")->mShaderResourceView.Get());
                const auto compileCodeIcon =
                  (ImTextureID)(mTextureManager.GetTexture("CompileCodeIcon")->mShaderResourceView.Get());
                const auto cleanCodeIcon =
                  (ImTextureID)(mTextureManager.GetTexture("CleanCodeIcon")->mShaderResourceView.Get());

                ImGui::ImageButton("##undo_btn",
                                   undoIcon,
                                   btnSize,
                                   Gui::kUV_0,
                                   Gui::kUV_1,
                                   Colors::Transparent.ToImVec4(),
                                   ImGui::GetStyleColorVec4(ImGuiCol_CheckMark));
                ImGui::SameLine();

                ImGui::ImageButton("##redo_btn",
                                   redoIcon,
                                   btnSize,
                                   Gui::kUV_0,
                                   Gui::kUV_1,
                                   Colors::Transparent.ToImVec4(),
                                   ImGui::GetStyleColorVec4(ImGuiCol_CheckMark));
                ImGui::SameLine();

                ImGui::ImageButton("##compile_code_btn",
                                   compileCodeIcon,
                                   btnSize,
                                   Gui::kUV_0,
                                   Gui::kUV_1,
                                   Colors::Transparent.ToImVec4(),
                                   ImGui::GetStyleColorVec4(ImGuiCol_CheckMark));
                ImGui::SameLine();

                ImGui::ImageButton("##clean_code_btn",
                                   cleanCodeIcon,
                                   btnSize,
                                   Gui::kUV_0,
                                   Gui::kUV_1,
                                   Colors::Transparent.ToImVec4(),
                                   ImGui::GetStyleColorVec4(ImGuiCol_CheckMark));
            }

            ImGui::SameLine();
            Gui::SpacingX(20.0f);
            ImGui::SameLine();

            // GAME PLAY-MODE BUTTONS (PLAY, PAUSE, STOP, DETACH)
            {
                const auto playIcon = (ImTextureID)(mTextureManager.GetTexture("PlayIcon")->mShaderResourceView.Get());
                const auto pauseIcon =
                  (ImTextureID)(mTextureManager.GetTexture("PauseIcon")->mShaderResourceView.Get());
                const auto stopIcon = (ImTextureID)(mTextureManager.GetTexture("StopIcon")->mShaderResourceView.Get());
                const auto playWindowedIcon =
                  (ImTextureID)(mTextureManager.GetTexture("PlayWindowedIcon")->mShaderResourceView.Get());

                ImGui::ImageButton("##play_btn",
                                   playIcon,
                                   btnSize,
                                   Gui::kUV_0,
                                   Gui::kUV_1,
                                   Colors::Transparent.ToImVec4(),
                                   ImGui::GetStyleColorVec4(ImGuiCol_CheckMark));
                ImGui::SameLine();

                ImGui::ImageButton("##pause_btn",
                                   pauseIcon,
                                   btnSize,
                                   Gui::kUV_0,
                                   Gui::kUV_1,
                                   Colors::Transparent.ToImVec4(),
                                   ImGui::GetStyleColorVec4(ImGuiCol_CheckMark));
                ImGui::SameLine();

                ImGui::ImageButton("##stop_btn",
                                   stopIcon,
                                   btnSize,
                                   Gui::kUV_0,
                                   Gui::kUV_1,
                                   Colors::Transparent.ToImVec4(),
                                   ImGui::GetStyleColorVec4(ImGuiCol_CheckMark));
                ImGui::SameLine();

                ImGui::ImageButton("##play_win_btn",
                                   playWindowedIcon,
                                   btnSize,
                                   Gui::kUV_0,
                                   Gui::kUV_1,
                                   Colors::Transparent.ToImVec4(),
                                   ImGui::GetStyleColorVec4(ImGuiCol_CheckMark));
            }

            ImGui::SameLine();
            Gui::SpacingX(20.0f);
            ImGui::SameLine();

            // SELECT MODE TOOLBAR GROUP
            {
                static int currentSelectMode {0};
                const auto cursorIcon =
                  (ImTextureID)(mTextureManager.GetTexture("SelectIcon")->mShaderResourceView.Get());
                const auto moveIcon = (ImTextureID)(mTextureManager.GetTexture("MoveIcon")->mShaderResourceView.Get());
                const auto rotateIcon =
                  (ImTextureID)(mTextureManager.GetTexture("RotateIcon")->mShaderResourceView.Get());
                const auto scaleIcon =
                  (ImTextureID)(mTextureManager.GetTexture("ScaleIcon")->mShaderResourceView.Get());

                if (Gui::ToggleButtonGroup("##select_modes",
                                           btnSize,
                                           &currentSelectMode,
                                           {cursorIcon, moveIcon, rotateIcon, scaleIcon})) {
                    // Select mode has changed
                }
            }

            ImGui::End();
        }

        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(2);
    }

    void XEditor::View_SceneSettings() {
        ImGui::Begin("Scene");
        {
            if (mGame.IsInitialized() && GetCurrentScene()->Loaded()) {
                auto& state = GetSceneState();

                const f32 width = ImGui::GetContentRegionAvail().x;

                {
                    Gui::ScopedFont font(mFonts["display_20"]);
                    ImGui::Text("World");
                }

                Gui::SpacingY(8.0f);

                // Sun
                if (ImGui::CollapsingHeader("Sun", ImGuiTreeNodeFlags_DefaultOpen)) {
                    auto& sun = state.GetLightState().mSun;
                    ImGui::Text("Enabled:");
                    ImGui::SameLine(kLabelWidth);
                    ImGui::Checkbox("##sun_enabled", (bool*)&sun.mEnabled);

                    ImGui::Text("Intensity:");
                    ImGui::SameLine(kLabelWidth);
                    ImGui::SetNextItemWidth(width - kLabelWidth);
                    ImGui::InputFloat("##sun_intensity", &sun.mIntensity, 0.0f, 0.0f, "%.1f");

                    ImGui::Text("Color:");
                    ImGui::SameLine(kLabelWidth);
                    ImGui::SetNextItemWidth(width - kLabelWidth);
                    ImGui::ColorEdit3("##sun_color", (f32*)&sun.mColor);

                    ImGui::Text("Direction:");
                    ImGui::SameLine(kLabelWidth);
                    if (Gui::DragFloatNColored("##sun_direction",
                                               (f32*)&sun.mDirection,
                                               3,
                                               0.01f,
                                               0.01f,
                                               1.0f,
                                               "%.3f",
                                               1.0f)) {
                        GetCurrentScene()->Update(0.0f);
                    }

                    ImGui::Text("Casts Shadows:");
                    ImGui::SameLine(kLabelWidth);
                    ImGui::SetNextItemWidth(width - kLabelWidth);
                    ImGui::Checkbox("##sun_casts_shadows", (bool*)&sun.mCastsShadows);
                }

                Gui::SpacingY(8.0f);

                static f32 skyColor[3] {0, 0, 0};
                if (ImGui::CollapsingHeader("Sky", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Text("Color:");
                    ImGui::SameLine(kLabelWidth);
                    ImGui::SetNextItemWidth(width - kLabelWidth);
                    if (ImGui::ColorEdit3("##sky_color", skyColor)) {
                        // TODO: This isn't doing anything ??
                        // mSceneViewport.SetClearColor(skyColor[0], skyColor[1], skyColor[2], 1.0f);
                        GetCurrentScene()->Update(0.0f);
                    }
                }
            }
        }
        ImGui::End();
    }

    void XEditor::View_StartupScreen(f32 yOffset) {
        constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                                                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize |
                                                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                                 ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoScrollbar;

        ImVec2 size = ImGui::GetMainViewport()->Size;
        ImGui::SetNextWindowSize({size.x, size.y - yOffset});
        ImGui::SetNextWindowPos({0, yOffset});

        const f32 yHalfway      = (size.y - yOffset) / 2.0f;
        const f32 buttonSize    = 220.0f;
        const f32 buttonSpacing = 40.0f;
        const f32 buttonsWidth  = (buttonSize * 3.0f) + (buttonSpacing * 2.0f);
        const ImVec2 buttonCursorPos {(size.x / 2.0f) - (buttonsWidth / 2.0f), yHalfway};
        const ImVec2 titleSize {579, 97};
        const ImVec2 titleCursorPos {(size.x / 2.0f) - (titleSize.x / 2.0f), yHalfway - buttonSize};

        {
            Gui::ScopedStyleVars vars({{ImGuiStyleVar_WindowRounding, 0.0f}, {ImGuiStyleVar_WindowBorderSize, 0.0f}});
            Gui::ScopedColorVars colors({{ImGuiCol_WindowBg, ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg)}});
            ImGui::Begin("Startup Screen", nullptr, windowFlags);
            {
                // Draw background logos
                ID3D11ShaderResourceView* bgLogos =
                  mTextureManager.GetTexture("BackgroundLogos")->mShaderResourceView.Get();
                ImGui::SetCursorPos({0, yOffset});
                ImGui::Image((ImTextureID)(bgLogos), {size.x, size.y - yOffset});

                // Draw title image
                ID3D11ShaderResourceView* titleImage =
                  mTextureManager.GetTexture("XEditorLogo")->mShaderResourceView.Get();
                ImGui::SetCursorPos(titleCursorPos);
                ImGui::Image((ImTextureID)(titleImage), titleSize);

                // Button icons
                // ID3D11ShaderResourceView* controllerIcon =
                //   mTextureManager.GetTexture("ControllerIcon")->mShaderResourceView.Get();
                // ID3D11ShaderResourceView* folderIcon =
                //   mTextureManager.GetTexture("FolderOutlineIcon")->mShaderResourceView.Get();
                // ID3D11ShaderResourceView* cogIcon = mTextureManager.GetTexture("CogIcon")->mShaderResourceView.Get();

                Gui::ScopedStyleVars buttonVars(
                  {{ImGuiStyleVar_FrameRounding, 16.0f}, {ImGuiStyleVar_FrameBorderSize, 4.0f}});
                Gui::ScopedColorVars buttonColors({{ImGuiCol_Border, Color("#353535").ToImVec4()},
                                                   {ImGuiCol_Button, Color("#161616").ToImVec4()},
                                                   {ImGuiCol_ButtonActive, Color("#222222").ToImVec4()},
                                                   {ImGuiCol_ButtonHovered, Color("#1F1F1F").ToImVec4()}});

                ImGui::PushFont(mFonts["display_20"]);

                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {buttonSpacing, 4.0f});
                ImGui::SetCursorPos(buttonCursorPos);
                if (Gui::BorderedButton("New Project", {buttonSize, buttonSize})) { mNewProjectOpen = true; }
                ImGui::SameLine();

                if (Gui::BorderedButton("Open Project", {buttonSize, buttonSize})) { OnOpenProject(); }
                ImGui::SameLine();

                if (Gui::BorderedButton("Settings", {buttonSize, buttonSize})) {}
                ImGui::PopStyleVar();

                ImGui::PopFont();
            }
            ImGui::End();
        }
    }

    void XEditor::View_Entities() {
        static bool showRenameEntity {false};

        ImGui::Begin("Entities");
        {
            const ImVec2 windowSize = ImGui::GetContentRegionAvail();

            // Define sizes with adjustment for borders and scrollbar appearance
            constexpr f32 buttonHeight    = 24.0f;
            constexpr f32 buttonPadding   = 4.0f;
            constexpr f32 totalButtonArea = buttonHeight + (buttonPadding * 2);
            // Add a small adjustment factor to prevent scrollbar appearances
            constexpr f32 heightAdjustment = 4.0f;
            const f32 listHeight           = windowSize.y - totalButtonArea - heightAdjustment;

            ImGui::BeginChild("##entities_scroll_list", {windowSize.x, listHeight}, true);
            {
                if (mGame.IsInitialized() && GetCurrentScene()->Loaded()) {
                    for (auto& [id, name] : GetEntities()) {
                        bool isSelected = id == sSelectedEntity;
                        if (ImGui::Selectable(name.c_str(), isSelected)) { sSelectedEntity = id; }

                        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                            sSelectedEntity = id;
                            ImGui::OpenPopup("entity_context_menu");
                        }
                    }
                }
            }

            if (ImGui::BeginPopup("entity_context_menu")) {
                ImGui::Text("Entity: %s", GetEntities()[sSelectedEntity].c_str());
                ImGui::Separator();
                if (ImGui::MenuItem("Rename")) { showRenameEntity = true; }
                if (ImGui::MenuItem("Delete")) {
                    GetSceneState().DestroyEntity(sSelectedEntity);
                    sSelectedEntity = GetSceneState().GetFirstEntity();
                    GetCurrentScene()->Update(0.0f);
                }

                ImGui::EndPopup();
            }

            ImGui::EndChild();

            ImGui::Dummy({0, buttonPadding});
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {buttonPadding, buttonPadding});
            const ImVec2 remainingSpace = ImGui::GetContentRegionAvail();

            if (ImGui::Button("Add Entity", {remainingSpace.x, buttonHeight})) { mAddEntityOpen = true; }
            ImGui::PopStyleVar();
        }
        ImGui::End();

        if (showRenameEntity) { ImGui::OpenPopup("Rename Entity"); }

        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, {0.5f, 0.5f});
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {10.0f, 10.0f});

        static char entityName[256] {0};
        if (ImGui::BeginPopupModal("Rename Entity",
                                   &showRenameEntity,
                                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
            if (ImGui::IsWindowAppearing()) { std::strcpy(entityName, GetEntities()[sSelectedEntity].c_str()); }

            {
                Gui::ScopedFont font(mFonts["display_26"]);
                ImGui::Text("Rename");
            }
            ImGui::Separator();

            Gui::SpacingY(10.0f);

            ImGui::Text("Name:");
            ImGui::PushItemWidth(408.0f);
            const bool enterPressed = ImGui::InputText("##rename_entity_name",
                                                       entityName,
                                                       sizeof(entityName),
                                                       ImGuiInputTextFlags_EnterReturnsTrue);
            ImGui::PopItemWidth();

            Gui::SpacingY(10.0f);

            if (Gui::PrimaryButton("OK", {200, 0}) || enterPressed) {
                if (std::strlen(entityName) > 0) {
                    GetSceneState().RenameEntity(sSelectedEntity, entityName);
                    showRenameEntity = false;
                }
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel", {200, 0})) { showRenameEntity = false; }

            ImGui::EndPopup();
        }
        ImGui::PopStyleVar();
    }

    void XEditor::View_EntityProperties() {
        static const ImTextureID selectAssetIcon =
          (ImTextureID)(mTextureManager.GetTexture("SelectAssetIcon").value().mShaderResourceView.Get());
        static AssetDescriptor selectedAsset {};

        ImGui::Begin("Properties");
        const ImVec2 size = ImGui::GetContentRegionAvail();
        {
            if (mGame.IsInitialized() && GetCurrentScene()->Loaded() && sSelectedEntity.Valid()) {
                auto& state = GetSceneState();

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
                auto* camera   = state.GetComponentMutable<CameraComponent>(sSelectedEntity);

                {
                    Gui::ScopedFont font(mFonts["display_20"]);
                    ImGui::Text(GetEntities()[sSelectedEntity].c_str());
                }

                Gui::SpacingY(8.0f);

                if (ImGui::CollapsingHeader("General##properties", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Text("Enabled:");
                    ImGui::SameLine(kLabelWidth);
                    ImGui::SetNextItemWidth(size.x - kLabelWidth);
                    static bool enabled {true};
                    ImGui::Checkbox("##entity_enabled", &enabled);
                }

                Gui::SpacingY(10.0f);
                if (ImGui::CollapsingHeader("Transform##properties", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Text("Position:");
                    ImGui::SameLine(kLabelWidth);
                    ImGui::SetNextItemWidth(size.x - kLabelWidth);
                    Gui::DragFloatNColored("##entity_transform_position",
                                           (f32*)&EditorState::TransformPosition,
                                           3,
                                           0.01f,
                                           -FLT_MAX,
                                           FLT_MAX,
                                           "%.3f",
                                           1.0f);
                    transform->SetPosition(EditorState::TransformPosition);

                    ImGui::Text("Rotation:");
                    ImGui::SameLine(kLabelWidth);
                    ImGui::SetNextItemWidth(size.x - kLabelWidth);
                    Gui::DragFloatNColored("##entity_transform_rotation",
                                           (f32*)&EditorState::TransformRotation,
                                           3,
                                           0.01f,
                                           -FLT_MAX,
                                           FLT_MAX,
                                           "%.3f",
                                           1.0f);
                    transform->SetRotation(EditorState::TransformRotation);

                    ImGui::Text("Scale:");
                    ImGui::SameLine(kLabelWidth);
                    ImGui::SetNextItemWidth(size.x - kLabelWidth);
                    Gui::DragFloatNColored("##entity_transform_scale",
                                           (f32*)&EditorState::TransformScale,
                                           3,
                                           0.01f,
                                           -FLT_MAX,
                                           FLT_MAX,
                                           "%.3f",
                                           1.0f);
                    transform->SetScale(EditorState::TransformScale);
                    transform->Update();
                }

                if (model) {
                    Gui::SpacingY(10.0f);
                    if (ImGui::CollapsingHeader("Model##properties", ImGuiTreeNodeFlags_DefaultOpen)) {
                        ImGui::Text("Mesh (Asset):");
                        ImGui::SameLine(kLabelWidth);

                        const auto& modelAsset =
                          std::ranges::find_if(mAssetDescriptors, [&model](const AssetDescriptor& asset) {
                              return asset.mId == model->GetModelId();
                          });
                        const bool foundModel = modelAsset != mAssetDescriptors.end();

                        auto UpdateMesh = [this, &model](const AssetDescriptor& descriptor) {
                            const auto oldId = model->GetModelId();
                            if (oldId == descriptor.mId) { return; }

                            auto& resourceManager = GetCurrentScene()->GetResourceManager();
                            if (resourceManager.LoadResource<Model>(descriptor.mId)) {
                                const ResourceHandle<Model> modelResource =
                                  resourceManager.FetchResource<Model>(descriptor.mId);
                                if (modelResource.Valid()) {
                                    model->SetModelHandle(modelResource);
                                    model->SetModelId(descriptor.mId);
                                }
                            }
                        };

                        static char modelBuffer[256] {0};
                        const auto currentModel = foundModel ? Path(modelAsset->mFilename).Filename() : "None";
                        std::strcpy(modelBuffer, currentModel.c_str());

                        if (Gui::AssetDropTarget("##model_drop_target",
                                                 modelBuffer,
                                                 sizeof(modelBuffer),
                                                 selectAssetIcon,
                                                 X_DROP_TARGET_MESH,
                                                 UpdateMesh)) {
                            mSelectAssetOpen   = true;
                            mSelectAssetFilter = kAssetType_Mesh;
                        }

                        ImGui::Text("Material (Asset):");
                        ImGui::SameLine(kLabelWidth);

                        const auto& materialAsset =
                          std::ranges::find_if(mAssetDescriptors, [&model](const AssetDescriptor& asset) {
                              return asset.mId == model->GetMaterialId();
                          });
                        const bool foundMaterial = materialAsset != mAssetDescriptors.end();

                        auto UpdateMaterial =
                          [this, &materialAsset, &foundMaterial, &model](const AssetDescriptor& descriptor) {
                              if (foundMaterial) {
                                  if (materialAsset->mId == descriptor.mId) { return; }
                              }

                              const auto assetBytes = AssetManager::GetAssetData(descriptor.mId);
                              X_ASSERT(assetBytes.has_value())
                              MaterialDescriptor desc;
                              if (!MaterialParser::Parse(*assetBytes, desc)) { return; }
                              const auto mat = GetCurrentScene()->LoadMaterial(desc);
                              X_ASSERT(mat.get() != nullptr)
                              model->SetMaterial(mat);
                              model->SetMaterialId(descriptor.mId);

                              GetCurrentScene()->Update(0.0f);
                          };

                        static char materialBuffer[256] {0};
                        const auto currentMaterial = foundMaterial ? Path(materialAsset->mFilename).Filename() : "None";
                        std::strcpy(materialBuffer, currentMaterial.c_str());

                        if (Gui::AssetDropTarget("##material_drop_target",
                                                 materialBuffer,
                                                 sizeof(materialBuffer),
                                                 selectAssetIcon,
                                                 X_DROP_TARGET_MATERIAL,
                                                 UpdateMaterial)) {
                            mSelectAssetOpen   = true;
                            mSelectAssetFilter = kAssetType_Material;
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
                    Gui::SpacingY(10.0f);
                    if (ImGui::CollapsingHeader("Behavior##properties", ImGuiTreeNodeFlags_DefaultOpen)) {
                        if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                            // TODO: Context menu for removing components
                        }

                        ImGui::Text("Script (Asset):");
                        ImGui::SameLine(kLabelWidth);
                        ImGui::SetNextItemWidth(size.x - kLabelWidth);

                        const auto& scriptAsset =
                          std::ranges::find_if(mAssetDescriptors, [&behavior](const AssetDescriptor& asset) {
                              return asset.mId == behavior->GetScriptId();
                          });
                        const bool foundScript = scriptAsset != mAssetDescriptors.end();

                        auto UpdateScript = [this, &scriptAsset, &foundScript](const AssetDescriptor& descriptor) {
                            if (foundScript) {
                                const auto oldId = scriptAsset->mId;
                                if (oldId == descriptor.mId) { return; }
                            }

                            // TODO: Implement UpdateScript
                        };

                        static char scriptBuffer[256] {0};
                        const auto currentScript = foundScript ? Path(scriptAsset->mFilename).Filename() : "None";
                        std::strcpy(scriptBuffer, currentScript.c_str());

                        if (Gui::AssetDropTarget("##script_drop_target",
                                                 scriptBuffer,
                                                 sizeof(scriptBuffer),
                                                 selectAssetIcon,
                                                 X_DROP_TARGET_SCRIPT,
                                                 UpdateScript)) {
                            mSelectAssetOpen   = true;
                            mSelectAssetFilter = kAssetType_Script;
                        }
                    }
                }

                static f32 fovDegrees {45.0f};
                static f32 nearZ {0.01f};
                static f32 farZ {1000.0f};
                static bool orthographic {false};
                static Float2 viewport {32.0f, 16.0f};

                if (camera) {
                    fovDegrees   = camera->GetFOVDegrees();
                    nearZ        = camera->GetNearPlane();
                    farZ         = camera->GetFarPlane();
                    orthographic = camera->GetOrthographic();
                    viewport.x   = camera->GetWidth();
                    viewport.y   = camera->GetHeight();

                    Gui::SpacingY(10.0f);
                    if (ImGui::CollapsingHeader("Camera##properties", ImGuiTreeNodeFlags_DefaultOpen)) {
                        ImGui::Text("Orthographic:");
                        ImGui::SameLine(kLabelWidth);
                        ImGui::SetNextItemWidth(size.x - kLabelWidth);
                        ImGui::Checkbox("##ortho_properties", &orthographic);

                        if (orthographic) {
                            ImGui::Text("Viewport:");
                            ImGui::SameLine(kLabelWidth);
                            ImGui::SetNextItemWidth(size.x - kLabelWidth);
                            Gui::DragFloatNColored("##viewport_properties",
                                                   (f32*)&viewport,
                                                   2,
                                                   1.0f,
                                                   0.0f,
                                                   FLT_MAX,
                                                   "%.1f",
                                                   1.0f);
                        } else {
                            ImGui::Text("FOV:");
                            ImGui::SameLine(kLabelWidth);
                            ImGui::SetNextItemWidth(size.x - kLabelWidth);
                            ImGui::InputFloat("##fov_properties", &fovDegrees, 0.0f, 0.0f, "%.1f");
                        }
                        ImGui::Text("Near Plane:");
                        ImGui::SameLine(kLabelWidth);
                        ImGui::SetNextItemWidth(size.x - kLabelWidth);
                        ImGui::InputFloat("##nearz_properties", &nearZ, 0.0f, 0.0f, "%.2f");

                        ImGui::Text("Far Plane:");
                        ImGui::SameLine(kLabelWidth);
                        ImGui::SetNextItemWidth(size.x - kLabelWidth);
                        ImGui::InputFloat("##farz_properties", &farZ, 0.0f, 0.0f, "%.2f");
                    }

                    camera->SetFOVDegrees(fovDegrees);
                    camera->SetNearPlane(nearZ);
                    camera->SetFarPlane(farZ);
                    camera->SetOrthographic(orthographic);
                    camera->SetWidth(viewport.x);
                    camera->SetHeight(viewport.y);
                    camera->Update();
                }

                Gui::SpacingY(10.0f);

                if (ImGui::Button("Add Component##button", {size.x, 0})) { mAddComponentOpen = true; }
            } else {
                {
                    Gui::ScopedFont font(mFonts["display_20"]);
                    ImGui::Text("No entity selected");
                }
            }
        }
        ImGui::End();
    }

    void XEditor::View_Viewport() {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
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
            ImGui::Image((ImTextureID)(srv), contentSize);
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    void XEditor::View_AssetBrowser() {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {10.0f, 10.0f});
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

                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {4.0f, 4.0f});
                Gui::ScopedColorVars colors({{ImGuiCol_ChildBg, mTheme.mWindowBackground.ToImVec4()}});
                if (ImGui::BeginChild("##GridScrollRegion", {0, 0}, false)) {
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
                            ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, {0.5f, 0.5f});
                            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {0.0f, 0.0f});
                            if (ImGui::Selectable("##cell",
                                                  sSelectedAsset == itemIndex,
                                                  0,
                                                  {cellWidth, thumbnailSize + 25})) {
                                // Handle selection
                                sSelectedAsset  = itemIndex;
                                auto descriptor = mAssetDescriptors.at(sSelectedAsset);
                                if (descriptor.GetTypeFromId() == kAssetType_Mesh) { OnSelectedMeshAsset(descriptor); }
                            }
                            ImGui::PopStyleVar(2);

                            // DRAG DROP
                            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                                auto GetPayloadType = [](const u64 id) -> const char* {
                                    const auto type = AssetDescriptor::GetTypeFromId(id);
                                    switch (type) {
                                        case kAssetType_Mesh:
                                            return X_DROP_TARGET_MESH;
                                        case kAssetType_Texture:
                                            return X_DROP_TARGET_TEXTURE;
                                        case kAssetType_Material:
                                            return X_DROP_TARGET_MATERIAL;
                                        case kAssetType_Audio:
                                            return X_DROP_TARGET_AUDIO;
                                        case kAssetType_Script:
                                            return X_DROP_TARGET_SCRIPT;
                                        default:
                                            return "\0";
                                    }
                                };

                                ImGui::SetDragDropPayload(GetPayloadType(asset.mId), &asset.mId, sizeof(u64));
                                ImGui::Text("%s", asset.mFilename.c_str());

                                ImGui::EndDragDropSource();
                            }

                            // Calculate image position (centered in the cell)
                            f32 imageStartX = ImGui::GetItemRectMin().x + (padding * 1.5f);
                            f32 imageStartY = ImGui::GetItemRectMin().y + (padding);  // Small top margin

                            // Truncate asset thumbnail text if necessary
                            str itemName = Path(asset.mFilename).Filename();
                            if (itemName.length() > 8) { itemName = itemName.substr(0, 10) + "..."; }

                            if (asset.GetTypeFromId() == kAssetType_Texture) {
                                auto thumbnail = mTextureManager.GetTexture(X_TOSTR(asset.mId));
                                X_ASSERT(thumbnail.has_value());

                                ImGui::GetWindowDrawList()->AddImage(
                                  (ImTextureID)(thumbnail->mShaderResourceView.Get()),
                                  {imageStartX, imageStartY},
                                  {imageStartX + thumbnailSize, imageStartY + thumbnailSize});
                            } else {
                                ID3D11ShaderResourceView* iconSrv {nullptr};

                                switch (asset.GetTypeFromId()) {
                                    case kAssetType_Audio: {
                                        auto icon = mTextureManager.GetTexture("AudioIcon");
                                        X_ASSERT(icon.has_value());
                                        iconSrv = icon->mShaderResourceView.Get();
                                    } break;
                                    case kAssetType_Material: {
                                        auto icon = mTextureManager.GetTexture("MaterialIcon");
                                        X_ASSERT(icon.has_value());
                                        iconSrv = icon->mShaderResourceView.Get();
                                    } break;
                                    case kAssetType_Mesh: {
                                        auto icon = mTextureManager.GetTexture("MeshIcon");
                                        X_ASSERT(icon.has_value());
                                        iconSrv = icon->mShaderResourceView.Get();
                                    } break;
                                    case kAssetType_Scene: {
                                        auto icon = mTextureManager.GetTexture("SceneIcon");
                                        X_ASSERT(icon.has_value());
                                        iconSrv = icon->mShaderResourceView.Get();
                                    } break;
                                    case kAssetType_Script: {
                                        auto icon = mTextureManager.GetTexture("ScriptIcon");
                                        X_ASSERT(icon.has_value());
                                        iconSrv = icon->mShaderResourceView.Get();
                                    } break;
                                    default:
                                        break;
                                }

                                ImGui::GetWindowDrawList()->AddImage(
                                  (ImTextureID)(iconSrv),
                                  {imageStartX, imageStartY},
                                  {imageStartX + thumbnailSize, imageStartY + thumbnailSize});
                            }

                            // Add text below the image (centered)
                            f32 textWidth  = ImGui::CalcTextSize(itemName.c_str()).x;
                            f32 textStartX = imageStartX + (thumbnailSize - textWidth) * 0.5f;
                            f32 textStartY = imageStartY + thumbnailSize + 4;  // 4 pixels below the image

                            ImGui::GetWindowDrawList()->AddText({textStartX, textStartY},
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
            }
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    void XEditor::View_Log() {
        static bool showInfo {true};
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

            ImGui::BeginChild("ScrollingRegion", {0, 0}, false, ImGuiWindowFlags_HorizontalScrollbar);

            // Display logs
            std::lock_guard<std::mutex> lock(logger.GetBufferMutex());
            const size_t startIndex =
              (logger.GetCurrentEntry() - logger.GetTotalEntries() + Logger::kMaxEntries) % Logger::kMaxEntries;
            for (size_t i = 0; i < logger.GetTotalEntries(); i++) {
                const size_t index = (startIndex + i) % Logger::kMaxEntries;
                const auto& entry  = logger.GetEntries()[index];

                // Apply filters
                if (!ShouldShowSeverity(entry.severity)) continue;

                ImGui::PushStyleColor(ImGuiCol_Text, GetLogEntryColor(entry.severity));
                ImGui::TextUnformatted(std::format("[{}] {}", entry.timestamp, entry.message).c_str());
                ImGui::PopStyleColor();
            }

            // Auto-scroll to bottom
            if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) { ImGui::SetScrollHereY(1.0f); }

            ImGui::Dummy({0, 4});

            ImGui::EndChild();
        }
        ImGui::End();
    }

    void XEditor::View_AssetPreview() {
        ImGui::Begin("Asset Preview", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        {
            if (sSelectedAsset != kNoSelection) {
                const auto& asset = mAssetDescriptors.at(sSelectedAsset);
                auto assetTypeStr = asset.GetTypeString();
                assetTypeStr[0]   = std::toupper(assetTypeStr[0]);

                const str filename  = Path(asset.mFilename).BaseName();
                const str assetText = std::format("{} ({})", filename, assetTypeStr);

                ImGui::Text(assetText.c_str());
                constexpr f32 buttonWidth = 60.0f;
                ImGui::SameLine(Gui::CalcOffset(buttonWidth));
                if (ImGui::Button("Open", {buttonWidth, 0})) {
                    // Open asset in its corresponding software
                }

                Gui::SpacingY(8.0f);
                ImGui::Separator();
                Gui::SpacingY(8.0f);

                const AssetType assetType = asset.GetTypeFromId();
                if (assetType == kAssetType_Texture) {
                    const auto preview =
                      (ImTextureID)(mTextureManager.GetTexture(X_TOSTR(asset.mId))->mShaderResourceView.Get());
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
                    ImGui::Image((ImTextureID)(srv), regionSize);
                } else if (assetType == kAssetType_Script) {
                }
            }
        }
        ImGui::End();
    }

    void XEditor::View_PostProcessing() {
        ImGui::Begin("Post Processing");
        {
            {
                Gui::ScopedFont font(mFonts["display_20"]);
                ImGui::Text("Post Processing");
            }

            Gui::SpacingY(8.0f);
        }
        ImGui::End();
    }

    void XEditor::View_Material() {
        ImGui::Begin("Material");
        {}
        ImGui::End();
    }

    void XEditor::View_Modals() {
        // View_MainMenu
        if (mSceneSelectorOpen) { ImGui::OpenPopup("Select Scene"); }
        Modal_SelectScene();

        if (mSaveSceneAsOpen) { ImGui::OpenPopup("Save Scene As"); }
        Modal_SaveSceneAs();

        if (mAboutOpen) { ImGui::OpenPopup("About"); }
        Modal_About();

        if (mNewProjectOpen) { ImGui::OpenPopup("New Project"); }
        Modal_NewProject();

        if (mCreateMaterialOpen) { ImGui::OpenPopup("Create Material"); }
        Modal_CreateMaterial();

        if (mProjectSettingsOpen) { ImGui::OpenPopup("Project Settings"); }
        Modal_ProjectSettings();

        // View_EntityProperties
        if (mAddComponentOpen) { ImGui::OpenPopup("Add Component"); }
        Modal_AddComponent();

        if (mSelectAssetOpen) { ImGui::OpenPopup("Select Asset"); }
        Modal_SelectAsset();

        // View_Entities
        if (mAddEntityOpen) { ImGui::OpenPopup("Add New Entity"); }
        Modal_AddEntity();
    }

    void XEditor::View_StatusBar() {
        constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar |            // No title bar needed
                                                 ImGuiWindowFlags_NoScrollbar |           // Disable scrolling
                                                 ImGuiWindowFlags_NoMove |                // Prevent moving
                                                 ImGuiWindowFlags_NoResize |              // Prevent resizing
                                                 ImGuiWindowFlags_NoCollapse |            // Prevent collapsing
                                                 ImGuiWindowFlags_NoSavedSettings |       // Don't save position/size
                                                 ImGuiWindowFlags_NoBringToFrontOnFocus;  // Don't change z-order

        ImGui::SetNextWindowPos({0, ImGui::GetMainViewport()->Size.y - kStatusBarHeight});
        ImGui::SetNextWindowSize({ImGui::GetMainViewport()->Size.x, kStatusBarHeight});

        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {2, 2});

        const ImVec4 background = ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, background);

        if (ImGui::Begin("##Statusbar", nullptr, windowFlags)) {
            ImGui::Dummy({2.0f, 0.0f});
            ImGui::SameLine();
            ImGui::Text("Ready");
            ImGui::End();
        }

        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
    }
#pragma endregion

#pragma region Editor Actions
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

    bool XEditor::OnCreateProject(const char* name, const char* location, const char* engineVersion) {
        const Path projectPath {location};
        if (projectPath.Exists()) {
            X_LOG_WARN("Project already exists");
            return false;
        }
        if (!projectPath.CreateAll()) {
            X_LOG_ERROR("Failed to create project directories");
            return false;
        }
        X_ASSERT(projectPath.Exists())

        // Create .xproj file
        ProjectDescriptor projectDescriptor;
        // TODO: This sucks and I hate it :( - just make these the same type bozo
        if (std::strcmp(engineVersion, "XENGINE 1.0.0") == 0) { projectDescriptor.mEngineVersion = 1; }
        projectDescriptor.mName = name;
        // TODO: Make this configurable
        projectDescriptor.mContentDirectory = "Content";  // default content directory
        projectDescriptor.mStartupScene     = "Empty";

        const str projectFileName = std::format("{}.xproj", name);
        const Path projectFile    = projectPath / projectFileName;
        if (!projectDescriptor.ToFile(projectFile)) {
            X_LOG_ERROR("Failed to save project");
            return false;
        }

        // Create content directory
        const Path contentRoot = projectPath / "Content";
        if (!contentRoot.Create()) {
            X_LOG_ERROR("Failed to create content directory");
            return false;
        }

        // Create content subdirectories
        vector<Path> subdirectories {
          contentRoot / "Audio",
          contentRoot / "Materials",
          contentRoot / "Models",
          contentRoot / "Scenes",
          contentRoot / "Scripts",
          contentRoot / "Textures",
        };
        for (const auto& subdirectory : subdirectories) {
            if (!subdirectory.Create()) { X_LOG_ERROR("Failed to create subdirectory %s", subdirectory.CStr()); }
        }

        // Create the empty scene
        SceneDescriptor emptyScene {};
        emptyScene.mName                             = "Empty";
        emptyScene.mDescription                      = "Empty scene";
        emptyScene.mWorld.mLights.mSun.mEnabled      = true;
        emptyScene.mWorld.mLights.mSun.mIntensity    = 2;
        emptyScene.mWorld.mLights.mSun.mDirection    = {0.5f, 0.5f, -0.5f};
        emptyScene.mWorld.mLights.mSun.mCastsShadows = true;
        emptyScene.mWorld.mLights.mSun.mColor        = {0.9f, 0.9f, 0.9f};

        CameraDescriptor cameraComponent {};
        cameraComponent.mNearZ        = 0.01f;
        cameraComponent.mFarZ         = 1000.0f;
        cameraComponent.mFOV          = 60.0f;
        cameraComponent.mOrthographic = false;
        cameraComponent.mWidth        = 0.0f;
        cameraComponent.mHeight       = 0.0f;

        EntityDescriptor mainCamera {};
        mainCamera.mId                  = 0;
        mainCamera.mName                = "MainCamera";
        mainCamera.mTransform.mPosition = {0.0f, 0.0f, -10.0f};
        mainCamera.mTransform.mRotation = {0.0f, 0.0f, 0.0f};
        mainCamera.mTransform.mScale    = {1.0f, 1.0f, 1.0f};
        mainCamera.mCamera              = cameraComponent;

        emptyScene.mEntities.push_back(mainCamera);

        const Path sceneFile = contentRoot / "Scenes" / "Empty.scene";
        SceneParser::WriteToFile(emptyScene, sceneFile);
        X_ASSERT(sceneFile.Exists())

        // Generate asset descriptors
        if (!AssetGenerator::GenerateAsset(sceneFile, kAssetType_Scene, contentRoot)) {
            X_LOG_ERROR("Failed to generate scene asset descriptor");
            return false;
        }

        LoadProject(projectFile.Str());
        return true;
    }

    // TODO: Refactor this system as well as the asset/scene loading to make this more robust
    void XEditor::OnSaveScene(const str& sceneName) {
        const auto contentRoot = Path(mLoadedProject.mContentDirectory);
        const Path sceneRoot   = contentRoot / "Scenes";
        X_ASSERT(contentRoot.Exists())
        X_ASSERT(sceneRoot.Exists())

        // Check whether we're overwriting the current scene or saving as new scene
        const bool overwriting = sceneName.empty();
        const str name         = overwriting ? GetCurrentScene()->GetName() : sceneName;

        // Parse the current scene state in the editor to a descriptor file
        SceneDescriptor descriptor {};
        if (!SceneParser::StateToDescriptor(GetSceneState(), descriptor, name)) {
            X_LOG_ERROR("Failed to parse state to descriptor");
            return;
        }
        X_ASSERT(descriptor.IsValid())

        // Scenes are saved to <Project>/<Content>/Scenes/<Name>.scene
        const Path scenePath = sceneRoot / (name + ".scene");
        // Write scene descriptor to file
        if (!SceneParser::WriteToFile(descriptor, scenePath)) {
            X_LOG_ERROR("Failed to write scene to file")
            return;
        }

        // If we're saving this as a new scene asset, we need to generate its descriptor file
        if (!overwriting) {
            const bool generateResult = AssetGenerator::GenerateAsset(scenePath, kAssetType_Scene, contentRoot);
            if (!generateResult) {
                X_LOG_ERROR("Failed to generate scene asset descriptor");
                return;
            }
        }

        // Reload editor caches
        ReloadAssetCache();
        mGame.ReloadSceneCache();
        OnLoadScene(name);
    }

    void XEditor::OnAddEntity(const str& name) const {
        auto& state              = GetSceneState();
        const EntityId newEntity = state.CreateEntity(name);
        if (newEntity.Valid()) {
            state.AddComponent<TransformComponent>(newEntity);
            // Scene needs to be updated or the viewport renderer freaks out
            GetCurrentScene()->Update(0.0f);
        }
    }

    void XEditor::OnCreateMaterial() {}

    void XEditor::OnResetWindow() {
        mDockspaceSetup       = false;
        mShowAssetBrowser     = true;
        mShowAssetPreview     = false;
        mShowEntities         = true;
        mShowEntityProperties = true;
        mShowLog              = true;
        mShowMaterial         = false;
        mShowPostProcessing   = true;
        mShowSceneSettings    = true;
        mShowViewport         = true;
    }

    void XEditor::OnImportEngineContent() {
        const Path contentSrc = Path::Current() / "EngineContent";
        if (!contentSrc.Exists()) {
            ShowAlert(str("Unable to load engine content, directory missing: " + contentSrc.Str()), Error);
        }

        const Path contentDest = mProjectRoot / "Content" / "EngineContent";
        if (contentDest.Exists()) {
            ShowAlert("Engine content has already been imported.", Info);
            return;
        }

        if (!contentSrc.CopyDirectory(contentDest)) {
            ShowAlert("Unable to copy engine content", Error);
            return;
        }

        vector<Path> failedToGenerate;
        for (const auto& entry : contentDest.Entries()) {
            if (entry.IsDirectory()) {
                for (const auto& file : entry.Entries()) {
                    if (!AssetGenerator::GenerateAsset(file, GetAssetTypeFromFile(file), mProjectRoot / "Content")) {
                        failedToGenerate.push_back(file);
                    }
                }
            }
        }
        if (failedToGenerate.size() > 0) {
            for (const auto& entry : failedToGenerate) {
                X_LOG_ERROR("Failed to generate asset for file: %s", entry.CStr());
            }
        }

        ReloadAssetCache();
        mGame.ReloadSceneCache();
        GenerateAssetThumbnails();

        ShowAlert("Engine content has been imported", Info);
    }

    void XEditor::OnOpenProject() {
        const auto filter  = "Project (*.xproj)|*.xproj|";
        Path initDirectory = Platform::GetPlatformDirectory(Platform::kPlatformDir_Documents) / "XENGINE Projects";
        if (!initDirectory.Exists()) { initDirectory = initDirectory.Parent(); }
        char filename[MAX_PATH];
        if (Platform::OpenFileDialog(mHwnd, initDirectory.CStr(), filter, "Open Project File", filename, MAX_PATH)) {
            LoadProject(filename);
        }
    }

    void XEditor::OnLoadScene(const str& selectedScene) {
        if (mGame.TransitionScene(selectedScene)) {
            std::strcpy(EditorState::CurrentSceneName, selectedScene.c_str());
            this->SetWindowTitle(std::format("XEditor | {}", selectedScene));
            sSelectedEntity = GetEntities().begin()->first;
            GetCurrentScene()->Update(0.0f);
        } else {
            ShowAlert("Failed to load scene: " + selectedScene, Error);
        }
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

            const Path dest = contentDir / assetFile.Filename();
            if (!assetFile.Copy(dest)) {
                X_LOG_ERROR("Failed to copy '%s'", filename);
                return;
            }

            if (!AssetGenerator::GenerateAsset(dest, assetType, rootContentDir)) {
                X_LOG_ERROR("Failed to generate asset '%s'", filename);
                return;
            }

            // AssetManager::LoadAssets(rootContentDir.Parent());
            // I might need to pass the rootContentDir.Parent, we'll see if disabling the above line affects anything,
            // or if I can just do a full reload and be fine
            ReloadAssetCache(true);
        }
    }
#pragma endregion

#pragma region Editor Utils
    SceneState& XEditor::GetSceneState() {
        return mGame.GetActiveScene()->GetState();
    }

    SceneState& XEditor::GetSceneState() const {
        return mGame.GetActiveScene()->GetState();
    }

    Scene* XEditor::GetCurrentScene() const {
        return mGame.GetActiveScene();
    }

    std::map<EntityId, str> XEditor::GetEntities() {
        return GetSceneState().GetEntities();
    }

    void XEditor::GenerateAssetThumbnails() {
        const auto& assets = mAssetDescriptors;
        for (const auto& asset : assets) {
            const auto type = asset.GetTypeFromId();
            switch (type) {
                // Texture thumbnails
                case kAssetType_Texture: {
                    auto textureFile      = Path(mLoadedProject.mContentDirectory) / asset.mFilename;
                    const auto loadResult = mTextureManager.LoadFromDDSFile(textureFile, X_TOSTR(asset.mId));
                    if (!loadResult) { ShowAlert("Failed to load texture asset with id " + X_TOSTR(asset.mId), Error); }
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
        if (mGame.IsInitialized()) { mGame.Reset(); }
        const auto projectFile = Path(filename);

        if (!mLoadedProject.FromFile(projectFile)) {
            ShowAlert("An error occurred when parsing the selected project file.", Error);
            return;
        }

        mEditorResources.Clear();
        mProjectRoot              = projectFile.Parent();
        mSession.mLastProjectPath = projectFile;
        mSession.SaveSession();
        mGame.Initialize(this, &mSceneViewport, mProjectRoot);
        ReloadAssetCache();

        // TODO: Do this asynchronously
        GenerateAssetThumbnails();

        // Load startup scene
        OnLoadScene(mLoadedProject.mStartupScene);
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

    void XEditor::ReloadAssetCache(bool fullReload) {
        if (mLoadedProject.mLoaded && mGame.IsInitialized()) {
            mAssetDescriptors.clear();
            if (fullReload) AssetManager::ReloadAssets();  // Update asset cache internally
            mAssetDescriptors = AssetManager::GetAssetDescriptors();
        }
    }

    void XEditor::SetupDockspace(const f32 yOffset) {
        const auto* imguiViewport          = ImGui::GetWindowViewport();
        constexpr ImGuiDockNodeFlags flags = ImGuiDockNodeFlags_PassthruCentralNode;

        ImGui::SetNextWindowPos({0, yOffset});
        ImGui::SetNextWindowSize({imguiViewport->Size.x, imguiViewport->Size.y - yOffset - kStatusBarHeight});
        ImGui::SetNextWindowViewport(imguiViewport->ID);

        constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                                                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                                 ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});

        if (ImGui::Begin("DockSpace", nullptr, windowFlags)) {
            const ImGuiID dockspaceId = ImGui::GetID("Editor::DockSpace");
            ImGui::DockSpace(dockspaceId, {0.0f, 0.0f}, flags);

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
                  ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Down, 0.42f, nullptr, &dockMainId);
                ImGuiID dockLeftBottomId =
                  ImGui::DockBuilderSplitNode(dockLeftId, ImGuiDir_Down, 0.5f, nullptr, &dockLeftId);
                ImGuiID dockRightBottomId =
                  ImGui::DockBuilderSplitNode(dockRightId, ImGuiDir_Down, 0.5f, nullptr, &dockRightId);

                ImGui::DockBuilderDockWindow("Scene", dockLeftId);
                ImGui::DockBuilderDockWindow("Entities", dockLeftBottomId);
                ImGui::DockBuilderDockWindow("Properties", dockRightId);
                ImGui::DockBuilderDockWindow("Viewport", dockMainId);
                ImGui::DockBuilderDockWindow("Assets", dockBottomId);
                ImGui::DockBuilderDockWindow("Log", dockBottomId);
                ImGui::DockBuilderDockWindow("Asset Preview", dockRightBottomId);
                ImGui::DockBuilderDockWindow("Post Processing", dockRightId);
                ImGui::DockBuilderDockWindow("Material", dockMainId);

                ImGui::DockBuilderFinish(imguiViewport->ID);
            }

            ImGui::End();
        }
        ImGui::PopStyleVar(3);
    }

    void XEditor::LoadEditorIcons() {
        auto Load = [&](const u8* data, size_t size, u32 width, u32 height, const char* name) {
            const auto result = mTextureManager.LoadFromMemoryCompressed(data, size, width, height, 4, name);
            if (!result) {
                X_LOG_ERROR("Failed to load editor icon '%s'", name);
                throw std::runtime_error("Failed to load editor icon '" + str(name) + "'");
            }
        };

        // Toolbar Icons
        Load(MOVEICON_BYTES, MOVEICON_COMPRESSED_SIZE, MOVEICON_WIDTH, MOVEICON_HEIGHT, "MoveIcon");
        Load(PAUSEICON_BYTES, PAUSEICON_COMPRESSED_SIZE, PAUSEICON_WIDTH, PAUSEICON_HEIGHT, "PauseIcon");
        Load(PLAYICON_BYTES, PLAYICON_COMPRESSED_SIZE, PLAYICON_WIDTH, PLAYICON_HEIGHT, "PlayIcon");
        Load(PLAYWINDOWEDICON_BYTES,
             PLAYWINDOWEDICON_COMPRESSED_SIZE,
             PLAYWINDOWEDICON_WIDTH,
             PLAYWINDOWEDICON_HEIGHT,
             "PlayWindowedIcon");
        Load(REDOICON_BYTES, REDOICON_COMPRESSED_SIZE, REDOICON_WIDTH, REDOICON_HEIGHT, "RedoIcon");
        Load(UNDOICON_BYTES, UNDOICON_COMPRESSED_SIZE, UNDOICON_WIDTH, UNDOICON_HEIGHT, "UndoIcon");
        Load(GRIDTOGGLE_BYTES, GRIDTOGGLE_COMPRESSED_SIZE, GRIDTOGGLE_WIDTH, GRIDTOGGLE_HEIGHT, "GridToggleIcon");
        Load(FOCUSSELECTED_BYTES,
             FOCUSSELECTED_COMPRESSED_SIZE,
             FOCUSSELECTED_WIDTH,
             FOCUSSELECTED_HEIGHT,
             "FocusSelectedIcon");
        Load(ROTATEICON_BYTES, ROTATEICON_COMPRESSED_SIZE, ROTATEICON_WIDTH, ROTATEICON_HEIGHT, "RotateIcon");
        Load(SCALEICON_BYTES, SCALEICON_COMPRESSED_SIZE, SCALEICON_WIDTH, SCALEICON_HEIGHT, "ScaleIcon");
        Load(SELECTICON_BYTES, SELECTICON_COMPRESSED_SIZE, SELECTICON_WIDTH, SELECTICON_HEIGHT, "SelectIcon");
        Load(STOPICON_BYTES, STOPICON_COMPRESSED_SIZE, STOPICON_WIDTH, STOPICON_HEIGHT, "StopIcon");
        Load(SELECTASSETICON_BYTES,
             SELECTASSETICON_COMPRESSED_SIZE,
             SELECTASSETICON_WIDTH,
             SELECTASSETICON_HEIGHT,
             "SelectAssetIcon");
        Load(OPENFOLDER_BYTES, OPENFOLDER_COMPRESSED_SIZE, OPENFOLDER_WIDTH, OPENFOLDER_HEIGHT, "OpenFolderIcon");
        Load(COMPILECODE_BYTES, COMPILECODE_COMPRESSED_SIZE, COMPILECODE_WIDTH, COMPILECODE_HEIGHT, "CompileCodeIcon");
        Load(CLEANCODE_BYTES, CLEANCODE_COMPRESSED_SIZE, CLEANCODE_WIDTH, CLEANCODE_HEIGHT, "CleanCodeIcon");

        // Asset Browser Icons
        Load(AUDIOICON_BYTES, AUDIOICON_COMPRESSED_SIZE, AUDIOICON_WIDTH, AUDIOICON_HEIGHT, "AudioIcon");
        Load(MATERIALICON_BYTES, MATERIALICON_COMPRESSED_SIZE, MATERIALICON_WIDTH, MATERIALICON_HEIGHT, "MaterialIcon");
        Load(MESHICON_BYTES, MESHICON_COMPRESSED_SIZE, MESHICON_WIDTH, MESHICON_HEIGHT, "MeshIcon");
        Load(SCENEICON_BYTES, SCENEICON_COMPRESSED_SIZE, SCENEICON_WIDTH, SCENEICON_HEIGHT, "SceneIcon");
        Load(SCRIPTICON_BYTES, SCRIPTICON_COMPRESSED_SIZE, SCRIPTICON_WIDTH, SCRIPTICON_HEIGHT, "ScriptIcon");
        Load(FOLDERICON_BYTES, FOLDERICON_COMPRESSED_SIZE, FOLDERICON_WIDTH, FOLDERICON_HEIGHT, "FolderIcon");
        Load(FOLDERICONEMPTY_BYTES,
             FOLDERICONEMPTY_COMPRESSED_SIZE,
             FOLDERICONEMPTY_WIDTH,
             FOLDERICONEMPTY_HEIGHT,
             "FolderEmptyIcon");

        // Logos / Branding
        Load(ABOUT_BANNER_BYTES, ABOUT_BANNER_COMPRESSED_SIZE, ABOUT_BANNER_WIDTH, ABOUT_BANNER_HEIGHT, "AboutBanner");
        Load(XEDITOR_LOGO_BYTES, XEDITOR_LOGO_COMPRESSED_SIZE, XEDITOR_LOGO_WIDTH, XEDITOR_LOGO_HEIGHT, "XEditorLogo");
        Load(BACKGROUND_LOGOS_BYTES,
             BACKGROUND_LOGOS_COMPRESSED_SIZE,
             BACKGROUND_LOGOS_WIDTH,
             BACKGROUND_LOGOS_HEIGHT,
             "BackgroundLogos");
    }

    void XEditor::RegisterEditorShortcuts() {
        mShortcutManager.RegisterShortcut(ImGuiKey_N, kModifier_Ctrl, [this]() { mNewProjectOpen = true; });
        mShortcutManager.RegisterShortcut(ImGuiKey_O, kModifier_Ctrl, [this]() { OnOpenProject(); });
        mShortcutManager.RegisterShortcut(ImGuiKey_O, kModifier_Ctrl | kModifier_Shift, [this]() {
            mSceneSelectorOpen = true;
        });
        mShortcutManager.RegisterShortcut(ImGuiKey_S, kModifier_Ctrl, [this]() { OnSaveScene(); });
        mShortcutManager.RegisterShortcut(ImGuiKey_S, kModifier_Ctrl | kModifier_Shift, [this]() {
            mSaveSceneAsOpen = true;
        });
        mShortcutManager.RegisterShortcut(ImGuiKey_S, kModifier_Ctrl | kModifier_Alt, [this]() {
            mProjectSettingsOpen = true;
        });
    }

    void XEditor::ShowAlert(const str& message, AlertSeverity severity) {
        std::strcpy(mAlertMessage, message.c_str());
        mAlertSeverity   = severity;
        mAlertDialogOpen = true;
    }

#pragma endregion
}  // namespace x