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

        // Lifecycle callbacks
        virtual void OnInitialize() {};
        virtual void OnShutdown() {};
        virtual void OnResize(u32 width, u32 height) {};
        virtual void OnUpdate() {};
        virtual void OnRender() {};

        X_NODISCARD u32 GetWidth() const {
            return mCurrentWidth;
        }

        X_NODISCARD u32 GetHeight() const {
            return mCurrentHeight;
        }

        X_NODISCARD HWND GetHandle() const {
            return mHwnd;
        }

        void SetOpenMaximized(bool maximized) {
            mOpenMaximized = maximized;
        }

    protected:
        HINSTANCE mInstance;
        HWND mHwnd;
        u32 mCurrentWidth, mCurrentHeight;
        str mTitle;
        RenderContext mContext;
        unique_ptr<Viewport> mWindowViewport;

        virtual LRESULT MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam);
        void SetWindowTitle(const str& title) const;

    private:
        bool mFocused {false};
        bool mOpenMaximized {false};

        bool Initialize();
        void Shutdown();

        LRESULT ResizeHandler(u32 width, u32 height);
        static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    };
}  // namespace x
