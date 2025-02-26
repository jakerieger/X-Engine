#pragma once

#include "Model.hpp"
#include "ResourceManager.hpp"

#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace x {
    class ModelLoader final : public ResourceLoader<Model> {
        static constexpr u32 kProcessFlags =
          aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_GenNormals | aiProcess_CalcTangentSpace;

        Model LoadImpl(RenderContext& context, const str& path) override {
            Model model;

            Assimp::Importer importer;
            const auto* scene = importer.ReadFile(path.c_str(), kProcessFlags);
            if (!scene) { X_PANIC("Failed to load model: '%s'", path.c_str()); }

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
                    vertex.tangent.x = mesh->mTangents[i].x;
                    vertex.tangent.y = mesh->mTangents[i].y;
                    vertex.tangent.z = mesh->mTangents[i].z;
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