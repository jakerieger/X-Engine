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
        ImVec4 mMenuBackground;
        ImVec4 mTabHeader;
        ImVec4 mPanelBackground;
        ImVec4 mButtonBackground;
        ImVec4 mInputBackground;
        ImVec4 mHeaderBackground;
        ImVec4 mTextHighlight;
        ImVec4 mTextPrimary;
        ImVec4 mTextSecondary;
        ImVec4 mSelected;
        ImVec4 mIcon;
        ImVec4 mBorder;
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

    class XEditor final : public IWindow {
    public:
        // Using `this->` to denote members of parent Window class
        // Window will initialize `mContext` and since its constructor gets called before our subclass initializes its
        // members, we can use `mContext` as inputs to our Editor member variable constructors.
        XEditor()
            : IWindow("XEditor", 1440, 800), mTextureManager(this->mContext), mMeshPreviewer(this->mContext),
              mSceneViewport(this->mContext), mGame(this->mContext), mEditorResources(this->mContext, X_GIGABYTES(2)) {
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

        void ReloadAssetCache();

        // Views/Tabs
        void View_MainMenu();
        void View_Toolbar(f32 menuBarHeight);
        void View_SceneSettings();
        void View_Entities();
        void View_EntityProperties();
        void View_Viewport();
        void View_AssetBrowser();
        void View_Log();
        void View_AssetPreview();
        void View_PostProcessing();
        void View_Material();

        // View toggles
        bool mShowSceneSettings {true};
        bool mShowEntities {true};
        bool mShowEntityProperties {true};
        bool mShowViewport {true};
        bool mShowAssetBrowser {true};
        bool mShowLog {true};
        bool mShowAssetPreview {true};
        bool mShowPostProcessing {true};
        bool mShowMaterial {false};

        // Popups
        bool mSceneSelectorOpen {false};
        bool mSelectAssetOpen {false};
        AssetType mSelectAssetFilter {kAssetType_Invalid};
        bool mAddComponentOpen {false};
        bool mSaveSceneAsOpen {false};
        bool mAboutOpen {false};
        bool mNewProjectOpen {false};
        bool mCreateMaterialOpen {false};

        void Modal_SelectScene();
        void Modal_SaveSceneAs();
        void Modal_AddComponent();
        void Modal_SelectAsset();
        void Modal_About();
        void Modal_NewProject();
        void Modal_CreateMaterial();

        // Button/menu actions
        void OnOpenProject();
        void OnLoadScene(const str& selectedScene);
        void OnSelectedMeshAsset(const AssetDescriptor& descriptor);
        void OnImportAsset();
        void OnSaveScene(const char* name = nullptr);
        void OnAddEntity(const str& name) const;
        void OnResetWindow();
        void OnImportEngineContent();
        void OnCreateMaterial();

        // Helpers
        SceneState& GetSceneState();
        SceneState& GetSceneState() const;
        Scene* GetCurrentScene() const;
        std::map<EntityId, str> GetEntities();

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
