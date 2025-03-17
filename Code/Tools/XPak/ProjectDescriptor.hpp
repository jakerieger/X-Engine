// Author: Jake Rieger
// Created: 3/2/2025.
//

#pragma once

#include <format>

#include "Common/Types.hpp"
#include "Common/Filesystem.hpp"
#include <yaml-cpp/yaml.h>

namespace x {
    struct ProjectDescriptor {
        str mName;
        f32 mEngineVersion;
        str mContentDirectory;
        str mStartupScene;
        bool mLoaded {false};

        bool FromFile(const str& filename);
        std::string ToString() const;
    };
}  // namespace x
