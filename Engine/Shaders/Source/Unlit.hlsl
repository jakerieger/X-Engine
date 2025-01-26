#include "Common.hlsli"
#include "IO.hlsli"

VSOutputPBR VS_Main(VSInputPBR input) {
    VSOutputPBR output;

    float4 pos = float4(input.position, 1.0f);
    float4 mvp = mul(pos, Transforms.modelViewProjection);

    output.position = mvp;
    output.texCoord0 = input.texCoord0;
    output.normal = input.normal;

    return output;
}

float4 PS_Main(VSOutputPBR input) : SV_Target {
    float3 normal = normalize(input.normal);
    return float4(normal, 1.0f);
}