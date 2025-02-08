// Author: Jake Rieger
// Created: 1/13/2025.
//

#pragma once

#include "Resource.hpp"
#include "Common/Types.hpp"
#include <limits>

namespace x {
    class EntityId {
    public:
        constexpr EntityId() : _value(kInvalidEntityId) {}
        explicit constexpr EntityId(u64 value) : _value(value) {}

        constexpr u64 value() const {
            return _value;
        }

        constexpr bool operator==(const EntityId& other) const {
            return _value == other._value;
        }

        constexpr bool operator!=(const EntityId& other) const {
            return _value != other._value;
        }

        constexpr bool operator<(const EntityId& other) const {
            return _value < other._value;
        }

        constexpr bool operator>(const EntityId& other) const {
            return _value > other._value;
        }

        constexpr bool operator<=(const EntityId& other) const {
            return _value <= other._value;
        }

        constexpr bool operator>=(const EntityId& other) const {
            return _value >= other._value;
        }

        constexpr u64 operator*() const {
            return _value;
        }

        constexpr bool valid() const {
            return _value != kInvalidEntityId;
        }

        static constexpr EntityId Invalid() {
            return {};
        }

    private:
        u64 _value;
        static constexpr u64 kInvalidEntityId = std::numeric_limits<u64>::max();
    };
} // namespace x

#ifndef X_ENTITY_ID_HASH_SPECIALIZATION
#define X_ENTITY_ID_HASH_SPECIALIZATION

// Allow EntityId to be used as a key with STL maps/sets
template<>
struct std::hash<x::EntityId> {
    std::size_t operator()(const x::EntityId& id) const noexcept {
        return std::hash<x::u64>{}(id.value());
    }
}; // namespace std
#endif