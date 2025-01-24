#include "Common.hlsli"
#include "IO.hlsli"

VSOutputPosTexNormal VS_Main(VSInputPosTexNormal input) {
    VSOutputPosTexNormal output;

    float4 pos = float4(input.position, 1.0f);
    float4 mvp = mul(pos, Transforms.modelViewProjection);

    output.position = mvp;
    output.texCoord = input.texCoord;
    output.normal = input.normal;

    return output;
}

float4 PS_Main(VSOutputPosTexNormal input) : SV_Target {
    return float4(input.normal, 1.0f);
}