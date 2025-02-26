#pragma once

#include "Common/Types.hpp"

namespace x {
    /// @brief Class trait signifying that this class needs to handle resizing when the window size changes.
    ///
    /// Classes that inherit this trait need to be be registered with the game instance in order for the resize handler
    /// to automatically get called. Static initialization might solve having to manually register volatiles, but that's
    /// a rabit hole I don't want to go down just yet.
    struct Volatile {
        virtual ~Volatile()                          = default;
        virtual void OnResize(u32 width, u32 height) = 0;
    };
}  // namespace x