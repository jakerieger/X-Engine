// Author: Jake Rieger
// Created: 1/13/2025.
//

#pragma once

#include "Common/Types.hpp"
#include "EntityId.hpp"
#include "ComponentManager.hpp"
#include "TransformComponent.hpp"
#include "Lights.hpp"
#include "Camera.hpp"

namespace x {
    template<typename T>
    concept IsValidComponent = std::is_same_v<T, TransformComponent>;

    class GameState {
    public:
        GameState() = default;

        EntityId CreateEntity() {
            const auto newId = ++_nextId;
            return EntityId(newId);
        }

        void DestroyEntity(EntityId entity) {
            _transforms.RemoveComponent(entity);
        }

        [[nodiscard]] GameState Clone() const {
            GameState newState;
            newState._nextId     = _nextId;
            newState._transforms = _transforms;
            newState._lightState = _lightState;
            return newState;
        }

        void ReleaseAllResources() {
            // TODO: Release component resources
        }

        template<typename T>
            requires IsValidComponent<T>
        const T* GetComponent(EntityId entity) const {
            if constexpr (std::is_same_v<T, TransformComponent>) {
                return _transforms.GetComponent(entity);
            }

            return None;
        }

        template<typename T>
            requires IsValidComponent<T>
        T* GetComponentMutable(EntityId entity) {
            if constexpr (std::is_same_v<T, TransformComponent>) {
                return _transforms.GetComponentMutable(entity);
            }

            return None;
        }

        template<typename T>
            requires IsValidComponent<T>
        T& AddComponent(EntityId entity) {
            if constexpr (std::is_same_v<T, TransformComponent>) {
                return _transforms.AddComponent(entity).component;
            }
        }

        template<typename T>
            requires IsValidComponent<T>
        const ComponentManager<T>& GetComponents() const {
            if constexpr (std::is_same_v<T, TransformComponent>) { return _transforms; }
        }

        template<typename T>
            requires IsValidComponent<T>
        ComponentManager<T>& GetComponents() {
            if constexpr (std::is_same_v<T, TransformComponent>) { return _transforms; }
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

    private:
        u64 _nextId = 0;

        // Global state
        LightState _lightState;
        Camera _mainCamera;

        // Component managers
        ComponentManager<TransformComponent> _transforms;

        template<typename T>
        void ReleaseComponentResources() {
            if constexpr (detail::release_resources<T>::value) {
                GetComponents<T>().ReleaseResources();
            }
        }
    };
} // namespace x