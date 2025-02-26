#include "Common.hlsli"
#include "IO.hlsli"

VSOutputPBR VS_Main(VSInputPBR input) {
    VSOutputPBR output;

    float4 pos        = float4(input.position, 1.0f);
    float4 mvp        = mul(pos, Transforms.modelViewProjection);

    output.position   = mvp;
    output.positionCS = output.position;
    float4 worldPos = mul(pos, Transforms.model);
    output.worldPos = worldPos.xyz;
    output.lightSpacePos = mul(worldPos, Sun.lightViewProj);

    output.texCoord0  = input.texCoord0;

    output.normal     = TransformNormal(input.normal, Transforms.model);
    output.normal     = normalize(output.normal);

    output.tangent    = TransformNormal(input.tangent, Transforms.model);
    output.tangent    = normalize(output.tangent);
    output.bitangent  = CalculateBitangent(output.normal, output.tangent, 1.0f);

    return output;
}

float3 CalculateWorldNormal(float3 tangent, float3 bitangent, float3 normal, float3 normalMap) {
    float3x3 TBN       = float3x3(normalize(tangent), normalize(bitangent), normalize(normal));
    float3 worldNormal = mul(normalMap, TBN);
    worldNormal        = normalize(worldNormal);

    return worldNormal;
}

float4 PS_Main(VSOutputPBR input) : SV_Target {
    return float4(1.0, 0.0, 0.0, 1.0);
}