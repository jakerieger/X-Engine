#include "Mouse.hpp"

namespace x {
    void Mouse::CaptureMouse(HWND hwnd) {
        if (!mCaptured) {
            mCaptured = true;
            ::ShowCursor(FALSE);
            ::SetCapture(hwnd);

            RECT windowRect;
            ::GetClientRect(hwnd, &windowRect);

            POINT center = {
              (windowRect.right - windowRect.left) / 2,
              (windowRect.bottom - windowRect.top) / 2,
            };

            mLastPos = center;

            ::ClientToScreen(hwnd, &center);
            ::SetCursorPos(center.x, center.y);
        }
    }

    void Mouse::ReleaseMouse(HWND hwnd) {
        if (mCaptured) {
            mCaptured = false;
            ::ShowCursor(TRUE);
            ::ReleaseCapture();

            ::ClientToScreen(hwnd, &mLastPos);
            ::SetCursorPos(mLastPos.x, mLastPos.y);
        }
    }

    void Mouse::OnMouseMove(HWND hwnd, Input& input, i32 xPos, i32 yPos) const {
        if (mCaptured) {
            RECT windowRect;
            ::GetClientRect(hwnd, &windowRect);
            POINT centerPoint = {(windowRect.right - windowRect.left) / 2, (windowRect.bottom - windowRect.top) / 2};

            POINT currentPos = {xPos, yPos};

            // Only update if the position has actually changed
            if (currentPos.x != centerPoint.x || currentPos.y != centerPoint.y) {
                // Calculate delta from center
                const int deltaX = currentPos.x - centerPoint.x;
                const int deltaY = currentPos.y - centerPoint.y;

                // Only update input if we have actual movement
                if (deltaX != 0 || deltaY != 0) {
                    input.UpdateMousePosition(deltaX, deltaY);

                    // Reset cursor to center
                    ::ClientToScreen(hwnd, &centerPoint);
                    ::SetCursorPos(centerPoint.x, centerPoint.y);
                }
            } else {
                // If no movement, ensure deltas are zero
                input.UpdateMousePosition(0, 0);
            }
        } else {
            input.UpdateMousePosition(xPos, yPos);
        }
    }
}  // namespace x