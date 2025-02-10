#pragma once

#include "Common/Types.hpp"
#include "ArenaAllocator.hpp"
#include "EngineCommon.hpp"
#include "RenderContext.hpp"

namespace x {
    class ShaderManager {
        CLASS_PREVENT_MOVES_COPIES(ShaderManager)

        ArenaAllocator _allocator;
        RenderContext& _renderContext;
    };
}