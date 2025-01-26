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
        Float2 texCoord;
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

    template<typename T>
    concept HasTexCoord0 = requires(T t) { t.texCoord; };

    template<typename T>
    concept HasNormal = requires(T t) { t.normal; };

    template<typename T>
    concept HasTangent = requires(T t) { t.tangent; };

    template<typename T>
    concept HasTexCoord1 = requires(T t) { t.texCoord1; };

    template<typename T>
    concept HasBoneIds = requires(T t) { t.boneIds; };

    template<typename T>
    concept HasWeights = requires(T t) { t.weights; };
}