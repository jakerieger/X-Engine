// Author: Jake Rieger
// Created: 2/18/2025.
//

#pragma once

#include "EventEmitter.hpp"
#include "Common/Types.hpp"
#include "Engine/EngineCommon.hpp"
#include "Engine/RenderContext.hpp"

namespace x {
    class Window : EventEmitter {
    public:
        Window(const str& title, int width, int height);
        virtual ~Window();

        int Run();
        LRESULT Quit();

    protected:
        HINSTANCE _instance;
        HWND _hwnd;
        u32 _currentWidth, _currentHeight;
        str _title;
        RenderContext _context;

        // Final output views for this window
        ComPtr<ID3D11RenderTargetView> _renderTargetView;
        ComPtr<ID3D11DepthStencilView> _depthStencilView;
        ComPtr<ID3D11DepthStencilState> _depthStencilState;

        virtual bool Initialize();
        virtual void Shutdown();

    private:
        LRESULT ResizeHandler(u32 width, u32 height);
        LRESULT MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam);
        static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    };
}  // namespace x
