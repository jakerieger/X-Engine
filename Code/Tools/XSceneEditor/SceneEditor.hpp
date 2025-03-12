// Author: Jake Rieger
// Created: 3/11/2025.
//

#pragma once

#include "Common/Types.hpp"
#include "Engine/Window.hpp"
#include "Engine/SceneParser.hpp"
#include <imgui.h>

namespace x {
    class SceneEditor final : public Window {
    public:
        SceneEditor() : Window("XSceneEditor", 1440, 800) {
            // this->SetOpenMaximized(true);
        }

        void OnInitialize() override;
        void OnResize(u32 width, u32 height) override;
        void OnShutdown() override;

        void Update() override;
        void Render() override;

        LRESULT MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) override;

    private:
        ImFont* mDefaultFont {nullptr};
        unordered_map<str, ImFont*> mFonts;
        SceneDescriptor mLoadedScene {};
        Path mLoadedScenePath {};
        Path mContentRootPath {};
        bool mDockspaceSetup {false};

        struct SceneSettings {
            char mName[128] {0};
            char mDesc[512] {0};
        } mSceneSettings;

        void MainMenu();
        void SceneSettingsView();
        void EntitiesView();
        void EntitiesPropertiesView();

        void LoadScene(const str& filename);
        void SaveScene() const;
        void SaveSceneAs(const str& filename) const;

        void SetupDockspace(const f32 yOffset);
        void ApplyTheme();
    };
}  // namespace x
