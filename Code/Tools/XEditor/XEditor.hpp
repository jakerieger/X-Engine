// Author: Jake Rieger
// Created: 3/11/2025.
//

#pragma once

#include "Common/Types.hpp"
#include "Engine/Window.hpp"
#include "Engine/SceneParser.hpp"
#include <imgui.h>

#include "TextureManager.hpp"
#include "Engine/Game.hpp"
#include "XPak/ProjectDescriptor.hpp"

namespace x {
    class XEditor final : public Window {
    public:
        XEditor() : Window("XEditor", 1440, 800), mTextureManager(mContext), mSceneViewport(mContext), mGame(mContext) {
            this->SetOpenMaximized(true);
        }

        void OnInitialize() override;
        void OnResize(u32 width, u32 height) override;
        void OnShutdown() override;
        void OnUpdate() override;
        void OnRender() override;

        LRESULT MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) override;

    private:
        // UI / ImGui
        ImFont* mDefaultFont {nullptr};
        unordered_map<str, ImFont*> mFonts;
        Path mLoadedScenePath {};
        Path mProjectRoot {};
        bool mDockspaceSetup {false};
        TextureManager mTextureManager;

        // Engine API
        Viewport mSceneViewport;
        Game mGame;
        ProjectDescriptor mLoadedProject {};
        vector<AssetDescriptor> mAssetDescriptors;

        // ImGui settings variables.
        // These are used to store editable data that is then used to update the current scene
        struct SceneSettings {
            char mName[128] {0};
            char mDesc[512] {0};
        } mSceneSettings;

        struct EntityProperties {
            char mName[256] {0};
        } mEntityProperties;

        void UpdateAssetDescriptors();

        // Views/Tabs
        void MainMenu();
        void SceneSettingsView();
        void EntitiesView();
        void EntitiesPropertiesView();
        void ViewportView();
        void AssetsView();

        // Popups
        bool mSceneSelectorOpen {false};

        // Button/menu actions
        void OnOpenProject();
        void OnLoadScene(const str& selectedScene);

        // I/O
        void LoadProject(const str& filename);
        void SaveScene(const char* filename = nullptr) const;

        // ImGui-specific functions
        void SetupDockspace(const f32 yOffset);
        void ApplyTheme();
        void LoadEditorIcons();
        void GenerateAssetThumbnails();
    };
}  // namespace x
