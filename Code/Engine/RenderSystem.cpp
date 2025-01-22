#include "RenderSystem.hpp"

#pragma region Shaders
#pragma endregion

namespace x {
    RenderSystem::RenderSystem(Renderer& renderer) : _renderer(renderer) {
        Initialize();
    }

    void RenderSystem::Initialize() {
        auto* backBuffer = _renderer.GetBackBuffer();
        D3D11_TEXTURE2D_DESC desc;
        backBuffer->GetDesc(&desc);
    }

    void RenderSystem::Render() {
        auto* context = _renderer.GetContext();

        // context->VSSetConstantBuffers(0, 1, _sceneConstantBuffer.GetAddressOf());
        // context->PSSetConstantBuffers(0, 1, _sceneConstantBuffer.GetAddressOf());
        //
        // context->OMSetDepthStencilState(_opaqueDepthState.Get(), 0);
        // context->OMSetBlendState(_opaqueBlendState.Get(), None, 0xFFFFFFFF);
        //
        // for (const auto& item : _opaqueQueue) {
        //     // Setup per-object constants and render
        //     // TODO: Implement material and mesh rendering
        // }
        //
        // context->OMSetDepthStencilState(_transparentDepthState.Get(), 0);
        // context->OMSetBlendState(_transparentBlendState.Get(), None, 0xFFFFFFFF);
        //
        // for (const auto& item : _transparentQueue) {
        //     // Set per-object constants and render
        //     // TODO: Implement material and mesh rendering
        // }
        //
        // _opaqueQueue.clear();
        // _transparentQueue.clear();
    }

    void RenderSystem::OnResize(u32 width, u32 height) {}
}