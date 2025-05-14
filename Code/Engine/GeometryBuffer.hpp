#pragma once

#include "Common/Typedefs.hpp"
#include "D3D.hpp"
#include "RenderContext.hpp"

namespace x {
    class GeometryBuffer {
        ComPtr<ID3D11Buffer> mVertexBuffer;
        ComPtr<ID3D11Buffer> mIndexBuffer;
        u32 mStride      = 0;
        u32 mOffset      = 0;
        u32 mIndexCount  = 0;
        u32 mVertexCount = 0;

    public:
        GeometryBuffer() = default;

        void Create(const RenderContext& renderer,
                    const void* vertexData,
                    size_t vertexStride,
                    size_t vertexCount,
                    const u32* indexData,
                    size_t indexCount) {
            mStride = vertexStride;

            D3D11_BUFFER_DESC vbd {};
            vbd.Usage     = D3D11_USAGE_IMMUTABLE;
            vbd.ByteWidth = vertexCount * vertexStride;
            vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

            D3D11_SUBRESOURCE_DATA vd {};
            vd.pSysMem = vertexData;

            auto* device = renderer.GetDevice();
            auto hr      = device->CreateBuffer(&vbd, &vd, &mVertexBuffer);
            X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create vertex buffer.")

            D3D11_BUFFER_DESC ibd {};
            ibd.Usage     = D3D11_USAGE_IMMUTABLE;
            ibd.ByteWidth = indexCount * sizeof(u32);
            ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

            D3D11_SUBRESOURCE_DATA id {};
            id.pSysMem = indexData;

            hr = device->CreateBuffer(&ibd, &id, &mIndexBuffer);
            X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create index buffer.")

            mIndexCount  = indexCount;
            mVertexCount = vertexCount;
        }

        void Bind(const RenderContext& renderer) const {
            auto* context = renderer.GetDeviceContext();
            context->IASetVertexBuffers(0, 1, mVertexBuffer.GetAddressOf(), &mStride, &mOffset);
            context->IASetIndexBuffer(mIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
            context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        }

        [[nodiscard]] ID3D11Buffer* GetVertexBuffer() {
            return mVertexBuffer.Get();
        }

        [[nodiscard]] ID3D11Buffer* GetIndexBuffer() {
            return mIndexBuffer.Get();
        }

        [[nodiscard]] u32 GetIndexCount() const {
            return mIndexCount;
        }

        [[nodiscard]] u32 GetVertexCount() const {
            return mVertexCount;
        }
    };
}  // namespace x