#pragma once

#include "Common/Types.hpp"
#include "D3D.hpp"
#include "Renderer.hpp"

namespace x {
    // TODO: This doesn't really need to be a template class because every other class that uses it would need to be as well.
    // Maybe some other way caclulating stride would suffice

    template<typename T>
    class GeometryBuffer {
        ComPtr<ID3D11Buffer> _vertexBuffer;
        ComPtr<ID3D11Buffer> _indexBuffer;
        u32 _stride     = sizeof(T);
        u32 _offset     = 0;
        u32 _indexCount = 0;

    public:
        GeometryBuffer() = default;
        void Create(const Renderer& renderer,
                    const T* vertexData,
                    size_t vertexDataSize,
                    const u32* indexData,
                    size_t indexDataSize);
        void Bind(const Renderer& renderer);

        [[nodiscard]] ID3D11Buffer* GetVertexBuffer() {
            return _vertexBuffer.Get();
        }

        [[nodiscard]] ID3D11Buffer* GetIndexBuffer() {
            return _indexBuffer.Get();
        }

        [[nodiscard]] u32 GetIndexCount() const {
            return _indexCount;
        }
    };

    template<typename T>
    void GeometryBuffer<T>::Create(const Renderer& renderer,
                                   const T* vertexData,
                                   const size_t vertexDataSize,
                                   const u32* indexData,
                                   const size_t indexDataSize) {
        D3D11_BUFFER_DESC vbd{};
        vbd.Usage     = D3D11_USAGE_IMMUTABLE;
        vbd.ByteWidth = vertexDataSize * sizeof(T);
        vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA vd{};
        vd.pSysMem = vertexData;

        auto* device = renderer.GetDevice();
        PANIC_IF_FAILED(device->CreateBuffer(&vbd, &vd, &_vertexBuffer), "Failed to create vertex buffer.")

        D3D11_BUFFER_DESC ibd{};
        ibd.Usage     = D3D11_USAGE_IMMUTABLE;
        ibd.ByteWidth = indexDataSize * sizeof(u32);
        ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA id{};
        id.pSysMem = indexData;

        PANIC_IF_FAILED(device->CreateBuffer(&ibd, &id, &_indexBuffer), "Failed to create index buffer.")

        _indexCount = indexDataSize;
    }

    template<typename T>
    void GeometryBuffer<T>::Bind(const Renderer& renderer) {
        auto* context = renderer.GetContext();
        context->IASetVertexBuffers(0, 1, _vertexBuffer.GetAddressOf(), &_stride, &_offset);
        context->IASetIndexBuffer(_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }
}