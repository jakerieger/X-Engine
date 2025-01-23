// Author: Jake Rieger
// Created: 1/13/2025.
//

#pragma once

namespace x {
    /// @brief Class trait signifying that this object contains memory resources that need to be properly cleaned up
    struct Resource {
        virtual void Release() = 0;
        virtual ~Resource() = default;
    };
} // namespace x