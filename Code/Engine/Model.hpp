#pragma once

#include "GeometryBuffer.hpp"
#include "InputLayouts.hpp"

namespace x {
    class Mesh {
        GeometryBuffer mGeometryBuffer;

    public:
        Mesh(const RenderContext& context, const vector<VSInputPBR>& vertices, const vector<u32>& indices) {
            mGeometryBuffer
              .Create(context, vertices.data(), sizeof(VSInputPBR), vertices.size(), indices.data(), indices.size());
        }

        void Draw(RenderContext& context) const {
            mGeometryBuffer.Bind(context);
            context.DrawIndexed(mGeometryBuffer.GetIndexCount());
        }
    };

    class Model {
        friend class ModelLoader;
        vector<Mesh> mMeshes;

    public:
        Model() = default;

        void Draw(RenderContext& context) const {
            for (auto& mesh : mMeshes) {
                mesh.Draw(context);
            }
        }
    };

    using ModelHandle = Model*;
}  // namespace x