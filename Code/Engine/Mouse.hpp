#pragma once

#include "EngineCommon.hpp"
#include "Platform.hpp"
#include "Input.hpp"

namespace x {
    class Mouse {
        friend class Game;
        X_CLASS_PREVENT_MOVES_COPIES(Mouse)

    public:
        Mouse() = default;

    private:
        bool _captured = false;
        POINT _lastPos = {0, 0};

        void CaptureMouse(HWND hwnd);
        void ReleaseMouse(HWND hwnd);
        void OnMouseMove(HWND hwnd, Input& input, i32 xPos, i32 yPos) const;

        X_NODISCARD bool IsCaptured() const {
            return _captured;
        }
    };
}