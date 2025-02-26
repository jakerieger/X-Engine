// Author: Jake Rieger
// Created: 2/18/2025.
//

#pragma once

#include "EventListener.hpp"

namespace x {
    class EventEmitter {
    public:
        void AddListener(EventListener* listener) {
            mListeners.push_back(listener);
        }

        void RemoveListener(EventListener* listener) {
            mListeners.erase(std::remove(mListeners.begin(), mListeners.end(), listener));
        }

    protected:
        void Emit(const Event& e) const {
            for (const auto& listener : mListeners) {
                listener->HandleEvent(e);
            }
        }

    private:
        vector<EventListener*> mListeners;
    };
}  // namespace x
