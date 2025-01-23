#include "Model.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "TransformMatrices.hpp"

namespace x {
    #pragma region ModelHandle
    ModelHandle ModelHandle::LoadFromFile(Renderer& renderer, const str& filename) {
        ModelHandle handle;
        handle._modelData = make_shared<ModelData>(renderer);
        if (!handle._modelData->LoadFromFile(filename)) { handle._modelData.reset(); }
        return handle;
    }

    ModelHandle ModelHandle::LoadFromMemory(Renderer& renderer, const u8* data, const size_t size) {
        ModelHandle handle;
        handle._modelData = make_shared<ModelData>(renderer);
        if (!handle._modelData->LoadFromMemory(data, size)) { handle._modelData.reset(); }
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
        // prep transform matrices
        // bind material
        // update cbuffers, maybe move this to an update callback instead

        TransformMatrices transformMatrices(transform.GetTransformMatrix(),
                                            camera.GetViewMatrix(),
                                            camera.GetProjectionMatrix());

        for (const auto& mesh : _meshes) {
            mesh->Draw();
        }
    }

    bool ModelData::LoadFromFile(const str& filename) {
        Assimp::Importer importer;
        const auto* scene = importer.ReadFile(filename.c_str(),
                                              aiProcess_Triangulate | aiProcess_GenNormals |
                                              aiProcess_ConvertToLeftHanded | aiProcess_CalcTangentSpace);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            return false;
        }
        ProcessNode(scene->mRootNode, scene);
        return true;
    }

    bool ModelData::LoadFromMemory(const u8* data, const size_t size) {
        Assimp::Importer importer;
        const auto* scene = importer.ReadFileFromMemory(data,
                                                        size,
                                                        aiProcess_Triangulate | aiProcess_GenNormals |
                                                        aiProcess_ConvertToLeftHanded | aiProcess_CalcTangentSpace);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) { return false; }
        ProcessNode(scene->mRootNode, scene);
        return true;
    }

    void ModelData::ProcessNode(const aiNode* node, const aiScene* scene) {
        for (u32 i = 0; i < node->mNumMeshes; i++) {
            auto* mesh = scene->mMeshes[node->mMeshes[i]];
            _meshes.push_back(ProcessMesh(mesh, scene));
        }

        for (u32 i = 0; i < node->mNumChildren; i++) {
            ProcessNode(node->mChildren[i], scene);
        }
    }

    unique_ptr<Mesh> ModelData::ProcessMesh(aiMesh* mesh, const aiScene* scene) {
        vector<VSInputPosTexNormal> vertices;
        vector<u32> indices;

        for (u32 i = 0; i < mesh->mNumVertices; i++) {
            VSInputPosTexNormal vertex = {};

            vertex.position.x = mesh->mVertices[i].x;
            vertex.position.y = mesh->mVertices[i].y;
            vertex.position.z = mesh->mVertices[i].z;

            vertex.normal.x = mesh->mNormals[i].x;
            vertex.normal.y = mesh->mNormals[i].y;
            vertex.normal.z = mesh->mNormals[i].z;

            // vertex.tangent.x = mesh->mTangents[i].x;
            // vertex.tangent.y = mesh->mTangents[i].y;
            // vertex.tangent.z = mesh->mTangents[i].z;
            //
            // vertex.biTangent.x = mesh->mBitangents[i].x;
            // vertex.biTangent.y = mesh->mBitangents[i].y;
            // vertex.biTangent.z = mesh->mBitangents[i].z;

            if (mesh->mTextureCoords[0]) {
                vertex.texCoord.x = mesh->mTextureCoords[0][i].x;
                vertex.texCoord.y = mesh->mTextureCoords[0][i].y;
            } else {
                vertex.texCoord.x = 0;
                vertex.texCoord.y = 0;
            }

            vertices.push_back(vertex);
        }

        for (u32 i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (u32 j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }

        return make_unique<Mesh>(
            _renderer,
            vertices,
            indices);
    }
    #pragma endregion
}