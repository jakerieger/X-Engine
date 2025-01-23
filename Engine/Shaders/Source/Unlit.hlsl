#include "Common.hlsli"
#include "IO.hlsli"

VSOutputPosTexNormal VS_Main(VSInputPosTexNormal input) {
    VSOutputPosTexNormal output;

    output.position = float4(input.position, 1.0f);
    output.texCoord = input.texCoord;
    output.normal = input.normal;

    return output;
}

float4 PS_Main(VSOutputPosTexNormal input) : SV_Target {
    return float4(input.texCoord, 0.0f, 1.0f);
}