// Author: Jake Rieger
// Created: 3/11/2025.
//

#include "SceneEditor.hpp"
#include "Common/Platform/FileDialogs.hpp"
#include <Inter.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <imgui_internal.h>
#include <yaml-cpp/yaml.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace x {
    static constexpr f32 kLabelWidth    = 140.0f;
    static constexpr i64 kInvalidEntity = -1;
    static i64 sSelectedEntity {kInvalidEntity};

    static ImTextureID SrvAsTextureId(ID3D11ShaderResourceView* srv) {
        return RCAST<ImTextureID>(RCAST<void*>(srv));
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

        color.x = (f32)r / 255.0f;
        color.y = (f32)g / 255.0f;
        color.z = (f32)b / 255.0f;
        color.w = alpha;

        return color;
    }

    void SceneEditor::OnInitialize() {
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

        ApplyTheme();

        mWindowViewport->SetClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        mSceneViewport.SetClearColor(0.7f, 0.7f, 0.7f, 1.0f);
        mSceneViewport.Resize(1, 1);

        ImGui_ImplWin32_Init(mHwnd);
        ImGui_ImplDX11_Init(mContext.GetDevice(), mContext.GetDeviceContext());

        SetWindowTitle("XSceneEditor | Untitled");
    }

    void SceneEditor::OnResize(u32 width, u32 height) {
        Window::OnResize(width, height);
    }

    void SceneEditor::OnShutdown() {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    void SceneEditor::Update() {
        Window::Update();
    }

    void SceneEditor::Render() {
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

        ImGui::PopFont();

        mWindowViewport->AttachViewport();
        mWindowViewport->BindRenderTarget();
        ImGui::Render();

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    LRESULT SceneEditor::MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) {
        if (ImGui_ImplWin32_WndProcHandler(mHwnd, msg, wParam, lParam)) return true;
        return Window::MessageHandler(msg, wParam, lParam);
    }

    void SceneEditor::MainMenu() {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Open Project", "Ctrl+O")) {
                    const char* filter = "Project (*.xproj)|*.xproj|";
                    char filename[MAX_PATH];
                    if (Platform::OpenFileDialog(mHwnd, nullptr, filter, "Open Project File", filename, MAX_PATH)) {
                        LoadProject(filename);
                    }
                }
                ImGui::Separator();
                // if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
                //     // Do new scene action
                // }
                if (ImGui::MenuItem("Open Scene", "Ctrl+Shift+O", false, mLoadedProject.mLoaded)) {
                    mSceneSelectorOpen = true;
                }
                // if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
                //     if (Platform::ShowAlert(mHwnd,
                //                             "Save Scene",
                //                             "You are about to overwrite the current scene. Do you wish to continue?",
                //                             Platform::AlertSeverity::Question) == IDYES &&
                //         !mLoadedScenePath.Exists()) {
                //         SaveScene();
                //     }
                // }
                // if (ImGui::MenuItem("Save Scene As", "Ctrl+Shift+S")) {
                //     const char* filter = "Scene (*.scene)|*.scene|";
                //     char filename[MAX_PATH];
                //     if (Platform::SaveFileDialog(mHwnd,
                //                                  mLoadedScenePath.Parent().CStr(),
                //                                  filter,
                //                                  "Save Scene File",
                //                                  "scene",
                //                                  filename,
                //                                  MAX_PATH)) {
                //         SaveSceneAs(filename);
                //         LoadScene(filename);
                //     }
                // }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit", "Alt+F4")) { this->Quit(); }
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
                    mDockspaceSetup = false;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
        ImGui::PopStyleVar();

        // Select Scene dialog
        {
            if (mSceneSelectorOpen) { ImGui::OpenPopup("Select Scene"); }

            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            if (ImGui::BeginPopupModal("Select Scene", &mSceneSelectorOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
                static str selectedScene;

                for (const auto& [name, desc] : mGame.GetSceneMap()) {
                    if (ImGui::Selectable(name.c_str(), selectedScene == name, true, ImVec2(300, 36))) {
                        selectedScene = name;
                    }
                }

                // Buttons
                if (ImGui::Button("OK", ImVec2(150, 0))) {
                    if (!selectedScene.empty()) {
                        mLoadedScene = mGame.GetSceneMap().at(selectedScene);
                        mGame.TransitionScene(selectedScene);
                    }

                    mSceneSelectorOpen = false;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SetItemDefaultFocus();
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(150, 0))) {
                    mSceneSelectorOpen = false;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }
    }

    void SceneEditor::SceneSettingsView() {
        ImGui::Begin("Scene");
        // TODO: This resets the current loaded scene for some reason
        // TODO: Change mLoadedScene to a ptr that points directly into the scene map of the game
        // instead of copying it into its own member var separate from the scene actually being used by the engine.
        {
            if (mLoadedScene.IsValid()) {
                const f32 width = ImGui::GetContentRegionAvail().x;
                ImGui::Text("Name: ");
                ImGui::SameLine(kLabelWidth);
                ImGui::SetNextItemWidth(width - kLabelWidth);
                ImGui::InputText("##scene_name", mSceneSettings.mName, sizeof(mSceneSettings.mName));

                ImGui::Text("Description: ");
                ImGui::SameLine(kLabelWidth);
                ImGui::SetNextItemWidth(width - kLabelWidth);
                ImGui::InputText("##scene_desc", mSceneSettings.mDesc, sizeof(mSceneSettings.mDesc));

                ImGui::Spacing();

                if (ImGui::CollapsingHeader("World", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Text("Camera");
                    ImGui::Spacing();
                    ImGui::Text("Position:");
                    ImGui::SameLine(kLabelWidth);
                    ImGui::SetNextItemWidth(width - kLabelWidth);
                    ImGui::DragFloat3("##camera_pos", (f32*)&mLoadedScene.mWorld.mCamera.mPosition, 0.01f);

                    ImGui::Text("Eye:");
                    ImGui::SameLine(kLabelWidth);
                    ImGui::SetNextItemWidth(width - kLabelWidth);
                    ImGui::DragFloat3("##camera_eye", (f32*)&mLoadedScene.mWorld.mCamera.mEye, 0.01f);

                    ImGui::Text("FOV Y:");
                    ImGui::SameLine(kLabelWidth);
                    ImGui::SetNextItemWidth(width - kLabelWidth);
                    ImGui::SliderFloat("##camera_fov", &mLoadedScene.mWorld.mCamera.mFovY, 1.0f, 120.0f, "%.1f");

                    ImGui::Text("Near Z:");
                    ImGui::SameLine(kLabelWidth);
                    ImGui::SetNextItemWidth(width - kLabelWidth);
                    ImGui::InputFloat("##camera_nearz", &mLoadedScene.mWorld.mCamera.mNearZ, 0.1f, 100.0f, "%.1f");

                    ImGui::Text("Far Z:");
                    ImGui::SameLine(kLabelWidth);
                    ImGui::SetNextItemWidth(width - kLabelWidth);
                    ImGui::InputFloat("##camera_farz", &mLoadedScene.mWorld.mCamera.mFarZ, 0.1f, 100.0f, "%.1f");

                    ImGui::Spacing();
                    ImGui::Spacing();

                    // Sun
                    auto& sun = mLoadedScene.mWorld.mLights.mSun;
                    ImGui::Text("Sun");
                    ImGui::Spacing();

                    ImGui::Text("Enabled:");
                    ImGui::SameLine(kLabelWidth);
                    ImGui::Checkbox("##sun_enabled", &sun.mEnabled);

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
                    ImGui::Checkbox("##sun_casts_shadows", &sun.mCastsShadows);
                }

                // mLoadedScene.mName        = mSceneSettings.mName;
                // mLoadedScene.mDescription = mSceneSettings.mDesc;
            } else {
                ImGui::Text("No scene loaded. Go to \"File->Open Scene\" to load a scene.");
            }
        }
        ImGui::End();
    }

    void SceneEditor::EntitiesView() {
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
            if (mLoadedScene.IsValid()) {
                for (auto& entity : mLoadedScene.mEntities) {
                    if (ImGui::Selectable(entity.mName.c_str(), entity.mId == sSelectedEntity)) {
                        sSelectedEntity = entity.mId;
                        std::strcpy(mEntityProperties.mName, entity.mName.c_str());
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

        ImGui::PushStyleColor(ImGuiCol_PopupBg, HexToImVec4("36363a"));
        if (ImGui::BeginPopupModal("Add New Entity", &openAddEntityPopup, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Entity name:");
            ImGui::PushItemWidth(300.0f);
            const bool enterPressed = ImGui::InputText("##new_entity_name",
                                                       entityNameBuffer,
                                                       sizeof(entityNameBuffer),
                                                       ImGuiInputTextFlags_EnterReturnsTrue);
            ImGui::PopItemWidth();
            ImGui::Separator();
            if (ImGui::Button("Confirm", ImVec2(150, 0)) || enterPressed) {
                if (strlen(entityNameBuffer) > 0) {
                    EntityDescriptor descriptor {};
                    descriptor.mId   = mLoadedScene.mEntities.size();
                    descriptor.mName = entityNameBuffer;
                    mLoadedScene.mEntities.push_back(descriptor);

                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel", ImVec2(150, 0))) { ImGui::CloseCurrentPopup(); }

            ImGui::EndPopup();
        }
        ImGui::PopStyleColor();

        ImGui::End();
    }

    void SceneEditor::EntitiesPropertiesView() {
        ImGui::Begin("Properties");
        const ImVec2 size = ImGui::GetContentRegionAvail();
        {
            if (sSelectedEntity != kInvalidEntity) {
                auto& entity = mLoadedScene.mEntities[sSelectedEntity];

                if (ImGui::CollapsingHeader("General##properties", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Text("Name:");
                    ImGui::SameLine(kLabelWidth);
                    ImGui::SetNextItemWidth(size.x - kLabelWidth);
                    const bool enterPressed = ImGui::InputText("##entity_name_input",
                                                               mEntityProperties.mName,
                                                               sizeof(mEntityProperties.mName),
                                                               ImGuiInputTextFlags_EnterReturnsTrue);
                    if (enterPressed) { entity.mName = mEntityProperties.mName; }
                }

                if (ImGui::CollapsingHeader("Transform##properties", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Text("Position:");
                    ImGui::SameLine(kLabelWidth);
                    ImGui::SetNextItemWidth(size.x - kLabelWidth);
                    ImGui::DragFloat3("##entity_transform_position", (f32*)&entity.mTransform.mPosition);

                    ImGui::Text("Rotation:");
                    ImGui::SameLine(kLabelWidth);
                    ImGui::SetNextItemWidth(size.x - kLabelWidth);
                    ImGui::DragFloat3("##entity_transform_rotation", (f32*)&entity.mTransform.mRotation);

                    ImGui::Text("Scale:");
                    ImGui::SameLine(kLabelWidth);
                    ImGui::SetNextItemWidth(size.x - kLabelWidth);
                    ImGui::DragFloat3("##entity_transform_scale", (f32*)&entity.mTransform.mScale);
                }

                if (entity.mModel.has_value()) {
                    auto& model = *entity.mModel;
                    if (ImGui::CollapsingHeader("Model##properties", ImGuiTreeNodeFlags_DefaultOpen)) {
                        ImGui::Text("Mesh (Asset):");
                        ImGui::SameLine(kLabelWidth);
                        ImGui::Button(std::to_string(model.mMeshId).c_str(), ImVec2(size.x - kLabelWidth, 0));

                        ImGui::Text("Material (Asset):");
                        ImGui::SameLine(kLabelWidth);
                        ImGui::Button(std::to_string(model.mMaterialId).c_str(), ImVec2(size.x - kLabelWidth, 0));

                        ImGui::Text("Casts Shadows:");
                        ImGui::SameLine(kLabelWidth);
                        ImGui::SetNextItemWidth(size.x - kLabelWidth);
                        ImGui::Checkbox("##entity_model_casts_shadows", &model.mCastsShadows);

                        ImGui::Text("Receives Shadows:");
                        ImGui::SameLine(kLabelWidth);
                        ImGui::SetNextItemWidth(size.x - kLabelWidth);
                        ImGui::Checkbox("##entity_model_receives_shadows", &model.mReceiveShadows);
                    }
                }

                if (entity.mBehavior.has_value()) {
                    auto& behavior = *entity.mBehavior;
                    if (ImGui::CollapsingHeader("Behavior##properties", ImGuiTreeNodeFlags_DefaultOpen)) {
                        ImGui::Text("Script (Asset):");
                        ImGui::SameLine(kLabelWidth);
                        ImGui::SetNextItemWidth(size.x - kLabelWidth);
                        ImGui::Button(std::to_string(behavior.mScriptId).c_str(), ImVec2(size.x - kLabelWidth, 0));
                    }
                }
            }
        }
        ImGui::End();
    }

    void SceneEditor::ViewportView() {
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
            if (mLoadedScene.IsValid() && mLoadedProject.mLoaded && mGame.IsInitialized()) {
                mGame.Resize(contentWidth, contentHeight);
                mGame.RenderFrame();
            }

            auto* srv = mSceneViewport.GetShaderResourceView().Get();
            ImGui::Image(SrvAsTextureId(srv), contentSize);
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    void SceneEditor::AssetsView() {
        ImGui::Begin("Assets");
        {
            if (mLoadedProject.mLoaded && mProjectRoot.Exists()) {}
        }
        ImGui::End();
    }

    void SceneEditor::LoadProject(const str& filename) {
        if (!mLoadedProject.FromFile(filename)) {
            Platform::ShowAlert(mHwnd,
                                "Error loading project",
                                "An error occurred when parsing the selected project file.",
                                Platform::AlertSeverity::Error);
            return;
        }

        mProjectRoot = Path(filename).Parent();
        mGame.Initialize(this, &mSceneViewport, mProjectRoot);
    }

    void SceneEditor::LoadScene(const str& filename) {
        const auto currentScene = mLoadedScene;
        mLoadedScene            = SceneDescriptor {};
        SceneParser::Parse(filename, mLoadedScene);

        if (mLoadedScene.IsValid()) {
            mLoadedScenePath = Path(filename);

            // Update UI input values
            std::strcpy(mSceneSettings.mName, mLoadedScene.mName.c_str());
            std::strcpy(mSceneSettings.mDesc, mLoadedScene.mDescription.c_str());
            mGame.TransitionScene(mLoadedScene);
        } else {
            Platform::ShowAlert(mHwnd,
                                "Error loading scene",
                                "An unknown error occurred when loading the selected scene",
                                Platform::AlertSeverity::Error);
            mLoadedScene = currentScene;
        }

        SetWindowTitle(std::format("XSceneEditor | {}", mLoadedScenePath.Str()));
    }

    void SceneEditor::SaveScene() const {
        SceneParser::WriteToFile(mLoadedScene, mLoadedScenePath.Str());
    }

    void SceneEditor::SaveSceneAs(const str& filename) const {
        SceneParser::WriteToFile(mLoadedScene, filename);
    }

    void SceneEditor::SetupDockspace(const f32 yOffset) {
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

                ImGui::DockBuilderDockWindow("Scene", dockLeftId);
                ImGui::DockBuilderDockWindow("Entities", dockLeftBottomId);
                ImGui::DockBuilderDockWindow("Properties", dockRightId);
                ImGui::DockBuilderDockWindow("Viewport", dockMainId);
                ImGui::DockBuilderDockWindow("Assets", dockBottomId);

                ImGui::DockBuilderFinish(imguiViewport->ID);
            }

            ImGui::End();
        }

        ImGui::PopStyleVar(3);
    }

    void SceneEditor::ApplyTheme() {
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
}  // namespace x