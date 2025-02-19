// Author: Jake Rieger
// Created: 2/18/2025.
//

#pragma once

#include "Common/Types.hpp"
#include "Engine/EngineCommon.hpp"
#include "Engine/RenderContext.hpp"

namespace x::Editor {
    class Editor {
        X_CLASS_PREVENT_MOVES_COPIES(Editor)

    public:
        Editor();
        ~Editor();

        int Run();
        LRESULT Quit();

    private:
        HINSTANCE _instance;
        HWND _hwnd;
        u32 _currentWidth, _currentHeight;
        str _title;
        RenderContext _context;

        bool Initialize();
        void Shutdown();

        LRESULT ResizeHandler(u32 width, u32 height);
        LRESULT MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam);
        static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    };
}  // namespace x::Editor
