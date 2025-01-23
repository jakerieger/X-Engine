// Author: Jake Rieger
// Created: 1/13/2025.
//

#pragma once

#include "Common/Types.hpp"
#include "EntityId.hpp"

namespace x {
    template<typename T>
    class ComponentManager {
        vector<T> _components;
        unordered_map<EntityId, size_t> _entityToIndex;
        vector<EntityId> _indexToEntity;

    public:
        void ReleaseResources() {
            if constexpr (detail::release_resources<T>::value) {
                for (auto& component : _components) {
                    component.Release();
                }
            }
        }

        struct ComponentView {
            EntityId entity;
            T& component;
        };

        struct ConstComponentView {
            EntityId entity;
            const T& component;
        };

        class Iterator {
            vector<T>& _components;
            vector<EntityId>& _entities;
            size_t _index;

        public:
            Iterator(vector<T>& components, vector<EntityId>& entities, size_t index)
                : _components(components), _entities(entities), _index(index) {}

            ComponentView operator*() const {
                return {_entities[_index], _components[_index]};
            }

            Iterator& operator++() {
                ++_index;
                return *this;
            }

            bool operator!=(const Iterator& other) const {
                return _index != other._index;
            }

            bool operator==(const Iterator& other) const {
                return _index == other._index;
            }
        };

        class ConstIterator {
            const vector<T>& _components;
            const vector<EntityId>& _entities;
            size_t _index;

        public:
            ConstIterator(const vector<T>& components,
                          const vector<EntityId>& entities,
                          size_t index)
                : _components(components), _entities(entities), _index(index) {}

            ConstComponentView operator*() const {
                return {_entities[_index], _components[_index]};
            }

            ConstIterator& operator++() {
                ++_index;
                return *this;
            }

            bool operator!=(const ConstIterator& other) const {
                return _index != other._index;
            }

            bool operator==(const ConstIterator& other) const {
                return _index == other._index;
            }
        };

        Iterator BeginMutable() {
            return {_components, _indexToEntity, 0};
        };

        Iterator EndMutable() {
            return {_components, _indexToEntity, _components.size()};
        }

        class MutableView {
            ComponentManager& _manager;

        public:
            MutableView(ComponentManager& manager) : _manager(manager) {}

            Iterator begin() const {
                return _manager.BeginMutable();
            }

            Iterator end() const {
                return _manager.EndMutable();
            }
        };

        MutableView GetMutable() {
            return {*this};
        }

        ConstIterator begin() const {
            return ConstIterator(_components, _indexToEntity, 0);
        }

        ConstIterator end() const {
            return ConstIterator(_components, _indexToEntity, _components.size());
        }

        ComponentView AddComponent(EntityId entity) {
            const size_t newIndex = _components.size();
            _components.emplace_back();
            _entityToIndex[entity] = newIndex;
            _indexToEntity.push_back(entity);
            return {entity, _components.back()};
        }

        void RemoveComponent(EntityId entity) {
            const auto it = _entityToIndex.find(entity);
            if (it != _entityToIndex.end()) {
                size_t indexToRemove = it->second;
                size_t lastIndex     = _components.size() - 1;
                if (indexToRemove != lastIndex) {
                    _components[indexToRemove]    = std::move(_components[lastIndex]);
                    EntityId movedEntity          = _indexToEntity[lastIndex];
                    _entityToIndex[movedEntity]   = indexToRemove;
                    _indexToEntity[indexToRemove] = movedEntity;
                }
                _components.pop_back();
                _indexToEntity.pop_back();
                _entityToIndex.erase(entity);
            }
        }

        const T* GetComponent(EntityId entity) const {
            const auto it = _entityToIndex.find(entity);
            if (it != _entityToIndex.end()) { return &_components[it->second]; }
            return None;
        }

        T* GetComponentMutable(EntityId entity) {
            const auto it = _entityToIndex.find(entity);
            if (it != _entityToIndex.end()) { return &_components[it->second]; }
            return None;
        }

        EntityId GetEntity(const T* component) const {
            size_t index = component - _components.data();
            if (index < _components.size())
                return _indexToEntity[index];
            return EntityId{0};
        }

        const vector<T>& GetRawComponents() const {
            return _components;
        }
    };
} // namespace x