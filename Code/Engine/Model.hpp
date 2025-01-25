#pragma once

#include "Common/Types.hpp"
#include "InputLayouts.hpp"
#include "Renderer.hpp"

#include "Camera.hpp"
#include "Mesh.hpp"
#include "TransformComponent.hpp"

#include "tiny_gltf.h"
#include "ufbx.h"

namespace x {
    class ModelData;
    class ModelHandle;

    class ModelHandle final : public Resource {
        shared_ptr<ModelData> _modelData;

    public:
        ModelHandle() = default;

        static ModelHandle LoadGLTF(Renderer& renderer, const str& filename);
        static ModelHandle LoadFBX(Renderer& renderer, const str& filename);

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
        bool LoadGLTF(const str& filename);
        bool LoadFBX(const str& filename);

        unique_ptr<Mesh> ProcessGLTFMesh(const tinygltf::Model& model, const tinygltf::Mesh& mesh);
        unique_ptr<Mesh> ProcessFBXMesh(const ufbx_scene* scene, const ufbx_mesh* mesh);

        template<typename T>
        vector<T> GetBufferData(const tinygltf::Model& model, const tinygltf::Accessor& accessor) {
            const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
            const T* data = RCAST<const T*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
            return vector<T>(data, data + accessor.count);
        }
    };
}