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
        WindowResizeEvent(u32 width, u32 height) : mWidth(width), mHeight(height) {}

        str GetType() const override {
            return "WindowResizeEvent";
        }
        u32 GetWidth() const {
            return mWidth;
        }
        u32 GetHeight() const {
            return mHeight;
        }

    private:
        u32 mWidth;
        u32 mHeight;
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
        explicit KeyPressedEvent(u32 keycode) : mKeycode(keycode) {};

        str GetType() const override {
            return "KeyPressedEvent";
        }

        u32 GetKey() const {
            return mKeycode;
        }

    private:
        u32 mKeycode;
    };

    class KeyReleasedEvent final : public Event {
    public:
        explicit KeyReleasedEvent(u32 keycode) : mKeycode(keycode) {};

        str GetType() const override {
            return "KeyPressedEvent";
        }

        u32 GetKey() const {
            return mKeycode;
        }

    private:
        u32 mKeycode;
    };

    class MouseButtonPressedEvent final : public Event {
    public:
        explicit MouseButtonPressedEvent(u32 button) : mButton(button) {};

        str GetType() const override {
            return "MouseButtonPressedEvent";
        }

        u32 GetButton() const {
            return mButton;
        }

    private:
        u32 mButton;
    };

    class MouseButtonReleasedEvent final : public Event {
    public:
        explicit MouseButtonReleasedEvent(u32 button) : mButton(button) {};

        str GetType() const override {
            return "MouseButtonReleasedEvent";
        }

        u32 GetButton() const {
            return mButton;
        }

    private:
        u32 mButton;
    };

    class MouseMoveEvent final : public Event {
    public:
        explicit MouseMoveEvent(i32 x, i32 y) : mX(x), mY(y) {};

        str GetType() const override {
            return "MouseMoveEvent";
        }

        i32 GetX() const {
            return mX;
        }

        i32 GetY() const {
            return mY;
        }

    private:
        i32 mX;
        i32 mY;
    };
}  // namespace x
