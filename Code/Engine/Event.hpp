// Author: Jake Rieger
// Created: 2/18/2025.
//

#pragma once

#include "Common/Types.hpp"

namespace x {
    class Event {
    public:
        virtual ~Event()            = default;
        virtual str GetType() const = 0;
    };

    template<typename T>
    concept EventType = BaseOf<Event, T>;

    class WindowResizeEvent final : public Event {
    public:
        WindowResizeEvent(u32 width, u32 height) : _width(width), _height(height) {}

        str GetType() const override {
            return "WindowResizeEvent";
        }
        u32 GetWidth() const {
            return _width;
        }
        u32 GetHeight() const {
            return _height;
        }

    private:
        u32 _width;
        u32 _height;
    };

    class WindowLostFocusEvent final : public Event {
    public:
        WindowLostFocusEvent() = default;

        str GetType() const override {
            return "WindowLostFocusEvent";
        }
    };

    class WindowFocusEvent final : public Event {
    public:
        WindowFocusEvent() = default;
        str GetType() const override {
            return "WindowFocusEvent";
        }
    };

    class KeyPressedEvent final : public Event {
    public:
        explicit KeyPressedEvent(u32 keycode) : _keycode(keycode) {};

        str GetType() const override {
            return "KeyPressedEvent";
        }

        u32 GetKey() const {
            return _keycode;
        }

    private:
        u32 _keycode;
    };

    class KeyReleasedEvent final : public Event {
    public:
        explicit KeyReleasedEvent(u32 keycode) : _keycode(keycode) {};

        str GetType() const override {
            return "KeyPressedEvent";
        }

        u32 GetKey() const {
            return _keycode;
        }

    private:
        u32 _keycode;
    };

    class MouseButtonPressedEvent final : public Event {
    public:
        explicit MouseButtonPressedEvent(u32 button) : _button(button) {};

        str GetType() const override {
            return "MouseButtonPressedEvent";
        }

        u32 GetButton() const {
            return _button;
        }

    private:
        u32 _button;
    };

    class MouseButtonReleasedEvent final : public Event {
    public:
        explicit MouseButtonReleasedEvent(u32 button) : _button(button) {};

        str GetType() const override {
            return "MouseButtonReleasedEvent";
        }

        u32 GetButton() const {
            return _button;
        }

    private:
        u32 _button;
    };

    class MouseMoveEvent final : public Event {
    public:
        explicit MouseMoveEvent(i32 x, i32 y) : _x(x), _y(y) {};

        str GetType() const override {
            return "MouseMoveEvent";
        }

        i32 GetX() const {
            return _x;
        }

        i32 GetY() const {
            return _y;
        }

    private:
        i32 _x;
        i32 _y;
    };
}  // namespace x
