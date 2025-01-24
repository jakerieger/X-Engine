#include "Model.hpp"

namespace x {
    #pragma region ModelHandle
    ModelHandle ModelHandle::LoadGLTF(Renderer& renderer, const str& filename) {
        ModelHandle handle;
        handle._modelData = make_shared<ModelData>(renderer);
        if (!handle._modelData->LoadGLTF(filename)) { handle._modelData.reset(); }
        return handle;
    }

    void ModelHandle::Draw(const Camera& camera, const TransformComponent& transform) {
        if (_modelData) {
            _modelData->Draw(camera, transform);
        }
    }

    void ModelHandle::Release() {
        _modelData.reset();
    }

    bool ModelHandle::Valid() const {
        return _modelData && _modelData->Valid();
    }
    #pragma endregion

    #pragma region ModelData
    bool ModelData::Valid() const {
        return !_meshes.empty();
    }

    void ModelData::Draw(const Camera& camera, const TransformComponent& transform) {
        for (const auto& mesh : _meshes) {
            mesh->Draw();
        }
    }

    bool ModelData::LoadGLTF(const str& filename) {
        tinygltf::TinyGLTF loader;
        tinygltf::Model model;
        str err, warn;

        bool success = loader.LoadBinaryFromFile(&model, &err, &warn, filename);
        if (!success)
            return false;

        for (const auto& mesh : model.meshes) {
            auto processedMesh = ProcessGLTFMesh(model, mesh);
            if (processedMesh) {
                _meshes.push_back(std::move(processedMesh));
            }
        }

        return !_meshes.empty();
    }

    unique_ptr<Mesh> ModelData::ProcessGLTFMesh(const tinygltf::Model& model, const tinygltf::Mesh& mesh) {
        vector<VSInputPosTexNormal> vertices;
        vector<u32> indices;

        for (const auto& primitive : mesh.primitives) {
            const auto& posAccessor = model.accessors[primitive.attributes.at("POSITION")];
            auto positions          = GetBufferData<Float3>(model, posAccessor);

            vector<Float3> normals;
            if (primitive.attributes.contains("NORMAL")) {
                const auto& normalAccessor = model.accessors[primitive.attributes.at("NORMAL")];
                normals                    = GetBufferData<Float3>(model, normalAccessor);
            }

            vector<Float2> texCoords;
            if (primitive.attributes.contains("TEXCOORD_0")) {
                const auto& uvAccessor = model.accessors[primitive.attributes.at("TEXCOORD_0")];
                texCoords              = GetBufferData<Float2>(model, uvAccessor);
            }

            const auto& indexAccessor = model.accessors[primitive.indices];
            auto primitiveIndices     = GetBufferData<u32>(model, indexAccessor);

            for (size_t i = 0; i < positions.size(); i++) {
                VSInputPosTexNormal vertex{};
                vertex.position = positions[i];
                vertex.normal   = normals.empty() ? Float3{0, 1, 0} : normals[i];
                vertex.texCoord = texCoords.empty() ? Float2{0, 0} : texCoords[i];
                vertices.push_back(vertex);
            }

            indices.insert(indices.end(), primitiveIndices.begin(), primitiveIndices.end());
        }

        return make_unique<Mesh>(_renderer, vertices, indices);
    }
    #pragma endregion
}