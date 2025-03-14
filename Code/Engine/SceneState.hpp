// Author: Jake Rieger
// Created: 1/13/2025.
//

#pragma once

#include <map>

#include "Common/Types.hpp"
#include "EntityId.hpp"
#include "ComponentManager.hpp"
#include "Lights.hpp"
#include "Camera.hpp"
#include "TransformComponent.hpp"
#include "ModelComponent.hpp"
#include "BehaviorComponent.hpp"

namespace x {
    template<typename T>
    concept IsValidComponent = Same<T, TransformComponent> || Same<T, ModelComponent> || Same<T, BehaviorComponent>;

    class SceneState {
        friend class Scene;

    public:
        SceneState() = default;

        EntityId CreateEntity(const str& name) {
            const auto newId  = ++mNextId;
            const auto entity = EntityId {newId};
            mEntities[entity] = name;
            return entity;
        }

        void DestroyEntity(EntityId entity) {
            mTransforms.RemoveComponent(entity);
            mModels.RemoveComponent(entity);
            mBehaviors.RemoveComponent(entity);
            mEntities.erase(entity);
        }

        const std::map<EntityId, str>& GetEntities() const {
            return mEntities;
        }

        void RenameEntity(const EntityId entity, const str& name) {
            mEntities[entity] = name;
        }

        [[nodiscard]] SceneState Clone() const {
            SceneState newState;

            newState.mNextId   = mNextId;
            newState.mEntities = mEntities;

            newState.Lights     = Lights;
            newState.MainCamera = MainCamera;

            newState.mTransforms = mTransforms;
            newState.mModels     = mModels;
            newState.mBehaviors  = mBehaviors;

            return newState;
        }

        template<typename T>
            requires IsValidComponent<T>
        const T* GetComponent(EntityId entity) const {
            if constexpr (Same<T, TransformComponent>) { return mTransforms.GetComponent(entity); }

            if constexpr (Same<T, ModelComponent>) { return mModels.GetComponent(entity); }

            if constexpr (Same<T, BehaviorComponent>) { return mBehaviors.GetComponent(entity); }

            return nullptr;
        }

        template<typename T>
            requires IsValidComponent<T>
        T* GetComponentMutable(EntityId entity) {
            if constexpr (Same<T, TransformComponent>) { return mTransforms.GetComponentMutable(entity); }

            if constexpr (Same<T, ModelComponent>) { return mModels.GetComponentMutable(entity); }

            if constexpr (Same<T, BehaviorComponent>) { return mBehaviors.GetComponentMutable(entity); }

            return nullptr;
        }

        template<typename T>
            requires IsValidComponent<T>
        T& AddComponent(EntityId entity) {
            if constexpr (Same<T, TransformComponent>) { return mTransforms.AddComponent(entity).component; }

            if constexpr (Same<T, ModelComponent>) { return mModels.AddComponent(entity).component; }

            if constexpr (Same<T, BehaviorComponent>) { return mBehaviors.AddComponent(entity).component; }
        }

        template<typename T>
            requires IsValidComponent<T>
        const ComponentManager<T>& GetComponents() const {
            if constexpr (Same<T, TransformComponent>) { return mTransforms; }
            if constexpr (Same<T, ModelComponent>) { return mModels; }
            if constexpr (Same<T, BehaviorComponent>) { return mBehaviors; }
        }

        template<typename T>
            requires IsValidComponent<T>
        ComponentManager<T>& GetComponents() {
            if constexpr (Same<T, TransformComponent>) { return mTransforms; }
            if constexpr (Same<T, ModelComponent>) { return mModels; }
            if constexpr (Same<T, BehaviorComponent>) { return mBehaviors; }
        }

        LightState& GetLightState() {
            return Lights;
        }

        [[nodiscard]] LightState const& GetLightState() const {
            return Lights;
        }

        Camera& GetMainCamera() {
            return MainCamera;
        }

        [[nodiscard]] Camera const& GetMainCamera() const {
            return MainCamera;
        }

        void Reset() {
            mEntities.clear();
            mNextId     = 0;
            Lights      = {};
            MainCamera  = {};
            mTransforms = {};
            mModels     = {};
            mBehaviors  = {};
        }

        // Global state
        LightState Lights;
        Camera MainCamera;

    private:
        u64 mNextId = 0;
        std::map<EntityId, std::string> mEntities;

        // Component managers
        ComponentManager<TransformComponent> mTransforms;
        ComponentManager<ModelComponent> mModels;
        ComponentManager<BehaviorComponent> mBehaviors;
    };
}  // namespace x