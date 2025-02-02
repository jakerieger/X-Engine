#pragma once

#include "Common/Types.hpp"
#include "Common/Panic.hpp"
#include "D3D.hpp"
#include "Volatile.hpp"

namespace x {
    struct DeviceInfo {
        str vendor;
        str model;
        size_t videoMemoryInBytes;
        size_t sharedMemoryInBytes;
    };

    struct FrameInfo {
        u32 drawCallsPerFrame = 0;
        u32 numTriangles      = 0;
    };

    class PostProcessSystem;

    class Renderer final : public Volatile {
        ComPtr<IDXGISwapChain> _swapChain;
        ComPtr<ID3D11Device> _device;
        ComPtr<ID3D11DeviceContext> _context;
        ComPtr<ID3D11Texture2D> _backBuffer;
        ComPtr<ID3D11RasterizerState> _rasterizerState;
        ComPtr<ID3D11RenderTargetView> _renderTargetView;
        ComPtr<ID3D11DepthStencilView> _depthStencilView;
        ComPtr<ID3D11DepthStencilState> _depthStencilState;

        ComPtr<ID3D11Texture2D> _sceneTexture;
        ComPtr<ID3D11RenderTargetView> _sceneRTV;
        ComPtr<ID3D11ShaderResourceView> _sceneSRV;
        unique_ptr<PostProcessSystem> _postProcess;

        DeviceInfo _deviceInfo;
        FrameInfo _frameInfo;

    public:
        Renderer() = default;
        ~Renderer() override;

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

        [[nodiscard]] ID3D11RenderTargetView* GetRTV() const {
            return _renderTargetView.Get();
        }

        [[nodiscard]] DeviceInfo GetDeviceInfo() const {
            return _deviceInfo;
        }

        [[nodiscard]] FrameInfo GetFrameInfo() const {
            return _frameInfo;
        }

        bool Initialize(HWND hwnd, int width, int height);
        void BeginScenePass();
        void BeginScenePass(const f32 clearColor[4]);
        void EndScenePass();
        void RenderPostProcess();
        void EndFrame();

        void Draw(u32 vertexCount);
        void DrawIndexed(u32 indexCount);

        void OnResize(u32 width, u32 height) override;

        friend class Mesh;

    private:
        void ResizeSwapchainBuffers(u32 width, u32 height);
        void QueryDeviceInfo();
        void AddTriangleCountToFrame(u32 count);
        bool CreatePostProcessResources(u32 width, u32 height);
    };
}