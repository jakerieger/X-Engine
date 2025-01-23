#pragma once

#include "Common/Types.hpp"
#include "D3D.hpp"

namespace x {
    using float4 = XMFLOAT4;
    using float3 = XMFLOAT3;
    using float2 = XMFLOAT2;
    using uint4  = XMUINT4;

    struct VSInputPos {
        float3 position;
    };

    struct VSInputPosTex {
        float3 position;
        float2 texCoord;
    };

    struct VSInputPosTexNormal {
        float3 position;
        float3 normal;
        float2 texCoord;
    };

    struct VSInputPBR {
        float3 position;
        float3 normal;
        float3 tangent;
        float2 texCoord0;
        float2 texCoord1;
    };

    struct VSInputSkinned {
        float3 position;
        float3 normal;
        float3 tangent;
        float2 texCoord;
        uint4 boneIds;
        float4 weights;
    };
}