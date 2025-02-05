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

    // class PostProcessSystem;

    class RenderContext final {
        ComPtr<IDXGISwapChain> _swapChain;
        ComPtr<ID3D11Device> _device;
        ComPtr<ID3D11DeviceContext> _context;
        ComPtr<ID3D11Texture2D> _backBuffer;

        DeviceInfo _deviceInfo;
        FrameInfo _frameInfo;

    public:
        RenderContext() = default;

        // Prevent moves or copies
        RenderContext(const RenderContext& other)            = delete;
        RenderContext(RenderContext&& other)                 = delete;
        RenderContext& operator=(const RenderContext& other) = delete;
        RenderContext& operator=(RenderContext&& other)      = delete;

        // clang-format off
        // D3D objects
        [[nodiscard]] ID3D11Device* GetDevice() const { return _device.Get(); }
        [[nodiscard]] ID3D11DeviceContext* GetDeviceContext() const { return _context.Get(); }
        [[nodiscard]] ID3D11Texture2D* GetBackBuffer() const { return _backBuffer.Get(); }

        // Debug information / profiler
        [[nodiscard]] DeviceInfo GetDeviceInfo() const { return _deviceInfo; }
        [[nodiscard]] FrameInfo GetFrameInfo() const { return _frameInfo; }
        //
        // clang-format on

        void Initialize(HWND hwnd, int width, int height);
        void Present();

        void Draw(u32 vertexCount);
        void DrawIndexed(u32 indexCount);

        friend class Mesh;

    private:
        friend class RenderSystem;

        void ResizeSwapchainBuffers(u32 width, u32 height);
        void QueryDeviceInfo();
        void AddTriangleCountToFrame(u32 count);
    };
} // namespace x