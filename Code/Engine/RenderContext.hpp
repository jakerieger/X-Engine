#pragma once

#include "Common/Typedefs.hpp"
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

        ComPtr<IDXGISwapChain> mSwapChain;
        ComPtr<ID3D11Device> mDevice;
        ComPtr<ID3D11DeviceContext> mContext;
        ComPtr<ID3D11Texture2D> mBackBuffer;

        DeviceInfo mDeviceInfo;
        FrameInfo mFrameInfo;

    public:
        RenderContext() = default;

        // clang-format off
        // D3D objects
        X_NODISCARD ID3D11Device* GetDevice() const { return mDevice.Get(); }
        X_NODISCARD ID3D11DeviceContext* GetDeviceContext() const { return mContext.Get(); }
        X_NODISCARD ID3D11Texture2D* GetBackBuffer() const { return mBackBuffer.Get(); }

        // Debug information / profiler
        X_NODISCARD DeviceInfo GetDeviceInfo() const { return mDeviceInfo; }
        X_NODISCARD FrameInfo GetFrameInfo() const { return mFrameInfo; }
        //
        // clang-format on

        void Initialize(HWND hwnd, int width, int height);
        void Present();

        void Draw(u32 vertexCount);
        void DrawIndexed(u32 indexCount);

    private:
        friend class Mesh;
        friend class IWindow;
        friend class Viewport;

        void ResizeSwapchainBuffers(u32 width, u32 height);
        void QueryDeviceInfo();
        void AddTriangleCountToFrame(u32 count);
    };
}  // namespace x