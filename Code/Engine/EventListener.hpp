// Author: Jake Rieger
// Created: 2/18/2025.
//

#pragma once

#include "Common/Types.hpp"
#include "Event.hpp"
#include <functional>
#include <typeindex>

namespace x {
    class EventListener {
    public:
        template<typename T>
        using EventHandler = std::function<void(const T&)>;

        using EventCallback = std::function<void(const Event&)>;

        template<EventType T>
        void RegisterHandler(EventHandler<T> handler) {
            const auto type = std::type_index(typeid(T));
            _handlers[type].push_back([handler](const Event& e) { handler(CAST<const T&>(e)); });
        }

        void HandleEvent(const Event& e) {
            const auto type = std::type_index(typeid(e));
            if (_handlers.contains(type)) {
                for (auto& handler : _handlers[type]) {
                    handler(e);
                }
            }
        }

    private:
        unordered_map<std::type_index, vector<EventCallback>> _handlers;
    };
}  // namespace x
