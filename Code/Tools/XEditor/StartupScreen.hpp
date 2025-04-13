// Author: Jake Rieger
// Created: 4/12/2025.
//

#pragma once

#include <imgui.h>
#include "Common/Types.hpp"
#include "Engine/Window.hpp"

namespace x {

    class StartupScreen final : public IWindow {
    public:
        StartupScreen() : IWindow("XEditor", 1000, 800) {
            this->SetOpenMaximized(false);
        }

        void OnInitialize() override;
        void OnResize(u32 width, u32 height) override;
        void OnShutdown() override;
        void OnUpdate() override;
        void OnRender() override;

        LRESULT MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) override;

    private:
        ImFont* mDefaultFont {nullptr};
        unordered_map<str, ImFont*> mFonts;
    };

}  // namespace x
