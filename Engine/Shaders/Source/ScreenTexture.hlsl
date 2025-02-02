#include "IO.hlsli"

VSOutputPosTex VS_Main(uint vertexID : SV_VertexID) {
    VSOutputPosTex output;

    // Oversized triangle technique for fullscreen quad
    float2 positions[3] =
    {
        float2(-1.0f, -1.0f),
        float2(-1.0f, 3.0f),
        float2(3.0f, -1.0f)
    };

    output.position = float4(positions[vertexID], 0.0f, 1.0f);
    output.texCoord = output.position.xy * float2(0.5f, -0.5f) + 0.5f;

    return output;
}

Texture2D SceneTexture : register(t0);
SamplerState SceneState : register(s0);

float4 PS_Main(VSOutputPosTex input) : SV_Target {
    float4 color = SceneTexture.Sample(SceneState, input.texCoord);

    // post processing here
    color.rgb = float3(1.0 - color.r, 1.0 - color.g, 1.0 - color.b);

    return color;
}