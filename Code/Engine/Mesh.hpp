#pragma once

#include "Common/Types.hpp"
#include "GeometryBuffer.hpp"
#include "InputLayouts.hpp"

namespace x {
    class Mesh {
        Renderer& _renderer;
        GeometryBuffer _geometryBuffers{};

    public:
        Mesh(Renderer& renderer, const vector<VSInputPBR>& vertices, const vector<u32>& indices)
            : _renderer(renderer) {
            _geometryBuffers.Create(renderer,
                                    vertices.data(),
                                    sizeof(VSInputPBR),
                                    vertices.size(),
                                    indices.data(),
                                    indices.size());
        }

        void Draw() {
            const auto indexCount = _geometryBuffers.GetIndexCount();
            _geometryBuffers.Bind(_renderer);
            _renderer.AddTriangleCountToFrame(indexCount / 3); // for debug purposes
            _renderer.DrawIndexed(indexCount);
        }
    };
}