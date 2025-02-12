#pragma once

#include "Common/Types.hpp"
#include "D3D.hpp"
#include "RenderContext.hpp"

namespace x {
    class GeometryBuffer {
        ComPtr<ID3D11Buffer> _vertexBuffer;
        ComPtr<ID3D11Buffer> _indexBuffer;
        u32 _stride      = 0;
        u32 _offset      = 0;
        u32 _indexCount  = 0;
        u32 _vertexCount = 0;

    public:
        GeometryBuffer() = default;

        void Create(const RenderContext& renderer,
                    const void* vertexData,
                    size_t vertexStride,
                    size_t vertexCount,
                    const u32* indexData,
                    size_t indexCount) {
            _stride = vertexStride;

            D3D11_BUFFER_DESC vbd{};
            vbd.Usage     = D3D11_USAGE_IMMUTABLE;
            vbd.ByteWidth = vertexCount * vertexStride;
            vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

            D3D11_SUBRESOURCE_DATA vd{};
            vd.pSysMem = vertexData;

            auto* device = renderer.GetDevice();
            auto hr      = device->CreateBuffer(&vbd, &vd, &_vertexBuffer);
            X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create vertex buffer.")

            D3D11_BUFFER_DESC ibd{};
            ibd.Usage     = D3D11_USAGE_IMMUTABLE;
            ibd.ByteWidth = indexCount * sizeof(u32);
            ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

            D3D11_SUBRESOURCE_DATA id{};
            id.pSysMem = indexData;

            hr = device->CreateBuffer(&ibd, &id, &_indexBuffer);
            X_PANIC_ASSERT(SUCCEEDED(hr), "Failed to create index buffer.")

            _indexCount  = indexCount;
            _vertexCount = vertexCount;
        }

        void Bind(const RenderContext& renderer) const {
            auto* context = renderer.GetDeviceContext();
            context->IASetVertexBuffers(0, 1, _vertexBuffer.GetAddressOf(), &_stride, &_offset);
            context->IASetIndexBuffer(_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
            context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        }

        [[nodiscard]] ID3D11Buffer* GetVertexBuffer() {
            return _vertexBuffer.Get();
        }

        [[nodiscard]] ID3D11Buffer* GetIndexBuffer() {
            return _indexBuffer.Get();
        }

        [[nodiscard]] u32 GetIndexCount() const {
            return _indexCount;
        }

        [[nodiscard]] u32 GetVertexCount() const {
            return _vertexCount;
        }
    };
}