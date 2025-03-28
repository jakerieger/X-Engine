// Author: Jake Rieger
// Created: 3/11/2025.
//

#pragma once

#include <imgui.h>

#include "MeshPreviewer.hpp"
#include "TextureManager.hpp"

#include "Common/Types.hpp"

#include "Engine/Window.hpp"
#include "Engine/Game.hpp"

#include "XPak/ProjectDescriptor.hpp"

namespace x {
    struct EditorSession {
        Path mLastProjectPath;

        bool LoadSession();
        void SaveSession() const;
    };

    struct EditorTheme {
        str mName;
        ImVec4 mWindowBackground;
        ImVec4 mChildBackground;
        ImVec4 mFrameBackground;
        ImVec4 mAltBackground;
        ImVec4 mHeaderBackground;
        ImVec4 mTextHighlight;
        ImVec4 mTextPrimary;
        ImVec4 mTextSecondary;
        ImVec4 mTextDisabled;
        ImVec4 mBorder;
        ImVec4 mError;
        ImVec4 mWarning;
        ImVec4 mSuccess;
        ImVec4 mLink;
        ImVec4 mScrollbar;
        ImVec4 mPrimary;
        ImVec4 mSecondary;
        f32 mBorderRadius;
        f32 mBorderWidth;

        bool LoadTheme(const str& theme);
        void SaveTheme() const;
        void Apply() const;
    };

    struct EditorSettings {
        str mTheme {"Dark"};

        bool LoadSettings();
        void SaveSettings() const;
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

        // Editor structures
        EditorSession mSession;
        EditorSettings mSettings;
        EditorTheme mTheme;

        // ImGui settings variables.
        // These are used to store editable data that is then used to update the current scene
        // TODO: Remove
        struct SceneSettings {
            char mName[128] {0};
            char mDesc[512] {0};
        } mSceneSettings;

        // TODO: Remove
        struct EntityProperties {
            char mName[256] {0};
        } mEntityProperties;

        void ReloadAssetCache();

        // Views/Tabs
        void View_MainMenu();
        void View_SceneSettings();
        void View_Entities();
        void View_EntityProperties();
        void View_Viewport();
        void View_AssetBrowser();
        void View_Log();
        void View_AssetPreview();

        // Popups
        bool mSceneSelectorOpen {false};
        bool mSelectAssetOpen {false};
        AssetType mSelectAssetFilter {kAssetType_Invalid};
        bool mAddComponentOpen {false};
        bool mSaveSceneAsOpen {false};
        bool mAboutOpen {false};

        void Modal_SelectScene();
        void Modal_SaveSceneAs();
        void Modal_AddComponent();
        void Modal_SelectAsset();
        void Modal_About();

        // Button/menu actions
        void OnOpenProject();
        void OnLoadScene(const str& selectedScene);
        void OnSelectedMeshAsset(const AssetDescriptor& descriptor);
        void OnImportAsset();
        void OnSaveScene(const char* name = nullptr);
        void OnAddEntity(const str& name) const;

        // I/O
        void LoadProject(const str& filename);
        /// @brief Returns the initial directory for file dialogs when opening/saving files
        Path GetInitialDirectory() const;
        /// @brief Returns the AssetType based on the given filename/extension
        static AssetType GetAssetTypeFromFile(const Path& path);

        // ImGui-specific functions
        void SetupDockspace(const f32 yOffset);
        bool LoadEditorIcons();
        void GenerateAssetThumbnails();
    };
}  // namespace x
