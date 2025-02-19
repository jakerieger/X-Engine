// Author: Jake Rieger
// Created: 2/18/2025.
//

#pragma once

#include "EventListener.hpp"

namespace x {
    class EventEmitter {
    public:
        void AddListener(EventListener* listener) {
            _listeners.push_back(listener);
        }

        void RemoveListener(EventListener* listener) {
            _listeners.erase(std::remove(_listeners.begin(), _listeners.end(), listener));
        }

    protected:
        void Emit(const Event& e) const {
            for (const auto& listener : _listeners) {
                listener->HandleEvent(e);
            }
        }

    private:
        vector<EventListener*> _listeners;
    };
}  // namespace x
