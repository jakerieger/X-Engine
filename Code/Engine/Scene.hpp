#pragma once

#include "Common/Types.hpp"
#include "SceneState.hpp"
#include "TextureLoader.hpp"
#include "ModelLoader.hpp"
#include "ScriptTypeRegistry.hpp"
#include "MaterialParser.hpp"

namespace x {
    class Scene {
        ResourceManager _resources;
        SceneState _state;
        RenderContext& _context;
        ScriptEngine& _scriptEngine;
        str _name;
        str _description;
        unordered_map<str, EntityId> _entities;

    public:
        explicit Scene(RenderContext& context, ScriptEngine& scriptEngine);
        ~Scene();

        void Load(const str& path);
        void Unload();

        void Awake();
        void Update(f32 deltaTime);
        void Destroyed();

        SceneState& GetState();
        X_NODISCARD const SceneState& GetState() const;

        void RegisterVolatiles(vector<Volatile*>& volatiles);

    private:
        void LoadMaterial(const MaterialDescriptor& material, ModelComponent& modelComponent);
    };
}  // namespace x