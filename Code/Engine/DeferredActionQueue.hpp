// Author: Jake Rieger
// Created: 4/19/2025.
//

#pragma once

#include <queue>
#include <functional>
#include "Common/Macros.hpp"
#include "Common/Typedefs.hpp"

namespace x {
    class DeferredActionQueue {
    public:
        template<typename Func>
        void Defer(Func&& action) {
            mActions.push(std::forward<Func>(action));
        }

        void Execute() {
            while (!mActions.empty()) {
                auto action = mActions.front();
                mActions.pop();
                action();
            }
        }

        bool IsEmpty() const {
            return mActions.empty();
        }

        X_NODISCARD size_t Size() const {
            return mActions.size();
        }

    private:
        std::queue<std::function<void()>> mActions;
    };
}  // namespace x
