#pragma once

#include "Common/Types.hpp"
#include "D3D.hpp"
#include "EngineCommon.hpp"

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

    class RenderContext final {
        X_CLASS_PREVENT_MOVES_COPIES(RenderContext)

        ComPtr<IDXGISwapChain> _swapChain;
        ComPtr<ID3D11Device> _device;
        ComPtr<ID3D11DeviceContext> _context;
        ComPtr<ID3D11Texture2D> _backBuffer;

        DeviceInfo _deviceInfo;
        FrameInfo _frameInfo;

    public:
        RenderContext() = default;

        // clang-format off
        // D3D objects
        X_NODISCARD ID3D11Device* GetDevice() const { return _device.Get(); }
        X_NODISCARD ID3D11DeviceContext* GetDeviceContext() const { return _context.Get(); }
        X_NODISCARD ID3D11Texture2D* GetBackBuffer() const { return _backBuffer.Get(); }

        // Debug information / profiler
        X_NODISCARD DeviceInfo GetDeviceInfo() const { return _deviceInfo; }
        X_NODISCARD FrameInfo GetFrameInfo() const { return _frameInfo; }
        //
        // clang-format on

        void Initialize(HWND hwnd, int width, int height);
        void Present();

        void Draw(u32 vertexCount);
        void DrawIndexed(u32 indexCount);

    private:
        friend class Mesh;
        friend class RenderSystem;

        void ResizeSwapchainBuffers(u32 width, u32 height);
        void QueryDeviceInfo();
        void AddTriangleCountToFrame(u32 count);
    };
} // namespace x