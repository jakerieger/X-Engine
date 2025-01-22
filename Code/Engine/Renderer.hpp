#pragma once

#include "Common/Types.hpp"
#include "D3D.hpp"
#include "Volatile.hpp"

namespace x {
    class Renderer : public Volatile {
        ComPtr<IDXGISwapChain> _swapChain;
        ComPtr<ID3D11Device> _device;
        ComPtr<ID3D11DeviceContext> _context;
        ComPtr<ID3D11Texture2D> _backBuffer;
        ComPtr<ID3D11RenderTargetView> _renderTargetView;
        ComPtr<ID3D11DepthStencilView> _depthStencilView;
        ComPtr<ID3D11DepthStencilState> _depthStencilState;

    public:
        Renderer() = default;

        // Prevent moves or copies
        Renderer(const Renderer& other)            = delete;
        Renderer(Renderer&& other)                 = delete;
        Renderer& operator=(const Renderer& other) = delete;
        Renderer& operator=(Renderer&& other)      = delete;

        [[nodiscard]] ID3D11Device* GetDevice() const {
            return _device.Get();
        }

        [[nodiscard]] ID3D11DeviceContext* GetContext() const {
            return _context.Get();
        }

        [[nodiscard]] ID3D11Texture2D* GetBackBuffer() const {
            return _backBuffer.Get();
        }

        bool Initialize(HWND hwnd, int width, int height);
        void BeginFrame();
        void BeginFrame(const f32 clearColor[4]);
        void EndFrame();

        void OnResize(u32 width, u32 height) override;

    private:
        void ResizeSwapchainBuffers(u32 width, u32 height);
    };
}