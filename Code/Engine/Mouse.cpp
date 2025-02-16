#include "Mouse.hpp"

namespace x {
    void Mouse::CaptureMouse(HWND hwnd) {
        if (!_captured) {
            _captured = true;
            ::ShowCursor(FALSE);
            ::SetCapture(hwnd);

            ::GetCursorPos(&_lastPos);
            ::ScreenToClient(hwnd, &_lastPos);

            RECT windowRect;
            ::GetClientRect(hwnd, &windowRect);

            POINT center = {
                (windowRect.right - windowRect.left) / 2,
                (windowRect.bottom - windowRect.top) / 2,
            };

            ::ClientToScreen(hwnd, &center);
            ::SetCursorPos(center.x, center.y);
        }
    }

    void Mouse::ReleaseMouse(HWND hwnd) {
        if (_captured) {
            _captured = false;
            ::ShowCursor(TRUE);
            ::ReleaseCapture();

            ::ClientToScreen(hwnd, &_lastPos);
            ::SetCursorPos(_lastPos.x, _lastPos.y);
        }
    }

    void Mouse::OnMouseMove(HWND hwnd, Input& input, LPARAM lParam) {
        if (_captured) {
            RECT windowRect;
            ::GetClientRect(hwnd, &windowRect);
            POINT centerPoint = {(windowRect.right - windowRect.left) / 2,
                                 (windowRect.bottom - windowRect.top) / 2};

            POINT currentPos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

            // Calculate delta from center
            const int deltaX = currentPos.x - centerPoint.x;
            const int deltaY = currentPos.y - centerPoint.y;

            if (deltaX != 0 || deltaY != 0) {
                input.UpdateMousePosition(deltaX, deltaY);

                // Reset cursor to center
                ::ClientToScreen(hwnd, &centerPoint);
                ::SetCursorPos(centerPoint.x, centerPoint.y);
            }
        } else {
            const int xPos = GET_X_LPARAM(lParam);
            const int yPos = GET_Y_LPARAM(lParam);
            input.UpdateMousePosition(xPos, yPos);
        }
    }
}