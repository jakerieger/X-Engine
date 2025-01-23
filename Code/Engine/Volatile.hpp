#pragma once

#include "Common/Types.hpp"

namespace x {
    /// @brief Class trait signifying that this class needs to handle resizing when the window size changes
    struct Volatile {
        virtual ~Volatile() = default;
        virtual void OnResize(u32 width, u32 height) = 0;
    };
}