#pragma once

#include "Common/Types.hpp"
#include "SceneState.hpp"
#include "TextureLoader.hpp"
#include "ModelLoader.hpp"
#include "ScriptTypeRegistry.hpp"
#include "MaterialParser.hpp"

namespace x {
    class Scene {
        ResourceManager mResources;
        SceneState mState;
        SceneState mInitialState;
        RenderContext& mContext;
        ScriptEngine& mScriptEngine;
        str mName;
        str mDescription;
        unordered_map<str, EntityId> mEntities;

    public:
        explicit Scene(RenderContext& context, ScriptEngine& scriptEngine);
        ~Scene();

        void Load(const str& path);
        void Unload();

        void Reset();
        void ResetToInitialState();

        void Awake();
        void Update(f32 deltaTime);
        void Destroyed();

        SceneState& GetState();
        unordered_map<str, EntityId>& GetEntities();
        X_NODISCARD const SceneState& GetState() const;
        X_NODISCARD u32 GetNumEntities() const;
        X_NODISCARD ResourceManager& GetResourceManager();

        void RegisterVolatiles(vector<Volatile*>& volatiles);

    private:
        void LoadMaterial(const MaterialDescriptor& material, ModelComponent& modelComponent);
    };
}  // namespace x