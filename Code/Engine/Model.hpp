#pragma once

#include "Common/Types.hpp"
#include "InputLayouts.hpp"
#include "Renderer.hpp"

#include <assimp/mesh.h>
#include <assimp/scene.h>

#include "Camera.hpp"
#include "Mesh.hpp"
#include "TransformComponent.hpp"

namespace x {
    class ModelData;
    class ModelHandle;

    class ModelHandle final : public Resource {
        shared_ptr<ModelData> _modelData;

    public:
        ModelHandle() = default;

        static ModelHandle LoadFromFile(Renderer& renderer, const str& filename);
        static ModelHandle LoadFromMemory(Renderer& renderer, const u8* data, size_t size);

        void Draw(const Camera& camera, const TransformComponent& transform);
        void Release() override;
        [[nodiscard]] bool Valid() const;
    };

    class ModelData {
        friend ModelHandle;

    public:
        explicit ModelData(Renderer& renderer) : _renderer(renderer) {}
        [[nodiscard]] bool Valid() const;

    private:
        Renderer& _renderer;
        vector<unique_ptr<Mesh>> _meshes;

        void Draw(const Camera& camera, const TransformComponent& transform);
        bool LoadFromFile(const str& filename);
        bool LoadFromMemory(const u8* data, size_t size);

        void ProcessNode(const aiNode* node, const aiScene* scene);
        unique_ptr<Mesh> ProcessMesh(aiMesh* mesh, const aiScene* scene);
    };
}