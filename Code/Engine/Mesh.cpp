#include "Mesh.hpp"

namespace x {
    Mesh::Mesh(RenderContext& renderer,
               const vector<VSInputPBR>& vertices,
               const vector<u32>& indices): _renderContext(renderer) {
        _geometryBuffers.Create(renderer,
                                vertices.data(),
                                sizeof(VSInputPBR),
                                vertices.size(),
                                indices.data(),
                                indices.size());
    }

    void Mesh::Draw() {
        const auto indexCount = _geometryBuffers.GetIndexCount();
        _geometryBuffers.Bind(_renderContext);
        _renderContext.AddTriangleCountToFrame(indexCount / 3); // for debug purposes
        _renderContext.DrawIndexed(indexCount);
    }
}