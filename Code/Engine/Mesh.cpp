#include "Mesh.hpp"

namespace x {
    Mesh::Mesh(Renderer& renderer, const vector<VSInputPosTexNormal>& vertices, const vector<u32>& indices) : _renderer(
        renderer) {
        _geometryBuffers.Create(renderer, vertices.data(), vertices.size(), indices.data(), indices.size());
    }

    void Mesh::Draw() {
        _geometryBuffers.Bind(_renderer);
        _renderer.GetContext()->DrawIndexed(_geometryBuffers.GetIndexCount(), 0, 0);
    }
}