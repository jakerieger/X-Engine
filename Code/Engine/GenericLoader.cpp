#include "GenericLoader.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace x {
    static constexpr u32 kProcessFlags = aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_GenNormals;

    shared_ptr<ModelData> GenericLoader::LoadFromFile(const str& path) {
        Assimp::Importer importer;

        const auto* scene = importer.ReadFile(path.c_str(), kProcessFlags);

        if (!scene) {
            _i_ = fprintf(stderr, "GenericLoader error reading file: %s\n", path.c_str());
            return None;
        }

        auto modelData = make_shared<ModelData>(_renderer);
        ProcessNode(scene->mRootNode, scene, modelData);
        return modelData;
    }

    shared_ptr<ModelData> GenericLoader::LoadFromMemory(const u8* data, size_t size) {
        Assimp::Importer importer;

        const auto* scene = importer.ReadFileFromMemory(data, size, kProcessFlags);

        if (!scene) {
            _i_ = fprintf(stderr, "GenericLoader error reading data from memory.\n");
            return None;
        }

        auto modelData = make_shared<ModelData>(_renderer);
        ProcessNode(scene->mRootNode, scene, modelData);
        return modelData;
    }

    void GenericLoader::ProcessNode(aiNode* node, const aiScene* scene, const shared_ptr<ModelData>& outData) {
        for (auto i = 0; i < node->mNumMeshes; i++) {
            auto* mesh = scene->mMeshes[node->mMeshes[i]];
            outData->_meshes.push_back(ProcessMesh(mesh, scene));
        }

        for (auto i = 0; i < node->mNumChildren; i++) {
            ProcessNode(node->mChildren[i], scene, outData);
        }
    }

    unique_ptr<Mesh> GenericLoader::ProcessMesh(aiMesh* mesh, const aiScene* scene) {
        vector<VSInputPosTexNormal> vertices;
        vector<u32> indices;

        for (auto i = 0; i < mesh->mNumVertices; ++i) {
            VSInputPosTexNormal vertex;

            vertex.position.x = mesh->mVertices[i].x;
            vertex.position.y = mesh->mVertices[i].y;
            vertex.position.z = mesh->mVertices[i].z;

            if (mesh->mTextureCoords[0]) {
                vertex.texCoord.x = mesh->mTextureCoords[0][i].x;
                vertex.texCoord.y = mesh->mTextureCoords[0][i].y;
            }

            if (mesh->HasNormals()) {
                vertex.normal.x = mesh->mNormals[i].x;
                vertex.normal.y = mesh->mNormals[i].y;
                vertex.normal.z = mesh->mNormals[i].z;
            }

            if (mesh->HasTangentsAndBitangents()) {
                // TODO: Load tangents and bitangents
            }

            vertices.push_back(vertex);
        }

        for (auto i = 0; i < mesh->mNumFaces; i++) {
            const auto face = mesh->mFaces[i];
            for (auto j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }

        return make_unique<Mesh>(_renderer, vertices, indices);
    }
}