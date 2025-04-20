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
#include "Engine/Color.hpp"
#include "Engine/DeferredActionQueue.hpp"

namespace x {
    struct EditorSession {
        Path mLastProjectPath;

        bool LoadSession();
        void SaveSession() const;
    };

    struct EditorTheme {
        str mName;
        Color mBorder;
        Color mButtonBackground;
        Color mHeaderBackground;
        Color mIcon;
        Color mInputBackground;
        Color mMenuBackground;
        Color mPanelBackground;
        Color mSelected;
        Color mTabHeader;
        Color mTextHighlight;
        Color mTextPrimary;
        Color mTextSecondary;
        Color mWindowBackground;
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

    struct EditorState {};

    class XEditor final : public IWindow {
    public:
        // Using `this->` to denote members of parent Window class.
        // Window will initialize `mContext` and since its constructor gets called before our subclass initializes its
        // members, we can use `mContext` as inputs to our Editor member variable constructors.
        XEditor()
            : IWindow("XEditor", 1440, 800), mMeshPreviewer(this->mContext), mTextureManager(this->mContext),
              mGame(this->mContext), mEditorResources(this->mContext, 2_GIGABYTES), mSceneViewport(this->mContext) {
            this->SetOpenMaximized(true);
        }

        void OnInitialize() override;
        void OnResize(u32 width, u32 height) override;
        void OnShutdown() override;
        void OnUpdate() override;
        void OnRender() override;
        LRESULT MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) override;

        static bool HasSession();

    private:
        // UI / ImGui
        ImFont* mDefaultFont {nullptr};
        MeshPreviewer mMeshPreviewer;
        Path mProjectRoot {};
        TextureManager mTextureManager;
        bool mDockspaceSetup {false};
        unordered_map<str, ImFont*> mFonts;
        DeferredActionQueue mPostRenderQueue;

        // Engine API
        Game mGame;
        ProjectDescriptor mLoadedProject {};
        ResourceManager mEditorResources;
        Viewport mSceneViewport;
        vector<AssetDescriptor> mAssetDescriptors;

        // Editor structures
        EditorSession mSession;
        EditorSettings mSettings;
        EditorTheme mTheme;

        // Views/Tabs
        void View_AssetBrowser();
        void View_AssetPreview();
        void View_Entities();
        void View_EntityProperties();
        void View_Log();
        void View_MainMenu();
        void View_Material();
        void View_Modals();
        void View_PostProcessing();
        void View_SceneSettings();
        void View_StartupScreen(f32 yOffset);
        void View_StatusBar();
        void View_Toolbar(f32 menuBarHeight);
        void View_Viewport();

        // View toggles
        bool mShowAssetBrowser {true};
        bool mShowAssetPreview {false};
        bool mShowEntities {true};
        bool mShowEntityProperties {true};
        bool mShowLog {true};
        bool mShowMaterial {false};
        bool mShowPostProcessing {true};
        bool mShowSceneSettings {true};
        bool mShowViewport {true};

        // Popups
        AssetType mSelectAssetFilter {kAssetType_Invalid};
        bool mAboutOpen {false};
        bool mAddComponentOpen {false};
        bool mCreateMaterialOpen {false};
        bool mNewProjectOpen {false};
        bool mSaveSceneAsOpen {false};
        bool mSceneSelectorOpen {false};
        bool mSelectAssetOpen {false};
        bool mAddEntityOpen {false};

        void Modal_About();
        void Modal_AddComponent();
        void Modal_CreateMaterial();
        void Modal_NewProject();
        void Modal_SaveSceneAs();
        void Modal_SelectAsset();
        void Modal_SelectScene();
        void Modal_AddEntity();

        // Button/menu actions
        void OnAddEntity(const str& name) const;
        void OnCreateMaterial();
        void OnImportAsset();
        void OnImportEngineContent();
        void OnLoadScene(const str& selectedScene);
        void OnOpenProject();
        void OnResetWindow();
        void OnSaveScene(const str& sceneName = "");
        void OnSelectedMeshAsset(const AssetDescriptor& descriptor);
        bool OnCreateProject(const char* name, const char* location, const char* engineVersion);

        // Helpers
        Scene* GetCurrentScene() const;
        SceneState& GetSceneState() const;
        SceneState& GetSceneState();
        std::map<EntityId, str> GetEntities();

        // I/O
        void LoadProject(const str& filename);
        /// @brief Returns the initial directory for file dialogs when opening/saving files
        Path GetInitialDirectory() const;
        /// @brief Returns the AssetType based on the given filename/extension
        static AssetType GetAssetTypeFromFile(const Path& path);

        /// @brief Reloads the editor's asset cache. If `fullReload` is true,
        /// this will tell the engine to do a cache reload internally as well.
        void ReloadAssetCache(bool fullReload = false);

        // ImGui-specific functions
        bool LoadEditorIcons();
        void GenerateAssetThumbnails();
        void SetupDockspace(const f32 yOffset);
    };
}  // namespace x
