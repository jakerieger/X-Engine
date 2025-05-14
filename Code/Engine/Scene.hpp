#pragma once

#include "Common/Typedefs.hpp"
#include "SceneState.hpp"
#include "TextureLoader.hpp"
#include "ModelLoader.hpp"
#include "ScriptTypeRegistry.hpp"
#include "MaterialParser.hpp"
#include "SceneParser.hpp"

namespace x {
    class Scene {
    public:
        explicit Scene(RenderContext& context, ScriptEngine& scriptEngine);
        ~Scene();

        void Load(const SceneDescriptor& descriptor);
        void Unload();

        void Reset();
        void ResetState();

        void Awake();
        void Update(f32 deltaTime);
        void Destroyed();

        void DrawOpaque();
        void DrawTransparent();

        X_NODISCARD SceneState& GetState();
        X_NODISCARD const SceneState& GetState() const;
        X_NODISCARD ResourceManager& GetResourceManager();
        X_NODISCARD const str& GetName() const;
        X_NODISCARD bool Loaded() const;

        shared_ptr<IMaterial> LoadMaterial(const MaterialDescriptor& material);

    private:
        ResourceManager mResources;
        SceneState mState;
        SceneState mInitialState;
        RenderContext& mContext;
        ScriptEngine& mScriptEngine;
        str mName;
        str mDescription;
        bool mLoaded {false};

        using ModelTransformPair = std::pair<const ModelComponent*, const TransformComponent*>;
        vector<ModelTransformPair> mOpaqueObjects;
        vector<ModelTransformPair> mTransparentObjects;

        void LoadMaterial(const MaterialDescriptor& material, ModelComponent& modelComponent);
    };
}  // namespace x