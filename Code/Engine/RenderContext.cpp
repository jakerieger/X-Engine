#include "RenderContext.hpp"
#include "Common/Str.hpp"

namespace x {
    void RenderContext::Initialize(HWND hwnd, int width, int height) {
        // Create device and swap chain
        DXGI_SWAP_CHAIN_DESC swapChainDesc               = {};
        swapChainDesc.BufferCount                        = 1;
        swapChainDesc.BufferDesc.Width                   = width;
        swapChainDesc.BufferDesc.Height                  = height;
        swapChainDesc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferDesc.RefreshRate.Numerator   = 60;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
        swapChainDesc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.OutputWindow                       = hwnd;
        swapChainDesc.SampleDesc.Count                   = 1;
        swapChainDesc.SampleDesc.Quality                 = 0;
        swapChainDesc.Windowed                           = TRUE;

        D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0};

        UINT numFeatureLevels = ARRAYSIZE(featureLevels);
        D3D_FEATURE_LEVEL featureLevel;

        UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
        #ifdef _DEBUG
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
        #endif

        // Create device and swap chain
        HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr,
                                                   // Default adapter
                                                   D3D_DRIVER_TYPE_HARDWARE,
                                                   // Hardware acceleration
                                                   nullptr,
                                                   // No software device
                                                   createDeviceFlags,
                                                   // Debug mode if needed
                                                   featureLevels,
                                                   numFeatureLevels,
                                                   D3D11_SDK_VERSION,
                                                   &swapChainDesc,
                                                   &_swapChain,
                                                   &_device,
                                                   &featureLevel,
                                                   &_context);

        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create device and swapchain.")

        // Create render target view
        hr = _swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &_backBuffer);
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to get swapchain back buffer.")

        QueryDeviceInfo(); // Cache device information
    }

    void RenderContext::ResizeSwapchainBuffers(u32 width, u32 height) {
        _backBuffer.Reset();

        auto hr = _swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
        X_PANIC_ASSERT(SUCCEEDED(hr),
                       "Failed to resize swapchain buffers.")

        hr = _swapChain->GetBuffer(0, IID_PPV_ARGS(&_backBuffer));
        X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to get swapchain back buffer.")
    }

    static str GetVendorNameFromId(const u32 id) {
        switch (id) {
            case 0x10DE:
                return "NVIDIA";
            case 0x1002:
                return "AMD";
            case 0x8086:
                return "Intel";
            default:
                return "Unknown";
        }
    }

    void RenderContext::QueryDeviceInfo() {
        IDXGIFactory* factory = None;
        if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory))) {
            throw std::runtime_error("Failed to create DXGI factory.");
        }

        IDXGIAdapter* adapter = None;
        if (FAILED(factory->EnumAdapters(0, &adapter))) {
            factory->Release();
            throw std::runtime_error("Failed to get GPU adapter");
        }

        DXGI_ADAPTER_DESC desc;
        if (FAILED(adapter->GetDesc(&desc))) {
            adapter->Release();
            factory->Release();
            throw std::runtime_error("Failed to get GPU description.");
        }

        _deviceInfo.vendor              = GetVendorNameFromId(desc.VendorId);
        _deviceInfo.model               = WideToAnsi(desc.Description);
        _deviceInfo.videoMemoryInBytes  = desc.DedicatedVideoMemory;
        _deviceInfo.sharedMemoryInBytes = desc.SharedSystemMemory;

        adapter->Release();
        factory->Release();
    }

    void RenderContext::Present() {
        const auto hr = _swapChain->Present(1, 0);
        if (FAILED(hr)) { _i_ = fprintf(stderr, "Failed to present.\n"); }
    }

    void RenderContext::Draw(const u32 vertexCount) {
        _context->Draw(vertexCount, 0);
    }

    void RenderContext::DrawIndexed(const u32 indexCount) {
        _context->DrawIndexed(indexCount, 0, 0);
    }

    void RenderContext::AddTriangleCountToFrame(u32 count) {}
} // namespace x