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

#include <Inter.h>
#include <JetBrainsMono.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <imgui_internal.h>
#include <yaml-cpp/yaml.h>

#include "XEditor.hpp"
#include "Res/resource.h"
#include "Controls.hpp"
#include "Utilities.hpp"
#include "ImGuiHelpers.hpp"
#include "Common/FileDialogs.hpp"
#include "Common/WindowsHelpers.hpp"
#include "Engine/SceneParser.hpp"
#include "Engine/EngineCommon.hpp"
#include "XPak/AssetGenerator.hpp"

#pragma region Embedded Resources
#include "AssetBrowserIcons.h"
#include "ToolbarIcons.h"
#include "Logos.h"
#include "WelcomeScreenIcons.h"
#pragma endregion

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace x {
#pragma region Global Constants
    static constexpr ImVec2 kViewportSize {1024, 576};  // Default viewport size
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
        if (!FileWriter::WriteAllText(sessionFile, out.c_str())) { X_LOG_ERROR("Failed to save editor session"); }
    }
#pragma endregion

#pragma region EditorTheme
    bool EditorTheme::LoadTheme(const str& theme) {
        const auto themeFile = Path::Current() / "Themes" / (theme + ".yaml");
        if (themeFile.Exists()) {
            YAML::Node themeNode = YAML::LoadFile(themeFile.Str());

            mName             = themeNode["name"].as<str>();
            mWindowBackground = HexToImVec4(themeNode["windowBackground"].as<str>());
            mMenuBackground   = HexToImVec4(themeNode["menuBackground"].as<str>());
            mTabHeader        = HexToImVec4(themeNode["tabHeader"].as<str>());
            mPanelBackground  = HexToImVec4(themeNode["panelBackground"].as<str>());
            mButtonBackground = HexToImVec4(themeNode["buttonBackground"].as<str>());
            mInputBackground  = HexToImVec4(themeNode["inputBackground"].as<str>());
            mHeaderBackground = HexToImVec4(themeNode["headerBackground"].as<str>());
            mTextHighlight    = HexToImVec4(themeNode["textHighlight"].as<str>());
            mTextPrimary      = HexToImVec4(themeNode["textPrimary"].as<str>());
            mTextSecondary    = HexToImVec4(themeNode["textSecondary"].as<str>());
            mSelected         = HexToImVec4(themeNode["selected"].as<str>());
            mIcon             = HexToImVec4(themeNode["icon"].as<str>());
            mBorder           = HexToImVec4(themeNode["border"].as<str>());
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
        colors[ImGuiCol_Border]                = mBorder;
        colors[ImGuiCol_ButtonActive]          = ColorWithOpacity(mButtonBackground, 0.67f);
        colors[ImGuiCol_ButtonHovered]         = ColorWithOpacity(mButtonBackground, 0.80f);
        colors[ImGuiCol_Button]                = mButtonBackground;
        colors[ImGuiCol_CheckMark]             = mIcon;
        colors[ImGuiCol_ChildBg]               = mPanelBackground;
        colors[ImGuiCol_DockingPreview]        = mSelected;
        colors[ImGuiCol_DragDropTarget]        = mSelected;
        colors[ImGuiCol_FrameBgActive]         = mInputBackground;
        colors[ImGuiCol_FrameBgHovered]        = mInputBackground;
        colors[ImGuiCol_FrameBg]               = mInputBackground;
        colors[ImGuiCol_HeaderActive]          = ColorWithOpacity(mHeaderBackground, 0.67f);
        colors[ImGuiCol_HeaderHovered]         = ColorWithOpacity(mHeaderBackground, 0.80f);
        colors[ImGuiCol_Header]                = mHeaderBackground;
        colors[ImGuiCol_MenuBarBg]             = mMenuBackground;
        colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.00f, 0.00f, 0.00f, 0.5f);
        colors[ImGuiCol_NavHighlight]          = ImVec4(30.f / 255.f, 30.f / 255.f, 30.f / 255.f, 1.00f);
        colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        colors[ImGuiCol_PlotLines]             = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        colors[ImGuiCol_PopupBg]               = mWindowBackground;
        colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        colors[ImGuiCol_ResizeGrip]            = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
        colors[ImGuiCol_ScrollbarBg]           = mMenuBackground;
        colors[ImGuiCol_ScrollbarGrabActive]   = mIcon;
        colors[ImGuiCol_ScrollbarGrabHovered]  = mIcon;
        colors[ImGuiCol_ScrollbarGrab]         = mIcon;
        colors[ImGuiCol_SeparatorActive]       = mSelected;
        colors[ImGuiCol_SeparatorHovered]      = mSelected;
        colors[ImGuiCol_Separator]             = mWindowBackground;
        colors[ImGuiCol_SliderGrabActive]      = mIcon;
        colors[ImGuiCol_SliderGrab]            = mIcon;
        colors[ImGuiCol_TabActive]             = mHeaderBackground;
        colors[ImGuiCol_TabHovered]            = mHeaderBackground;
        colors[ImGuiCol_TabUnfocusedActive]    = colors[ImGuiCol_TabActive];
        colors[ImGuiCol_TabUnfocused]          = colors[ImGuiCol_Tab];
        colors[ImGuiCol_Tab]                   = mTabHeader;
        colors[ImGuiCol_TableBorderLight]      = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);  // Prefer using Alpha=1.0 here
        colors[ImGuiCol_TableBorderLight]      = ImVec4(0.f, 0.f, 0.f, 0.f);
        colors[ImGuiCol_TableBorderStrong]     = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);  // Prefer using Alpha=1.0 here
        colors[ImGuiCol_TableBorderStrong]     = ImVec4(0.f, 0.f, 0.f, 0.f);
        colors[ImGuiCol_TableHeaderBg]         = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
        colors[ImGuiCol_TableRowBgAlt]         = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
        colors[ImGuiCol_TableRowBg]            = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_TextDisabled]          = mTextSecondary;
        colors[ImGuiCol_TextSelectedBg]        = ColorWithOpacity(mSelected, 0.5f);
        colors[ImGuiCol_Text]                  = mTextHighlight;
        colors[ImGuiCol_TitleBgActive]         = mTabHeader;
        colors[ImGuiCol_TitleBgCollapsed]      = mTabHeader;
        colors[ImGuiCol_TitleBg]               = mTabHeader;
        colors[ImGuiCol_WindowBg]              = mWindowBackground;
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
        FileWriter::WriteAllText(settingsFile, out.c_str());
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
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

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

        mWindowViewport->SetClearColor(0.05f, 0.05f, 0.05f, 1.0f);  // Editor background nearly black by default
        mSceneViewport.SetClearColor(Colors::Gray);                 // Viewport background grey by default
        // Resize to 1x1 initially so D3D creation code doesn't fail (0x0 invalid resource size)
        mSceneViewport.Resize(1, 1);

        if (mSession.LoadSession()) { LoadProject(mSession.mLastProjectPath.Str()); }
        if (!mSettings.LoadSettings()) { mSettings.SaveSettings(); }
        mTheme.LoadTheme(mSettings.mTheme);
        mTheme.Apply();
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

        ImGui::PopFont();

        mWindowViewport->AttachViewport();
        mWindowViewport->BindRenderTarget();
        ImGui::Render();

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
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
        if (ImGui::BeginPopupModal("Save Scene As", &mSaveSceneAsOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::SetNextItemWidth(408.0f);
            ImGui::InputText("##scene_name_as", EditorState::CurrentSceneName, sizeof(EditorState::CurrentSceneName));

            ImGui::Dummy({0, 2});

            if (ImGui::Button("OK", {200, 0})) {
                if (EditorState::CurrentSceneName[0] != '\0' || strcmp(EditorState::CurrentSceneName, "") == 0) {
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
    }

    void XEditor::Modal_AddComponent() {
        auto& state = GetSceneState();

        using ComponentMap             = unordered_map<str, str>;
        static ComponentMap components = {
          {"Model", "Renders a 3D model/mesh, making it visible in the scene"},
          {"Behavior", "Custom Lua script that defines unique behaviors and functionality"}};

        static str selectedComponent;
        static constexpr f32 itemHeight = 48.0f;

        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, {0.5f, 0.5f});
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

            // Buttons
            if (ImGui::Button("OK", {240, 0})) {
                if (selectedComponent == "Model") {
                    state.AddComponent<ModelComponent>(sSelectedEntity);
                } else if (selectedComponent == "Behavior") {
                    state.AddComponent<BehaviorComponent>(sSelectedEntity);
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
    }

    void XEditor::Modal_SelectAsset() {
        if (ImGui::BeginPopupModal("Select Asset", &mSelectAssetOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
            // Collect all available assets of the current filter type
            vector<AssetDescriptor> availableAssets;
            std::ranges::copy_if(
              mAssetDescriptors,
              std::back_inserter(availableAssets),
              [this](const AssetDescriptor& desc) { return desc.GetTypeFromId() == mSelectAssetFilter; });

            // Assets list
            static u64 selectedAssetId {0};
            for (const auto& desc : availableAssets) {
                if (Gui::SelectableWithHeaders(std::format("##{}_asset_select", desc.mId).c_str(),
                                               desc.mFilename.c_str(),
                                               std::to_string(desc.mId).c_str(),
                                               desc.mId == selectedAssetId,
                                               0,
                                               {408, 48})) {
                    selectedAssetId = desc.mId;
                }
            }

            // OK and Cancel buttons
            if (ImGui::Button("OK", {200, 0})) {
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
    }

    void XEditor::Modal_About() {
        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, {0.5f, 0.5f});
        if (ImGui::BeginPopupModal("About", &mAboutOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
            const auto banner = mTextureManager.GetTexture("AboutBanner");
            ImGui::Image(SrvToTextureId(banner->mShaderResourceView.Get()), {800, 300});
            ImGui::EndPopup();
        }
    }

    void XEditor::Modal_NewProject() {
        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, {0.5f, 0.5f});
        ImGui::SetNextWindowSize({600, 0});
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {20.0f, 20.0f});

        static char nameBuffer[256] {0};
        static char locationBuffer[512] {0};
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
                Gui::ScopedColorVars colors({{ImGuiCol_Separator, HexToImVec4("5e5e5e")}});
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
            if (ImGui::Button("##select_location", {selectLocationButtonWidth, 0})) {}

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
            {
                Gui::ScopedColorVars colors({{ImGuiCol_Button, HexToImVec4("1a97b8")},
                                             {ImGuiCol_ButtonActive, ColorWithOpacity(HexToImVec4("1a97b8"), 0.67f)},
                                             {ImGuiCol_ButtonHovered, ColorWithOpacity(HexToImVec4("1a97b8"), 0.8f)},
                                             {ImGuiCol_Text, HexToImVec4("FFFFFF")}});
                if (ImGui::Button("Create Project##new_project", {createButtonWidth, buttonHeight})) {
                    if (OnCreateProject(nameBuffer, locationBuffer, engineVersions[selectedVersion])) {
                        mNewProjectOpen = false;
                    }
                }
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
        if (ImGui::BeginPopupModal("Create Material", &mCreateMaterialOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Material Type");
            ImGui::SetNextItemWidth(408);
            if (ImGui::Combo("##material_type_combo", &selectedType, materialTypes, std::size(materialTypes))) {
                selectedType = selectedType % std::size(materialTypes);
            }
            ImGui::SetNextItemWidth(408);
            ImGui::InputText("##material_name", nameBuffer, sizeof(nameBuffer));

            if (ImGui::Button("OK", {200, 0})) {
                mCreateMaterialOpen = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", {200, 0})) {
                mCreateMaterialOpen = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    void XEditor::Modal_SelectScene() {
        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, {0.5f, 0.5f});
        if (ImGui::BeginPopupModal("Select Scene", &mSceneSelectorOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
            static str selectedScene;

            u32 index {0};
            for (const auto& [name, desc] : mGame.GetSceneMap()) {
                str id          = "##" + std::to_string(index) + "scene_select";  // ex. ##0_select_scene
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

            // Buttons
            if (ImGui::Button("OK", {200, 0})) {
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
    }
#pragma endregion

#pragma region Editor Views
    void XEditor::View_MainMenu() {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, mTheme.mTextPrimary);

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New Project", "Ctrl+N")) { mNewProjectOpen = true; }
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
                if (ImGui::BeginMenu("Create")) {
                    if (ImGui::MenuItem("Material")) {
                        mCreateMaterialOpen = true;
                        if (!mShowMaterial) { mShowMaterial = true; }
                    }
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

        ImGui::PopStyleVar();
        ImGui::PopStyleColor();

        if (mSceneSelectorOpen) { ImGui::OpenPopup("Select Scene"); }
        Modal_SelectScene();

        if (mSaveSceneAsOpen) { ImGui::OpenPopup("Save Scene As"); }
        Modal_SaveSceneAs();

        if (mAboutOpen) { ImGui::OpenPopup("About"); }
        Modal_About();

        // TODO: This will render twice when on the welcome screen, disabled for now
        // if (mNewProjectOpen) { ImGui::OpenPopup("New Project"); }
        // Modal_NewProject();

        if (mCreateMaterialOpen) { ImGui::OpenPopup("Create Material"); }
        Modal_CreateMaterial();
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

        if (ImGui::Begin("##Toolbar", nullptr, toolbarFlags)) {
            const auto playIcon  = SrvToTextureId(mTextureManager.GetTexture("PlayIcon")->mShaderResourceView.Get());
            const auto pauseIcon = SrvToTextureId(mTextureManager.GetTexture("PauseIcon")->mShaderResourceView.Get());
            const auto stopIcon  = SrvToTextureId(mTextureManager.GetTexture("StopIcon")->mShaderResourceView.Get());

            static constexpr ImVec2 btnSize = {24, 24};

            // Move the next 3 buttons to the window center
            // Total width is (24*3) + (2*3) or (72) + (6) or 78

            auto windowWidth        = ImGui::GetWindowWidth();
            auto middleButtonsWidth = (btnSize.x * 3) + 40;  // Calculated the '+ 40' using photoshop, no clue how it's
                                                             // derived, but I'll be damned if it ain't centered
            auto xOffset = (windowWidth / 2) - (middleButtonsWidth / 2);

            ImGui::SameLine(xOffset);

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::ImageButton("##play_btn",
                               playIcon,
                               btnSize,
                               {0, 0},
                               {1, 1},
                               ImVec4(0, 0, 0, 0),
                               ImGui::GetStyleColorVec4(ImGuiCol_CheckMark));
            ImGui::SameLine();  // 4
            ImGui::ImageButton("##pause_btn",
                               pauseIcon,
                               btnSize,
                               {0, 0},
                               {1, 1},
                               ImVec4(0, 0, 0, 0),
                               ImGui::GetStyleColorVec4(ImGuiCol_CheckMark));
            ImGui::SameLine();  // 4
            ImGui::ImageButton("##stop_btn",
                               stopIcon,
                               btnSize,
                               {0, 0},
                               {1, 1},
                               ImVec4(0, 0, 0, 0),
                               ImGui::GetStyleColorVec4(ImGuiCol_CheckMark));
            ImGui::PopStyleColor();

            ImGui::End();
        }

        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
    }

    void XEditor::View_SceneSettings() {
        ImGui::Begin("Scene");
        {
            if (mGame.IsInitialized() && GetCurrentScene()->Loaded()) {
                auto& state = GetSceneState();

                const f32 width = ImGui::GetContentRegionAvail().x;

                if (ImGui::CollapsingHeader("World", ImGuiTreeNodeFlags_DefaultOpen)) {
                    // Sun
                    auto& sun = state.GetLightState().mSun;
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
                    Gui::DragFloatNColored("##sun_direction",
                                           (f32*)&sun.mDirection,
                                           3,
                                           0.01f,
                                           0.01f,
                                           1.0f,
                                           "%.3f",
                                           1.0f);

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
                ImGui::Image(SrvToTextureId(bgLogos), {size.x, size.y - yOffset});

                // Draw title image
                ID3D11ShaderResourceView* titleImage =
                  mTextureManager.GetTexture("XEditorLogo")->mShaderResourceView.Get();
                ImGui::SetCursorPos(titleCursorPos);
                ImGui::Image(SrvToTextureId(titleImage), titleSize);

                // Button icons
                ID3D11ShaderResourceView* controllerIcon =
                  mTextureManager.GetTexture("ControllerIcon")->mShaderResourceView.Get();
                ID3D11ShaderResourceView* folderIcon =
                  mTextureManager.GetTexture("FolderOutlineIcon")->mShaderResourceView.Get();
                ID3D11ShaderResourceView* cogIcon = mTextureManager.GetTexture("CogIcon")->mShaderResourceView.Get();

                Gui::ScopedStyleVars buttonVars(
                  {{ImGuiStyleVar_FrameRounding, 16.0f}, {ImGuiStyleVar_FrameBorderSize, 4.0f}});
                Gui::ScopedColorVars buttonColors({{ImGuiCol_Border, HexToImVec4("353535")},
                                                   {ImGuiCol_Button, HexToImVec4("161616")},
                                                   {ImGuiCol_ButtonActive, HexToImVec4("222222")},
                                                   {ImGuiCol_ButtonHovered, HexToImVec4("1F1F1F")}});

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

        if (mNewProjectOpen) {
            ImGui::OpenPopup("New Project");
            Modal_NewProject();
        }
    }

    void XEditor::View_Entities() {
        ImGui::Begin("Entities");
        const ImVec2 windowSize = ImGui::GetContentRegionAvail();

        // Define sizes with adjustment for borders and scrollbar appearance
        constexpr f32 buttonHeight    = 24.0f;
        constexpr f32 buttonPadding   = 4.0f;
        constexpr f32 totalButtonArea = buttonHeight + (buttonPadding * 2);
        // Add a small adjustment factor to prevent scrollbar appearance
        constexpr f32 heightAdjustment = 4.0f;
        const f32 listHeight           = windowSize.y - totalButtonArea - heightAdjustment;

        ImGui::BeginChild("##entities_scroll_list", {windowSize.x, listHeight}, true);
        {
            if (mGame.IsInitialized() && GetCurrentScene()->Loaded()) {
                for (auto& [id, name] : GetEntities()) {
                    if (ImGui::Selectable(name.c_str(), id == sSelectedEntity)) { sSelectedEntity = id; }
                }
            }
        }
        ImGui::EndChild();

        ImGui::Dummy({0, buttonPadding});
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {buttonPadding, buttonPadding});
        const ImVec2 remainingSpace = ImGui::GetContentRegionAvail();

        static char entityNameBuffer[256] {0};
        static bool openAddEntityPopup {false};
        if (ImGui::Button("Add Entity", {remainingSpace.x, buttonHeight})) {
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
            if (ImGui::Button("OK", {150, 0}) || enterPressed) {
                if (strlen(entityNameBuffer) > 0) {
                    ImGui::CloseCurrentPopup();
                    OnAddEntity(entityNameBuffer);
                }
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel", {150, 0})) { ImGui::CloseCurrentPopup(); }

            ImGui::EndPopup();
        }

        ImGui::End();
    }

    void XEditor::View_EntityProperties() {
        static const ImTextureID selectAssetIcon =
          SrvToTextureId(mTextureManager.GetTexture("ScaleIcon").value().mShaderResourceView.Get());
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
                              const MaterialDescriptor desc = MaterialParser::Parse(*assetBytes);
                              const auto mat                = GetCurrentScene()->LoadMaterial(desc);
                              X_ASSERT(mat.get() != nullptr)
                              model->SetMaterial(mat);
                              model->SetMaterialId(descriptor.mId);

                              // TODO: You currently can't create more than one entity with a model component or
                              // Material->Bind throws

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
                        ImGui::Text("Script (Asset):");
                        ImGui::SameLine(kLabelWidth);
                        ImGui::SetNextItemWidth(size.x - kLabelWidth);

                        const auto& scriptAsset =
                          std::ranges::find_if(mAssetDescriptors, [&behavior](const AssetDescriptor& asset) {
                              return asset.mId == behavior->GetScriptId();
                          });
                        if (scriptAsset != mAssetDescriptors.end()) {
                            auto UpdateScript = [this, &scriptAsset](const AssetDescriptor& descriptor) {
                                const auto oldId = scriptAsset->mId;
                                if (oldId == descriptor.mId) { return; }

                                // TODO: Implement

                                X_LOG_DEBUG("Old script ID: %llu", oldId);
                                X_LOG_DEBUG("New script ID: %llu", descriptor.mId);
                            };

                            static char buffer[256] {0};
                            const auto currentValue = Path(scriptAsset->mFilename).Filename();
                            std::strcpy(buffer, currentValue.c_str());

                            if (Gui::AssetDropTarget("##script_drop_target",
                                                     buffer,
                                                     sizeof(buffer),
                                                     selectAssetIcon,
                                                     X_DROP_TARGET_SCRIPT,
                                                     UpdateScript)) {
                                mSelectAssetOpen   = true;
                                mSelectAssetFilter = kAssetType_Script;
                            }
                        }
                    }
                }

                static f32 fovDegrees {45.0f};
                static f32 nearZ {0.01f};
                static f32 farZ {1000.0f};
                static bool orthographic {false};
                static Float2 viewport {100.0f, 100.0f};

                if (camera) {
                    fovDegrees   = camera->GetFOVDegrees();
                    nearZ        = camera->GetNearPlane();
                    farZ         = camera->GetFarPlane();
                    orthographic = camera->GetOrthographic();
                    viewport.x   = camera->GetWidth();
                    viewport.y   = camera->GetHeight();

                    Gui::SpacingY(10.0f);
                    if (ImGui::CollapsingHeader("Camera##properties", ImGuiTreeNodeFlags_DefaultOpen)) {
                        ImGui::Text("FOV:");
                        ImGui::SameLine(kLabelWidth);
                        ImGui::SetNextItemWidth(size.x - kLabelWidth);
                        ImGui::InputFloat("##fov_properties", &fovDegrees, 1.0f, 5.0f, "%.1f");

                        ImGui::Text("Near Plane:");
                        ImGui::SameLine(kLabelWidth);
                        ImGui::SetNextItemWidth(size.x - kLabelWidth);
                        ImGui::InputFloat("##nearz_properties", &nearZ, 0.1f, 1.0f, "%.2f");

                        ImGui::Text("Far Plane:");
                        ImGui::SameLine(kLabelWidth);
                        ImGui::SetNextItemWidth(size.x - kLabelWidth);
                        ImGui::InputFloat("##farz_properties", &farZ, 0.1f, 1.0f, "%.2f");

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
                        }
                    }

                    camera->SetFOVDegrees(fovDegrees);
                    camera->SetNearPlane(nearZ);
                    camera->SetFarPlane(farZ);
                    camera->SetOrthographic(orthographic);
                    camera->SetWidth(viewport.x);
                    camera->SetHeight(viewport.y);
                }
            }

            ImGui::Dummy({0, 2.f});
            ImGui::Separator();
            ImGui::Dummy({0, 2.f});

            if (ImGui::Button("Add Component##button", {size.x, 0})) { mAddComponentOpen = true; }
        }
        ImGui::End();

        if (mAddComponentOpen) { ImGui::OpenPopup("Add Component"); }
        if (mSelectAssetOpen) { ImGui::OpenPopup("Select Asset"); }

        Modal_AddComponent();
        Modal_SelectAsset();
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
            ImGui::Image(SrvToTextureId(srv), contentSize);
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    void XEditor::View_AssetBrowser() {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});
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

                ImGui::Dummy({0, padding * 2});

                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {4.0f, 4.0f});
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
                                auto thumbnail = mTextureManager.GetTexture(std::to_string(asset.mId));
                                X_ASSERT(thumbnail.has_value());

                                ImGui::GetWindowDrawList()->AddImage(
                                  SrvToTextureId(thumbnail->mShaderResourceView.Get()),
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
                                  SrvToTextureId(iconSrv),
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

            ImGui::BeginChild("ScrollingRegion", {0, 0}, false, ImGuiWindowFlags_HorizontalScrollbar);

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

                ImGui::Text("Source: %s", asset.mFilename.c_str());
                ImGui::Text("ID: %llu", asset.mId);
                ImGui::Text("Type: %s", assetTypeStr.c_str());
                ImGui::Separator();
                ImGui::Separator();

                const AssetType assetType = asset.GetTypeFromId();
                if (assetType == kAssetType_Texture) {
                    const auto preview =
                      SrvToTextureId(mTextureManager.GetTexture(std::to_string(asset.mId))->mShaderResourceView.Get());
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
                    ImGui::Image(SrvToTextureId(srv), regionSize);
                } else if (assetType == kAssetType_Script) {
                }
            }
        }
        ImGui::End();
    }

    void XEditor::View_PostProcessing() {
        ImGui::Begin("Post Processing");
        {}
        ImGui::End();
    }

    void XEditor::View_Material() {
        ImGui::Begin("Material");
        {}
        ImGui::End();
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
        SceneParser::WriteToFile(emptyScene, sceneFile.Str());
        X_ASSERT(sceneFile.Exists())

        // Generate asset descriptors
        if (!AssetGenerator::GenerateAsset(sceneFile, kAssetType_Scene, contentRoot)) {
            X_LOG_ERROR("Failed to generate scene asset descriptor");
            return false;
        }

        LoadProject(projectFile.Str());
        return true;
    }

    void XEditor::OnSaveScene(const char* name) {
        const auto* scene = GetCurrentScene();
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
            Platform::ShowAlert(mHwnd,
                                "Error",
                                str("Unable to load engine content, directory missing: " + contentSrc.Str()).c_str(),
                                Platform::AlertSeverity::Error);
        }

        const Path contentDest = mProjectRoot / "Content" / "EngineContent";
        if (contentDest.Exists()) {
            Platform::ShowAlert(mHwnd,
                                "XEditor",
                                "Engine content has already been imported.",
                                Platform::AlertSeverity::Info);
            return;
        }

        if (!contentSrc.CopyDirectory(contentDest)) {
            Platform::ShowAlert(mHwnd, "Error", "Unable to copy engine content", Platform::AlertSeverity::Error);
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

        AssetManager::ReloadAssets();
        ReloadAssetCache();
        mGame.ReloadSceneCache();
        GenerateAssetThumbnails();

        Platform::ShowAlert(mHwnd, "XEditor", "Engine content has been imported", Platform::AlertSeverity::Info);
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
            // TODO: Make sure any previously loaded projects get properly unloaded or it'll just crash
            LoadProject(filename);
        }
    }

    void XEditor::OnLoadScene(const str& selectedScene) {
        mGame.TransitionScene(selectedScene);
        std::strcpy(EditorState::CurrentSceneName, selectedScene.c_str());
        this->SetWindowTitle(std::format("XEditor | {}", selectedScene));
        sSelectedEntity = GetEntities().begin()->first;
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

            AssetManager::LoadAssets(rootContentDir.Parent());
            ReloadAssetCache();
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

    void XEditor::ReloadAssetCache() {
        if (mLoadedProject.mLoaded && mGame.IsInitialized()) {
            mAssetDescriptors.clear();
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

    bool XEditor::LoadEditorIcons() {
        // Asset Browser Icons
        auto result = mTextureManager.LoadFromMemoryCompressed(AUDIOICON_BYTES,
                                                               AUDIOICON_COMPRESSED_SIZE,
                                                               AUDIOICON_WIDTH,
                                                               AUDIOICON_HEIGHT,
                                                               4,
                                                               "AudioIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load Audio icon");
            return false;
        }
        result = mTextureManager.LoadFromMemoryCompressed(MATERIALICON_BYTES,
                                                          MATERIALICON_COMPRESSED_SIZE,
                                                          MATERIALICON_WIDTH,
                                                          MATERIALICON_HEIGHT,
                                                          4,
                                                          "MaterialIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load Material icon");
            return false;
        }
        result = mTextureManager.LoadFromMemoryCompressed(MESHICON_BYTES,
                                                          MESHICON_COMPRESSED_SIZE,
                                                          MESHICON_WIDTH,
                                                          MESHICON_HEIGHT,
                                                          4,
                                                          "MeshIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load Mesh icon");
            return false;
        }
        result = mTextureManager.LoadFromMemoryCompressed(SCENEICON_BYTES,
                                                          SCENEICON_COMPRESSED_SIZE,
                                                          SCENEICON_WIDTH,
                                                          SCENEICON_HEIGHT,
                                                          4,
                                                          "SceneIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load Scene icon");
            return false;
        }
        result = mTextureManager.LoadFromMemoryCompressed(SCRIPTICON_BYTES,
                                                          SCRIPTICON_COMPRESSED_SIZE,
                                                          SCRIPTICON_WIDTH,
                                                          SCRIPTICON_HEIGHT,
                                                          4,
                                                          "ScriptIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load Script icon");
            return false;
        }
        result = mTextureManager.LoadFromMemoryCompressed(FOLDERICON_BYTES,
                                                          FOLDERICON_COMPRESSED_SIZE,
                                                          FOLDERICON_WIDTH,
                                                          FOLDERICON_HEIGHT,
                                                          4,
                                                          "FolderIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load Folder icon");
            return false;
        }
        result = mTextureManager.LoadFromMemoryCompressed(FOLDERICONEMPTY_BYTES,
                                                          FOLDERICONEMPTY_COMPRESSED_SIZE,
                                                          FOLDERICONEMPTY_WIDTH,
                                                          FOLDERICONEMPTY_HEIGHT,
                                                          4,
                                                          "FolderEmptyIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load Folder icon");
            return false;
        }

        // Toolbar / Editor Icons
        result = mTextureManager.LoadFromMemoryCompressed(MOVEICON_BYTES,
                                                          MOVEICON_COMPRESSED_SIZE,
                                                          MOVEICON_WIDTH,
                                                          MOVEICON_HEIGHT,
                                                          4,
                                                          "MoveIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load Move icon");
            return false;
        }
        result = mTextureManager.LoadFromMemoryCompressed(PAUSEICON_BYTES,
                                                          PAUSEICON_COMPRESSED_SIZE,
                                                          PAUSEICON_WIDTH,
                                                          PAUSEICON_HEIGHT,
                                                          4,
                                                          "PauseIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load Pause icon");
            return false;
        }
        result = mTextureManager.LoadFromMemoryCompressed(PLAYICON_BYTES,
                                                          PLAYICON_COMPRESSED_SIZE,
                                                          PLAYICON_WIDTH,
                                                          PLAYICON_HEIGHT,
                                                          4,
                                                          "PlayIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load Play icon");
            return false;
        }
        result = mTextureManager.LoadFromMemoryCompressed(REDOICON_BYTES,
                                                          REDOICON_COMPRESSED_SIZE,
                                                          REDOICON_WIDTH,
                                                          REDOICON_HEIGHT,
                                                          4,
                                                          "RedoIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load Redo icon");
            return false;
        }
        result = mTextureManager.LoadFromMemoryCompressed(UNDOICON_BYTES,
                                                          UNDOICON_COMPRESSED_SIZE,
                                                          UNDOICON_WIDTH,
                                                          UNDOICON_HEIGHT,
                                                          4,
                                                          "UndoIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load Undo icon");
            return false;
        }
        result = mTextureManager.LoadFromMemoryCompressed(ROTATEICON_BYTES,
                                                          ROTATEICON_COMPRESSED_SIZE,
                                                          ROTATEICON_WIDTH,
                                                          ROTATEICON_HEIGHT,
                                                          4,
                                                          "RotateIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load Rotate icon");
            return false;
        }
        result = mTextureManager.LoadFromMemoryCompressed(SCALEICON_BYTES,
                                                          SCALEICON_COMPRESSED_SIZE,
                                                          SCALEICON_WIDTH,
                                                          SCALEICON_HEIGHT,
                                                          4,
                                                          "ScaleIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load Scale icon");
            return false;
        }
        result = mTextureManager.LoadFromMemoryCompressed(SELECTICON_BYTES,
                                                          SELECTICON_COMPRESSED_SIZE,
                                                          SELECTICON_WIDTH,
                                                          SELECTICON_HEIGHT,
                                                          4,
                                                          "SelectIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load Select icon");
            return false;
        }
        result = mTextureManager.LoadFromMemoryCompressed(STOPICON_BYTES,
                                                          STOPICON_COMPRESSED_SIZE,
                                                          STOPICON_WIDTH,
                                                          STOPICON_HEIGHT,
                                                          4,
                                                          "StopIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load Stop icon");
            return false;
        }
        // result = mTextureManager.LoadFromMemoryCompressed(SELECTASSETICON_BYTES, 24, 24, 4, "SelectAssetIcon");
        // if (!result) {
        //     X_LOG_ERROR("Failed to load Stop icon");
        //     return false;
        // }

        result = mTextureManager.LoadFromMemoryCompressed(ABOUT_BANNER_BYTES,
                                                          ABOUT_BANNER_COMPRESSED_SIZE,
                                                          ABOUT_BANNER_WIDTH,
                                                          ABOUT_BANNER_HEIGHT,
                                                          4,
                                                          "AboutBanner");
        if (!result) {
            X_LOG_ERROR("Failed to load AboutBanner");
            return false;
        }

        result = mTextureManager.LoadFromMemoryCompressed(XEDITOR_LOGO_BYTES,
                                                          XEDITOR_LOGO_COMPRESSED_SIZE,
                                                          XEDITOR_LOGO_WIDTH,
                                                          XEDITOR_LOGO_HEIGHT,
                                                          4,
                                                          "XEditorLogo");
        if (!result) {
            X_LOG_ERROR("Failed to load XEditorLogo");
            return false;
        }

        result = mTextureManager.LoadFromMemoryCompressed(BACKGROUND_LOGOS_BYTES,
                                                          BACKGROUND_LOGOS_COMPRESSED_SIZE,
                                                          BACKGROUND_LOGOS_WIDTH,
                                                          BACKGROUND_LOGOS_HEIGHT,
                                                          4,
                                                          "BackgroundLogos");
        if (!result) {
            X_LOG_ERROR("Failed to load BackgroundLogos");
            return false;
        }

        // Welcome screen icons
        result = mTextureManager.LoadFromMemoryCompressed(CONTROLLER_BYTES,
                                                          CONTROLLER_COMPRESSED_SIZE,
                                                          CONTROLLER_WIDTH,
                                                          CONTROLLER_HEIGHT,
                                                          4,
                                                          "ControllerIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load ControllerIcon");
            return false;
        }
        result = mTextureManager.LoadFromMemoryCompressed(FOLDER_OUTLINE_BYTES,
                                                          FOLDER_OUTLINE_COMPRESSED_SIZE,
                                                          FOLDER_OUTLINE_WIDTH,
                                                          FOLDER_OUTLINE_HEIGHT,
                                                          4,
                                                          "FolderOutlineIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load FolderOutlineIcon");
            return false;
        }
        result =
          mTextureManager.LoadFromMemoryCompressed(COG_BYTES, COG_COMPRESSED_SIZE, COG_WIDTH, COG_HEIGHT, 4, "CogIcon");
        if (!result) {
            X_LOG_ERROR("Failed to load CogIcon");
            return false;
        }

        return true;
    }
#pragma endregion
}  // namespace x