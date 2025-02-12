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

            newState._lightState = _lightState;
            newState._mainCamera = _mainCamera;

            newState._transforms = _transforms;
            newState._models     = _models;

            return newState;
        }

        void ReleaseAllResources() {
            // TODO: Release component resources
        }

        template<typename T>
            requires IsValidComponent<T>
        const T* GetComponent(EntityId entity) const {
            if constexpr (Same<T, TransformComponent>) {
                return _transforms.GetComponent(entity);
            }

            if constexpr (Same<T, ModelComponent>) {
                return _models.GetComponent(entity);
            }

            if constexpr (Same<T, BehaviorComponent>) {
                return _behaviors.GetComponent(entity);
            }

            return None;
        }

        template<typename T>
            requires IsValidComponent<T>
        T* GetComponentMutable(EntityId entity) {
            if constexpr (Same<T, TransformComponent>) {
                return _transforms.GetComponentMutable(entity);
            }

            if constexpr (Same<T, ModelComponent>) {
                return _models.GetComponentMutable(entity);
            }

            if constexpr (Same<T, BehaviorComponent>) {
                return _behaviors.GetComponentMutable(entity);
            }

            return None;
        }

        template<typename T>
            requires IsValidComponent<T>
        T& AddComponent(EntityId entity) {
            if constexpr (Same<T, TransformComponent>) {
                return _transforms.AddComponent(entity).component;
            }

            if constexpr (Same<T, ModelComponent>) {
                return _models.AddComponent(entity).component;
            }

            if constexpr (Same<T, BehaviorComponent>) {
                return _behaviors.AddComponent(entity).component;
            }
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
            return _lightState;
        }

        [[nodiscard]] LightState const& GetLightState() const {
            return _lightState;
        }

        Camera& GetMainCamera() {
            return _mainCamera;
        }

        [[nodiscard]] Camera const& GetMainCamera() const {
            return _mainCamera;
        }

        void Reset() {
            _nextId     = 0;
            _lightState = {};
            _mainCamera = {};
            _transforms = {};
            _models     = {};
            _behaviors  = {};
        }

    private:
        u64 _nextId = 0;

        // Global state
        LightState _lightState;
        Camera _mainCamera;

        // Component managers
        ComponentManager<TransformComponent> _transforms;
        ComponentManager<ModelComponent> _models;
        ComponentManager<BehaviorComponent> _behaviors;
    };
} // namespace x