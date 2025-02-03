#include "Renderer.hpp"
#include "Common/Str.hpp"
#include "PostProcessSystem.hpp"
#include "ColorGradeEffect.hpp"
#include "TonemapEffect.hpp"

namespace x {
    Renderer::~Renderer() = default;

    bool Renderer::Initialize(HWND hwnd, int width, int height) {
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

        UINT numFeatureLevels             = ARRAYSIZE(featureLevels);
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

        if (FAILED(hr)) { return false; }

        // Create render target view
        hr = _swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &_backBuffer);
        if (FAILED(hr)) { return false; }

        hr = _device->CreateRenderTargetView(_backBuffer.Get(), nullptr, &_renderTargetView);
        if (FAILED(hr)) { return false; }

        // Create depth stencil texture
        D3D11_TEXTURE2D_DESC depthStencilDesc = {};
        depthStencilDesc.Width                = width;
        depthStencilDesc.Height               = height;
        depthStencilDesc.MipLevels            = 1;
        depthStencilDesc.ArraySize            = 1;
        depthStencilDesc.Format               = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilDesc.SampleDesc.Count     = 1;
        depthStencilDesc.SampleDesc.Quality   = 0;
        depthStencilDesc.Usage                = D3D11_USAGE_DEFAULT;
        depthStencilDesc.BindFlags            = D3D11_BIND_DEPTH_STENCIL;

