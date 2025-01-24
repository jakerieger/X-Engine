#pragma once

#include "Math.hpp"

namespace x {
    struct VSInputPos {
        Float3 position;
    };

    struct VSInputPosTex {
        Float3 position;
        Float2 texCoord;
    };

    struct VSInputPosTexNormal {
        Float3 position;
        Float3 normal;
        Float2 texCoord;
    };

    struct VSInputPBR {
        Float3 position;
        Float3 normal;
        Float3 tangent;
        Float2 texCoord0;
        Float2 texCoord1;
    };

    struct VSInputSkinned {
        Float3 position;
        Float3 normal;
        Float3 tangent;
        Float2 texCoord;
        Uint4 boneIds;
        Float4 weights;
    };
}