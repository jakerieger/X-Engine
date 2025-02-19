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
}  // namespace x