        ComPtr<ID3D11Texture2D> depthStencilTexture;
        hr = _device->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencilTexture);
        if (FAILED(hr)) { return false; }

        // Create depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
        depthStencilViewDesc.Format                        = depthStencilDesc.Format;
        depthStencilViewDesc.ViewDimension                 = D3D11_DSV_DIMENSION_TEXTURE2D;

        hr = _device->CreateDepthStencilView(depthStencilTexture.Get(), &depthStencilViewDesc, &_depthStencilView);
        if (FAILED(hr)) { return false; }

        // Create depth stencil state
        D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc = {};
        depthStencilStateDesc.DepthEnable              = TRUE;
        depthStencilStateDesc.DepthWriteMask           = D3D11_DEPTH_WRITE_MASK_ALL;
        depthStencilStateDesc.DepthFunc                = D3D11_COMPARISON_LESS;
        depthStencilStateDesc.StencilEnable            = FALSE;

        hr = _device->CreateDepthStencilState(&depthStencilStateDesc, &_depthStencilState);
        if (FAILED(hr)) { return false; }

        // Set render targets and viewport
        _context->OMSetRenderTargets(1, _renderTargetView.GetAddressOf(), _depthStencilView.Get());
        _context->OMSetDepthStencilState(_depthStencilState.Get(), 0);

        D3D11_VIEWPORT viewport = {};
        viewport.Width          = CAST<f32>(width);
        viewport.Height         = CAST<f32>(height);
        viewport.MinDepth       = 0.0f;
        viewport.MaxDepth       = 1.0f;
        viewport.TopLeftX       = 0.0f;
        viewport.TopLeftY       = 0.0f;

        _context->RSSetViewports(1, &viewport);

        if (!CreatePostProcessResources(width, height)) { return false; }

        QueryDeviceInfo();  // Cache device information

        return true;
    }

    void Renderer::ResizeSwapchainBuffers(u32 width, u32 height) {
        _renderTargetView.Reset();
        _depthStencilView.Reset();
        _backBuffer.Reset();

        PANIC_IF_FAILED(_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0),
                        "Failed to resize swapchain buffers.")
        PANIC_IF_FAILED(_swapChain->GetBuffer(0, IID_PPV_ARGS(&_backBuffer)), "Failed to get swapchain back buffer.")
        PANIC_IF_FAILED(_device->CreateRenderTargetView(_backBuffer.Get(), None, &_renderTargetView),
                        "Failed to create render target view.")

        D3D11_TEXTURE2D_DESC depthStencilDesc = {};
        depthStencilDesc.Width                = width;
        depthStencilDesc.Height               = height;
        depthStencilDesc.MipLevels            = 1;
        depthStencilDesc.ArraySize            = 1;
        depthStencilDesc.Format               = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilDesc.SampleDesc.Count     = 1;
        depthStencilDesc.SampleDesc.Quality   = 0;
        depthStencilDesc.Usage                = D3D11_USAGE_DEFAULT;
        depthStencilDesc.BindFlags            = D3D11_BIND_DEPTH_STENCIL;

        ComPtr<ID3D11Texture2D> depthStencilBuffer;
        PANIC_IF_FAILED(_device->CreateTexture2D(&depthStencilDesc, None, &depthStencilBuffer),
                        "Failed to create depth-stencil texture.")
        PANIC_IF_FAILED(_device->CreateDepthStencilView(depthStencilBuffer.Get(), None, &_depthStencilView),
                        "Failed to ceare depth-stencil view.")

        _context->OMSetRenderTargets(1, _renderTargetView.GetAddressOf(), _depthStencilView.Get());
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

    void Renderer::QueryDeviceInfo() {
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

    void Renderer::BeginScenePass() {
        constexpr float clearColor[4] = {0.01f, 0.01f, 0.01f, 1.0f};
        BeginScenePass(clearColor);
    }

    void Renderer::BeginScenePass(const f32 clearColor[4]) {
        _context->OMSetRenderTargets(1, _sceneRTV.GetAddressOf(), _depthStencilView.Get());
        _context->ClearRenderTargetView(_sceneRTV.Get(), clearColor);
        _context->ClearDepthStencilView(_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        _frameInfo.drawCallsPerFrame = 0;  // reset frame draw call count
        _frameInfo.numTriangles      = 0;
    }

    void Renderer::EndScenePass() {
        ID3D11RenderTargetView* nullRTV = None;
        _context->OMSetRenderTargets(1, &nullRTV, None);
    }

    void Renderer::RenderPostProcess() {
        _context->OMSetRenderTargets(1, _renderTargetView.GetAddressOf(), None);
        _postProcess->Execute(_sceneSRV.Get(), _renderTargetView.Get());
    }

    void Renderer::EndFrame() {
        const auto hr = _swapChain->Present(0, 0);
        if (FAILED(hr)) { _i_ = fprintf(stderr, "Failed to present.\n"); }
    }

    void Renderer::Draw(const u32 vertexCount) {
        _context->Draw(vertexCount, 0);
        _frameInfo.drawCallsPerFrame++;
    }

    void Renderer::DrawIndexed(const u32 indexCount) {
        _context->DrawIndexed(indexCount, 0, 0);
        _frameInfo.drawCallsPerFrame++;
    }

    void Renderer::AddTriangleCountToFrame(u32 count) { _frameInfo.numTriangles += count; }

    bool Renderer::CreatePostProcessResources(u32 width, u32 height) {
        D3D11_TEXTURE2D_DESC sceneDesc {};
        sceneDesc.Width            = width;
        sceneDesc.Height           = height;
        sceneDesc.MipLevels        = 1;
        sceneDesc.ArraySize        = 1;
        sceneDesc.Format           = DXGI_FORMAT_R16G16B16A16_FLOAT;
        sceneDesc.SampleDesc.Count = 1;
        sceneDesc.Usage            = D3D11_USAGE_DEFAULT;
        sceneDesc.BindFlags        = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

        auto hr                    = _device->CreateTexture2D(&sceneDesc, None, &_sceneTexture);
        if (FAILED(hr)) { return false; }

        hr = _device->CreateRenderTargetView(_sceneTexture.Get(), None, &_sceneRTV);
        if (FAILED(hr)) { return false; }

        hr = _device->CreateShaderResourceView(_sceneTexture.Get(), None, &_sceneSRV);
        if (FAILED(hr)) { return false; }

        // Create the post process system if it doesn't already exist
        if (!_postProcess) {
            _postProcess = make_unique<PostProcessSystem>(*this);
            if (!_postProcess->Initialize(width, height)) { return false; }

            const auto tonemap = _postProcess->AddEffect<TonemapEffect>();
            tonemap->SetOperator(TonemapOperator::ACES);
            tonemap->SetExposure(1.0f);

            const auto colorGrade = _postProcess->AddEffect<ColorGradeEffect>();
            colorGrade->SetContrast(1.0f);
            colorGrade->SetSaturation(2.0f);
            colorGrade->SetTemperature(6500.0f);
        }

        return true;
    }

    void Renderer::OnResize(u32 width, u32 height) {
        _sceneTexture.Reset();
        _sceneRTV.Reset();
        _sceneSRV.Reset();

        ResizeSwapchainBuffers(width, height);
        CreatePostProcessResources(width, height);
        _postProcess->OnResize(width, height);

        D3D11_VIEWPORT viewport;
        viewport.Width    = CAST<f32>(width);
        viewport.Height   = CAST<f32>(height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;

        _context->RSSetViewports(1, &viewport);
    }
}  // namespace x