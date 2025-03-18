// Author: Jake Rieger
// Created: 3/11/2025.
//

#pragma once

#include "Common/Types.hpp"
#include "Engine/Window.hpp"
#include "TextureManager.hpp"
#include "Engine/Game.hpp"
#include "XPak/ProjectDescriptor.hpp"
#include "Res/resource.h"

#include <imgui.h>

#include "MeshPreviewer.hpp"

namespace x {
    struct EditorSession {
        Path mLastProjectPath;

        bool LoadSession();
        void SaveSession() const;
    };

    class XEditor final : public Window {
    public:
        // Using `this->` to denote members of parent Window class
        // Window will initialize `mContext` and since its constructor gets called before our subclass initializes its
        // members, we can use `mContext` as inputs to our Editor member variable constructors.
        XEditor()
            : Window("XEditor", 1440, 800), mTextureManager(this->mContext), mMeshPreviewer(this->mContext),
              mSceneViewport(this->mContext), mGame(this->mContext),
              mEditorResources(this->mContext, Memory::BYTES_2GB) {
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
        MeshPreviewer mMeshPreviewer;

        // Engine API
        Viewport mSceneViewport;
        Game mGame;
        ProjectDescriptor mLoadedProject {};
        vector<AssetDescriptor> mAssetDescriptors;
        ResourceManager mEditorResources;

        EditorSession mSession;

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
        void AssetPreviewView();

        // Popups
        bool mSceneSelectorOpen {false};

        // Button/menu actions
        void OnOpenProject();
        void OnLoadScene(const str& selectedScene);

        void OnImportAsset();

        // I/O
        void LoadProject(const str& filename);
        /// @brief Saves the current scene state to a descriptor file on disk (.scene)
        void SaveScene(const char* filename = nullptr) const;
        /// @brief Returns the initial directory for file dialogs when opening/saving files
        Path GetInitialDirectory() const;
        /// @brief Returns the AssetType based on the given filename/extension
        static AssetType GetAssetTypeFromFile(const Path& path);

        // ImGui-specific functions
        void SetupDockspace(const f32 yOffset);
        void ApplyTheme();
        void LoadEditorIcons();
        void GenerateAssetThumbnails();
    };
}  // namespace x
