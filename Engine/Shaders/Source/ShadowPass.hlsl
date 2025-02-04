#include "IO.hlsli"

cbuffer ShadowParams : register (b0) {
    matrix lightViewProj;
    matrix world;
}

VSOutputPos VS_Main(VSInputPos input) {
    VSOutputPos output;

    float4 worldPos = mul(float4(input.position, 1.0f), world);
    output.position = mul(worldPos, lightViewProj);

    return output;
}

float4 PS_Main(VSOutputPos input) : SV_Target {
    // float depth = input.position.z / input.position.w;
    // // Add some debug coloring to see the range of values
    // if (depth < 0.0f || depth > 1.0f)
    // {
    //     return float4(1, 0, 0, 1); // Red for out-of-range values
    // }
    // return float4(depth.xxx, 1);
    return 0;
}