// Author: Jake Rieger
// Created: 3/2/2025.
//

#pragma once

#include <format>
#include "Common/Types.hpp"
#include "Common/Filesystem.hpp"

namespace x {
    struct ProjectDescriptor {
        str mName;
        f32 mEngineVersion;
        str mContentDirectory;
        str mStartupScene;
        bool mLoaded {false};

        bool FromFile(const Path& filename);
        bool ToFile(const Path& filename) const;
        std::string ToString() const;
    };
}  // namespace x
