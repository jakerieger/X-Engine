#pragma once

#include "Model.hpp"
#include "RenderContext.hpp"
#include "Common/Types.hpp"

#include <assimp/mesh.h>
#include <assimp/scene.h>

namespace x {
    /// @brief Generic mesh data loader. Not guaranteed to produce valid data for all 3D formats.
    /// @note Until `FBXLoader` is fixed, prefer this over the former.
    class GenericLoader {
        RenderContext& _renderer;

    public:
        explicit GenericLoader(RenderContext& renderer) : _renderer(renderer) {}

        shared_ptr<ModelData> LoadFromFile(const str& path);
        shared_ptr<ModelData> LoadFromMemory(const u8* data, size_t size);

    private:
        void ProcessNode(aiNode* node, const aiScene* scene, const shared_ptr<ModelData>& outData);
        unique_ptr<Mesh> ProcessMesh(aiMesh* mesh, const aiScene* scene);
    };
}