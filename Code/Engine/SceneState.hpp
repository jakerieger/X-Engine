// Author: Jake Rieger
// Created: 1/13/2025.
//

#pragma once

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

        EntityId CreateEntity() {
            const auto newId = ++_nextId;
            return EntityId(newId);
        }

        void DestroyEntity(EntityId entity) {
            _transforms.RemoveComponent(entity);
        }

        [[nodiscard]] SceneState Clone() const {
            SceneState newState;

            newState._nextId = _nextId;

            newState.Lights     = Lights;
            newState.MainCamera = MainCamera;

            newState._transforms = _transforms;
            newState._models     = _models;
            newState._behaviors  = _behaviors;

            return newState;
        }

        template<typename T>
            requires IsValidComponent<T>
        const T* GetComponent(EntityId entity) const {
            if constexpr (Same<T, TransformComponent>) { return _transforms.GetComponent(entity); }

            if constexpr (Same<T, ModelComponent>) { return _models.GetComponent(entity); }

            if constexpr (Same<T, BehaviorComponent>) { return _behaviors.GetComponent(entity); }

            return None;
        }

        template<typename T>
            requires IsValidComponent<T>
        T* GetComponentMutable(EntityId entity) {
            if constexpr (Same<T, TransformComponent>) { return _transforms.GetComponentMutable(entity); }

            if constexpr (Same<T, ModelComponent>) { return _models.GetComponentMutable(entity); }

            if constexpr (Same<T, BehaviorComponent>) { return _behaviors.GetComponentMutable(entity); }

            return None;
        }

        template<typename T>
            requires IsValidComponent<T>
        T& AddComponent(EntityId entity) {
            if constexpr (Same<T, TransformComponent>) { return _transforms.AddComponent(entity).component; }

            if constexpr (Same<T, ModelComponent>) { return _models.AddComponent(entity).component; }

            if constexpr (Same<T, BehaviorComponent>) { return _behaviors.AddComponent(entity).component; }
        }

        template<typename T>
            requires IsValidComponent<T>
        const ComponentManager<T>& GetComponents() const {
            if constexpr (Same<T, TransformComponent>) { return _transforms; }
            if constexpr (Same<T, ModelComponent>) { return _models; }
            if constexpr (Same<T, BehaviorComponent>) { return _behaviors; }
        }

        template<typename T>
            requires IsValidComponent<T>
        ComponentManager<T>& GetComponents() {
            if constexpr (Same<T, TransformComponent>) { return _transforms; }
            if constexpr (Same<T, ModelComponent>) { return _models; }
            if constexpr (Same<T, BehaviorComponent>) { return _behaviors; }
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
            _nextId     = 0;
            Lights      = {};
            MainCamera  = {};
            _transforms = {};
            _models     = {};
            _behaviors  = {};
        }

        // Global state
        LightState Lights;
        Camera MainCamera;

    private:
        u64 _nextId = 0;

        // Component managers
        ComponentManager<TransformComponent> _transforms;
        ComponentManager<ModelComponent> _models;
        ComponentManager<BehaviorComponent> _behaviors;
    };
}  // namespace x