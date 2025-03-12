#pragma once

#include "Model.hpp"
#include "ResourceManager.hpp"

#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "AssetManager.hpp"

namespace x {
    class ModelLoader final : public ResourceLoader<Model> {
        static constexpr u32 kProcessFlags =
          aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_GenNormals | aiProcess_CalcTangentSpace;

        Model LoadImpl(RenderContext& context, const u64 id) override {
            Model model;

            const auto modelBytes = AssetManager::GetAssetData(id);
            if (!modelBytes.has_value()) {
                X_LOG_ERROR("Failed to load model from id {}", id);
                return {};
            }

            Assimp::Importer importer;
            const auto* scene = importer.ReadFileFromMemory(modelBytes->data(), modelBytes->size(), kProcessFlags);
            if (!scene) {
                X_LOG_ERROR("Failed to read model from id {}", id);
                return {};
            }

            ProcessNode(context, scene->mRootNode, scene, model);

            return model;
        }

        void ProcessNode(const RenderContext& context, const aiNode* node, const aiScene* scene, Model& model) {
            for (u32 i = 0; i < node->mNumMeshes; i++) {
                const auto* mesh = scene->mMeshes[node->mMeshes[i]];
                model.mMeshes.push_back(ProcessMesh(context, mesh));
            }

            for (u32 i = 0; i < node->mNumChildren; i++) {
                ProcessNode(context, node->mChildren[i], scene, model);
            }
        }

        Mesh ProcessMesh(const RenderContext& context, const aiMesh* mesh) {
            vector<VSInputPBR> vertices;
            vector<u32> indices;

            for (u32 i = 0; i < mesh->mNumVertices; ++i) {
                VSInputPBR vertex;

                vertex.mPosition.x = mesh->mVertices[i].x;
                vertex.mPosition.y = mesh->mVertices[i].y;
                vertex.mPosition.z = mesh->mVertices[i].z;

                if (mesh->mTextureCoords[0]) {
                    vertex.mTexCoord.x = mesh->mTextureCoords[0][i].x;
                    vertex.mTexCoord.y = mesh->mTextureCoords[0][i].y;
                }

                if (mesh->HasNormals()) {
                    vertex.mNormal.x = mesh->mNormals[i].x;
                    vertex.mNormal.y = mesh->mNormals[i].y;
                    vertex.mNormal.z = mesh->mNormals[i].z;
                }

                if (mesh->HasTangentsAndBitangents()) {
                    vertex.mTangent.x = mesh->mTangents[i].x;
                    vertex.mTangent.y = mesh->mTangents[i].y;
                    vertex.mTangent.z = mesh->mTangents[i].z;
                }

                vertices.push_back(vertex);
            }

            for (u32 i = 0; i < mesh->mNumFaces; i++) {
                const auto& face = mesh->mFaces[i];
                for (u32 j = 0; j < face.mNumIndices; j++) {
                    indices.push_back(face.mIndices[j]);
                }
            }

            return Mesh(context, vertices, indices);
        }
    };

    X_REGISTER_RESOURCE_LOADER(Model, ModelLoader)
}  // namespace x