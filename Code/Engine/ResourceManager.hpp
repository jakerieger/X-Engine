#pragma once

#include "Common/Types.hpp"
#include "Memory.hpp"
#include "ArenaAllocator.hpp"

namespace x {
    class ResourceManager {
        CLASS_PREVENT_MOVES_COPIES(ResourceManager)

        ArenaAllocator _allocator;

    public:
        ResourceManager() : _allocator(Memory::BYTES_256MB) {}
    };
}