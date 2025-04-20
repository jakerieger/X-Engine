// Author: Jake Rieger
// Created: 1/13/2025.
//

// ReSharper disable CppNotAllPathsReturnValue
#pragma once

#include <map>
#include <ranges>

#include "Common/Types.hpp"
#include "EntityId.hpp"
#include "ComponentManager.hpp"
#include "Lights.hpp"
#include "Camera.hpp"
#include "TransformComponent.hpp"
#include "ModelComponent.hpp"
#include "BehaviorComponent.hpp"
#include "CameraComponent.hpp"

namespace x {
    template<typename T>
    concept IsValidComponent =
      Same<T, TransformComponent> || Same<T, ModelComponent> || Same<T, BehaviorComponent> || Same<T, CameraComponent>;

    class SceneState {
        friend class Scene;

    public:
        SceneState() = default;

        EntityId CreateEntity(const str& name) {
            // if name already exists in entity map
            for (const auto& [id, entityName] : mEntities) {
                if (entityName == name) { return id; }
            }

            const auto newId  = ++mNextId;
            const auto entity = EntityId {newId};
            mEntities[entity] = name;
            return entity;
        }

        void DestroyEntity(EntityId entity) {
            mTransforms.RemoveComponent(entity);
            mModels.RemoveComponent(entity);
            mBehaviors.RemoveComponent(entity);
            mCameras.RemoveComponent(entity);
            mEntities.erase(entity);

            // TODO: Find next available camera component to make main camera
        }

        EntityId GetFirstEntity() const {
            if (mEntities.empty()) { return EntityId::Invalid(); }
            return mEntities.begin()->first;
        }

        const std::map<EntityId, str>& GetEntities() const {
            return mEntities;
        }

        void RenameEntity(const EntityId entity, const str& name) {
            mEntities[entity] = name;
        }

        SceneState(const SceneState& other) {
            mTransforms = other.mTransforms;
            mModels     = other.mModels;
            mBehaviors  = other.mBehaviors;
            mEntities   = other.mEntities;
            mNextId     = other.mNextId;
            mLights     = other.mLights;
        }

        SceneState& operator=(const SceneState& other) {
            mTransforms = other.mTransforms;
            mModels     = other.mModels;
            mBehaviors  = other.mBehaviors;
            mEntities   = other.mEntities;
            mNextId     = other.mNextId;
            mLights     = other.mLights;
            return *this;
        }

        SceneState(SceneState&& other) noexcept
            : mNextId(other.mNextId), mEntities(std::move(other.mEntities)), mLights(std::move(other.mLights)),
              mTransforms(std::move(other.mTransforms)), mModels(std::move(other.mModels)),
              mBehaviors(std::move(other.mBehaviors)), mCameras(std::move(other.mCameras)) {}

        SceneState& operator=(SceneState&& other) noexcept {
            if (this != &other) {
                mNextId     = other.mNextId;
                mEntities   = std::move(other.mEntities);
                mLights     = std::move(other.mLights);
                mTransforms = std::move(other.mTransforms);
                mModels     = std::move(other.mModels);
                mBehaviors  = std::move(other.mBehaviors);
                mCameras    = std::move(other.mCameras);
            }
            return *this;
        };

        template<typename T>
            requires IsValidComponent<T>
        const T* GetComponent(EntityId entity) const {
            if constexpr (Same<T, TransformComponent>) { return mTransforms.GetComponent(entity); }
            if constexpr (Same<T, ModelComponent>) { return mModels.GetComponent(entity); }
            if constexpr (Same<T, BehaviorComponent>) { return mBehaviors.GetComponent(entity); }
            if constexpr (Same<T, CameraComponent>) { return mCameras.GetComponent(entity); }

            return nullptr;
        }

        template<typename T>
            requires IsValidComponent<T>
        T* GetComponentMutable(EntityId entity) {
            if constexpr (Same<T, TransformComponent>) { return mTransforms.GetComponentMutable(entity); }
            if constexpr (Same<T, ModelComponent>) { return mModels.GetComponentMutable(entity); }
            if constexpr (Same<T, BehaviorComponent>) { return mBehaviors.GetComponentMutable(entity); }
            if constexpr (Same<T, CameraComponent>) { return mCameras.GetComponentMutable(entity); }

            return nullptr;
        }

        template<typename T>
            requires IsValidComponent<T>
        T& AddComponent(EntityId entity) {
            if constexpr (Same<T, TransformComponent>) { return mTransforms.AddComponent(entity).component; }
            if constexpr (Same<T, ModelComponent>) { return mModels.AddComponent(entity).component; }
            if constexpr (Same<T, BehaviorComponent>) { return mBehaviors.AddComponent(entity).component; }
            if constexpr (Same<T, CameraComponent>) { return mCameras.AddComponent(entity).component; }
        }

        template<typename T>
            requires IsValidComponent<T>
        const ComponentManager<T>& GetComponents() const {
            if constexpr (Same<T, TransformComponent>) { return mTransforms; }
            if constexpr (Same<T, ModelComponent>) { return mModels; }
            if constexpr (Same<T, BehaviorComponent>) { return mBehaviors; }
            if constexpr (Same<T, CameraComponent>) { return mCameras; }
        }

        template<typename T>
            requires IsValidComponent<T>
        ComponentManager<T>& GetComponents() {
            if constexpr (Same<T, TransformComponent>) { return mTransforms; }
            if constexpr (Same<T, ModelComponent>) { return mModels; }
            if constexpr (Same<T, BehaviorComponent>) { return mBehaviors; }
            if constexpr (Same<T, CameraComponent>) { return mCameras; }
        }

        template<typename T>
            requires IsValidComponent<T>
        bool HasComponent(EntityId entity) const {
            if (GetComponent<T>(entity)) { return true; }
            return false;
        }

        X_NODISCARD LightState& GetLightState() {
            return mLights;
        }

        X_NODISCARD LightState const& GetLightState() const {
            return mLights;
        }

        X_NODISCARD CameraComponent* GetMainCamera() {
            if (mCameras.empty()) {
                X_LOG_WARN("No main camera");
                return nullptr;
            }
            return *(mCameras.BeginMutable());
        }

        X_NODISCARD const CameraComponent* GetMainCamera() const {
            if (mCameras.empty()) {
                X_LOG_WARN("No main camera");
                return nullptr;
            }
            return *(mCameras.begin());
        }

        void Reset() {
            mEntities.clear();
            mNextId     = 0;
            mLights     = {};
            mTransforms = {};
            mModels     = {};
            mBehaviors  = {};
        }

        // Global state
        // Camera MainCamera;

    private:
        u64 mNextId = 0;
        std::map<EntityId, str> mEntities;
        LightState mLights;

        // Component managers
        ComponentManager<TransformComponent> mTransforms;
        ComponentManager<ModelComponent> mModels;
        ComponentManager<BehaviorComponent> mBehaviors;
        ComponentManager<CameraComponent> mCameras;
    };
}  // namespace x