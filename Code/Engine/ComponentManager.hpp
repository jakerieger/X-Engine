// Author: Jake Rieger
// Created: 1/13/2025.
//

#pragma once

#include "Common/Types.hpp"
#include "EntityId.hpp"

namespace x {
    template<typename T>
    class ComponentManager {
        vector<T> mComponents;
        unordered_map<EntityId, size_t> mEntityToIndex;
        vector<EntityId> mIndexToEntity;

    public:
        struct ComponentView {
            EntityId entity;
            T& component;

            operator T&() {
                return component;
            }

            operator T*() {
                return &component;
            }
        };

        struct ConstComponentView {
            EntityId entity;
            const T& component;

            operator const T&() const {
                return component;
            }

            operator const T*() const {
                return &component;
            }
        };

        class Iterator {
            vector<T>& mComponents;
            vector<EntityId>& mEntities;
            size_t mIndex;

        public:
            Iterator(vector<T>& components, vector<EntityId>& entities, size_t index)
                : mComponents(components), mEntities(entities), mIndex(index) {}

            ComponentView operator*() const {
                return {mEntities[mIndex], mComponents[mIndex]};
            }

            T* operator->() const {
                return &mComponents[mIndex];
            }

            Iterator& operator++() {
                ++mIndex;
                return *this;
            }

            bool operator!=(const Iterator& other) const {
                return mIndex != other.mIndex;
            }

            bool operator==(const Iterator& other) const {
                return mIndex == other.mIndex;
            }
        };

        class ConstIterator {
            const vector<T>& mComponents;
            const vector<EntityId>& mEntities;
            size_t mIndex;

        public:
            ConstIterator(const vector<T>& components, const vector<EntityId>& entities, size_t index)
                : mComponents(components), mEntities(entities), mIndex(index) {}

            ConstComponentView operator*() const {
                return {mEntities[mIndex], mComponents[mIndex]};
            }

            const T* operator->() const {
                return &mComponents[mIndex];
            }

            ConstIterator& operator++() {
                ++mIndex;
                return *this;
            }

            bool operator!=(const ConstIterator& other) const {
                return mIndex != other.mIndex;
            }

            bool operator==(const ConstIterator& other) const {
                return mIndex == other.mIndex;
            }
        };

        Iterator BeginMutable() {
            return {mComponents, mIndexToEntity, 0};
        };

        Iterator EndMutable() {
            return {mComponents, mIndexToEntity, mComponents.size()};
        }

        class MutableView {
            ComponentManager& mManager;

        public:
            MutableView(ComponentManager& manager) : mManager(manager) {}

            Iterator begin() const {
                return mManager.BeginMutable();
            }

            Iterator end() const {
                return mManager.EndMutable();
            }

            size_t size() const {
                return mManager.size();
            }

            bool empty() const {
                return mManager.empty();
            }
        };

        MutableView GetMutable() {
            return {*this};
        }

        ConstIterator begin() const {
            return ConstIterator(mComponents, mIndexToEntity, 0);
        }

        ConstIterator end() const {
            return ConstIterator(mComponents, mIndexToEntity, mComponents.size());
        }

        size_t size() const {
            return mComponents.size();
        }

        bool empty() const {
            return mComponents.empty();
        }

        ComponentView AddComponent(EntityId entity) {
            const size_t newIndex = mComponents.size();
            mComponents.emplace_back();
            mEntityToIndex[entity] = newIndex;
            mIndexToEntity.push_back(entity);
            return {entity, mComponents.back()};
        }

        void RemoveComponent(EntityId entity) {
            const auto it = mEntityToIndex.find(entity);
            if (it != mEntityToIndex.end()) {
                size_t indexToRemove = it->second;
                size_t lastIndex     = mComponents.size() - 1;
                if (indexToRemove != lastIndex) {
                    mComponents[indexToRemove]    = std::move(mComponents[lastIndex]);
                    EntityId movedEntity          = mIndexToEntity[lastIndex];
                    mEntityToIndex[movedEntity]   = indexToRemove;
                    mIndexToEntity[indexToRemove] = movedEntity;
                }
                mComponents.pop_back();
                mIndexToEntity.pop_back();
                mEntityToIndex.erase(entity);
            }
        }

        const T* GetComponent(EntityId entity) const {
            const auto it = mEntityToIndex.find(entity);
            if (it != mEntityToIndex.end()) { return &mComponents[it->second]; }
            return nullptr;
        }

        T* GetComponentMutable(EntityId entity) {
            const auto it = mEntityToIndex.find(entity);
            if (it != mEntityToIndex.end()) { return &mComponents[it->second]; }
            return nullptr;
        }

        EntityId GetEntity(const T* component) const {
            size_t index = component - mComponents.data();
            if (index < mComponents.size()) return mIndexToEntity[index];
            return EntityId {0};
        }

        const vector<T>& GetRawComponents() const {
            return mComponents;
        }
    };
}  // namespace x