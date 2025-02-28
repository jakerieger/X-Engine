// Author: Jake Rieger
// Created: 2/18/2025.
//

#pragma once

#include "PropertiesPanel.hpp"
#include "TextureManager.hpp"
#include "Common/Types.hpp"
#include "Engine/Game.hpp"
#include "Engine/Window.hpp"

namespace x::Editor {
    struct EditorFiles {
        vector<Filesystem::Path> mAssets;
        vector<Filesystem::Path> mMaterials;
        vector<Filesystem::Path> mScripts;
    };

    class EditorWindow final : public Window {
    public:
        EditorWindow()
            : Window("XEditor", 1600, 900), mSceneViewport(mContext), mTextureManager(mContext), mGame(mContext),
              mPropertiesPanel(*this) {}

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

        bool mLayoutSetup {false};
        Viewport mSceneViewport;
        TextureManager mTextureManager;
        unordered_map<str, ImFont*> mFonts;
        ImFont* mDefaultFont {None};
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
