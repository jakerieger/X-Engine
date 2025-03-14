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
        constexpr EntityId() : mValue(kInvalidEntityId) {}
        explicit constexpr EntityId(u64 value) : mValue(value) {}

        constexpr u64 Value() const {
            return mValue;
        }

        constexpr bool operator==(const EntityId& other) const {
            return mValue == other.mValue;
        }

        constexpr bool operator!=(const EntityId& other) const {
            return mValue != other.mValue;
        }

        constexpr bool operator<(const EntityId& other) const {
            return mValue < other.mValue;
        }

        constexpr bool operator>(const EntityId& other) const {
            return mValue > other.mValue;
        }

        constexpr bool operator<=(const EntityId& other) const {
            return mValue <= other.mValue;
        }

        constexpr bool operator>=(const EntityId& other) const {
            return mValue >= other.mValue;
        }

        constexpr u64 operator*() const {
            return mValue;
        }

        constexpr bool Valid() const {
            return mValue != kInvalidEntityId;
        }

        static constexpr EntityId Invalid() {
            return {};
        }

    private:
        u64 mValue;
        static constexpr u64 kInvalidEntityId = std::numeric_limits<u64>::max();
    };
}  // namespace x

#ifndef X_ENTITY_ID_HASH_SPECIALIZATION
    #define X_ENTITY_ID_HASH_SPECIALIZATION

// Allow EntityId to be used as a key with STL maps/sets
template<>
struct std::hash<x::EntityId> {
    std::size_t operator()(const x::EntityId& id) const noexcept {
        return std::hash<x::u64> {}(id.Value());
    }
};  // namespace std
#endif