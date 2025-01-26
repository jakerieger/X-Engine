#pragma once

#include "Model.hpp"
#include "Renderer.hpp"
#include "Common/Types.hpp"

#include <assimp/mesh.h>
#include <assimp/scene.h>

namespace x {
    /// @brief Generic mesh data loader. Not guaranteed to produce valid data for all 3D formats.
    class GenericLoader {
        Renderer& _renderer;

    public:
        explicit GenericLoader(Renderer& renderer) : _renderer(renderer) {}

        shared_ptr<ModelData> LoadFromFile(const str& path);
        shared_ptr<ModelData> LoadFromMemory(const u8* data, size_t size);

    private:
        void ProcessNode(aiNode* node, const aiScene* scene, const shared_ptr<ModelData>& outData);
        unique_ptr<Mesh> ProcessMesh(aiMesh* mesh, const aiScene* scene);
    };
}