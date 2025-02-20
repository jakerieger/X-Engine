// Author: Jake Rieger
// Created: 2/18/2025.
//

#pragma once

#include "EventEmitter.hpp"
#include "Viewport.hpp"
#include "Common/Types.hpp"
#include "Engine/EngineCommon.hpp"
#include "Engine/RenderContext.hpp"

namespace x {
    class Window : public EventEmitter {
    public:
        Window(const str& title, int width, int height);
        virtual ~Window();

        int Run();
        LRESULT Quit();

        virtual void OnInitialize() {};
        virtual void OnShutdown() {};
        virtual void OnResize(u32 width, u32 height) {};

        virtual void Update() {}
        virtual void Render() {}

        X_NODISCARD u32 GetWidth() const {
            return _currentWidth;
        }

        X_NODISCARD u32 GetHeight() const {
            return _currentHeight;
        }

        X_NODISCARD HWND GetHandle() const {
            return _hwnd;
        }

    protected:
        HINSTANCE _instance;
        HWND _hwnd;
        u32 _currentWidth, _currentHeight;
        str _title;
        RenderContext _context;
        unique_ptr<Viewport> _windowViewport;

        virtual LRESULT MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam);

    private:
        bool _focused = false;

        bool Initialize();
        void Shutdown();

        LRESULT ResizeHandler(u32 width, u32 height);
        static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    };
}  // namespace x
