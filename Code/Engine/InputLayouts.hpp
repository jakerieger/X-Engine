#pragma once

#include "Math.hpp"

namespace x {
    struct VSInputPos {
        Float3 mPosition;
    };

    struct VSInputPosTex {
        Float3 mPosition;
        Float2 mTexCoord;
    };

    struct VSInputPosTexNormal {
        Float3 mPosition;
        Float3 mNormal;
        Float2 mTexCoord;
    };

    struct VSInputPBR {
        Float3 mPosition;
        Float3 mNormal;
        Float3 mTangent;
        Float2 mTexCoord;
    };

    struct VSInputSkinned {
        Float3 mPosition;
        Float3 mNormal;
        Float3 mTangent;
        Float2 mTexCoord;
        Uint4 mBoneIds;
        Float4 mWeights;
    };

    template<typename T>
    concept HasTexCoord0 = requires(T t) { t.mTexCoord; };

    template<typename T>
    concept HasNormal = requires(T t) { t.mNormal; };

    template<typename T>
    concept HasTangent = requires(T t) { t.mTangent; };

    template<typename T>
    concept HasBoneIds = requires(T t) { t.mBoneIds; };

    template<typename T>
    concept HasWeights = requires(T t) { t.mWeights; };
}  // namespace x