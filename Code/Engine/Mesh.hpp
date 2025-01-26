#pragma once

#include "Common/Types.hpp"
#include "GeometryBuffer.hpp"
#include "InputLayouts.hpp"

namespace x {
    class Mesh {
        Renderer& _renderer;
        GeometryBuffer<VSInputPosTexNormal> _geometryBuffers{};

    public:
        Mesh(Renderer& renderer, const vector<VSInputPosTexNormal>& vertices, const vector<u32>& indices)
            : _renderer(renderer) {
            _geometryBuffers.Create(renderer, vertices.data(), vertices.size(), indices.data(), indices.size());
        }

        void Draw() {
            _geometryBuffers.Bind(_renderer);
            _renderer.GetContext()->DrawIndexed(_geometryBuffers.GetIndexCount(), 0, 0);
        }
    };
}