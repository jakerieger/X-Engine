// Author: Jake Rieger
// Created: 2/18/2025.
//

#pragma once

#include "PropertiesPanel.hpp"
#include "TextureManager.hpp"
#include "Common/Types.hpp"
#include "Engine/Game.hpp"
#include "Engine/Window.hpp"
#include "XPak/ProjectDescriptor.hpp"

namespace x::Editor {
    struct EditorFiles {
        vector<Filesystem::Path> mAssets;
        unordered_map<AssetId, AssetDescriptor> mAssetDescriptors;
    };

    class EditorWindow final : public Window {
    public:
        EditorWindow()
            : Window("XEditor", 1600, 900), mCurrentProject(), mSceneViewport(mContext), mTextureManager(mContext),
              mGame(mContext), mPropertiesPanel(*this) {
            SetOpenMaximized(true);
        }

        bool LoadTextures();
        void OnInitialize() override;
        void OnResize(u32 width, u32 height) override;
        void OnShutdown() override;

        void Update() override;
        void Render() override;

        X_NODISCARD bool InPlayMode() const {
            return mGameRunning;
        }

        LRESULT MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) override;

        void OpenScene(const char* filename);
        void TogglePlayMode();
        void NewScene();
        void OpenProject(const char* filename);

    private:
        EditorFiles mEditorFiles;
        ProjectDescriptor mCurrentProject;

        bool mLayoutSetup {false};
        Viewport mSceneViewport;
        TextureManager mTextureManager;
        unordered_map<str, ImFont*> mFonts;
        ImFont* mDefaultFont {nullptr};
        unique_ptr<VertexShader> mOutlineVS;
        unique_ptr<PixelShader> mOutlinePS;

        Game mGame;
        bool mGameRunning {false};
        Camera mEditorCamera;
        Camera mSceneCamera;

        unordered_map<str, EntityId> mEntities;
        EntityId mSelectedEntity;

        // UI Panels
        friend PropertiesPanel;
        PropertiesPanel mPropertiesPanel;

        TextEditor mTextEditor;

        void MainMenu();
        void SetupDockspace(f32 yOffset);
        void SceneView();
        void ScriptingView();
        void MaterialView();
        void EntitiesView();
        void WorldSettingsView();
        void PropertiesView();
        void AssetsView();
        void EditorLogView();

        void UpdateWindowTitle(const str& title) const;
        static void ApplyTheme();
    };
}  // namespace x::Editor
