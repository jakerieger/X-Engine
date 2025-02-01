#include "Common.hlsli"
#include "IO.hlsli"

VSOutputPBR VS_Main(VSInputPBR input) {
    VSOutputPBR output;

    float4 pos = float4(input.position, 1.0f);
    float4 mvp = mul(pos, Transforms.modelViewProjection);

    output.position = mvp;
    output.texCoord0 = input.texCoord0;
    
    output.normal = TransformNormal(input.normal, Transforms.model);
    output.normal = normalize(output.normal);

    output.tangent = TransformNormal(input.tangent, Transforms.model);
    output.tangent = normalize(output.tangent);

    output.bitangent = cross(output.normal, output.tangent);
    output.worldPos = mul(pos, Transforms.model).xyz;

    return output;
}

float4 PS_Main(VSOutputPBR input) : SV_Target {
    // Just hard-coding these for now, this shader will likely only be used for debug purposes.
    float3 ambient = float3(0.05f, 0.05f, 0.05f);
    const float intensity = 1.0f;

    float3 normalMap = NormalMap.Sample(NormalState, input.texCoord0).rgb;
    normalMap = normalMap * 2.0f - 1.0f; // convert from [0,1] to [-1,1]

    float3x3 TBN = float3x3(
        normalize(input.tangent),
        normalize(input.bitangent),
        normalize(input.normal)
    );

    float3 worldNormal = mul(normalMap, TBN);
    worldNormal = normalize(worldNormal);

    float3 viewDir = normalize(CameraPosition.xyz - input.worldPos);

    float diffStrength = max(dot(input.normal, Sun.direction), 0.0f);
    float3 diffuse = AlbedoMap.Sample(AlbedoState, input.texCoord0) * diffStrength;

    float3 halfwayDir = normalize(Sun.direction + viewDir);
    float3 specDot = dot(worldNormal, halfwayDir);
    float specStrength = pow(max(specDot, 0.0f), 32.0f);
    float3 specular = float3(1.0f, 1.0f, 1.0f) * specStrength;

    float3 finalColor = (diffuse + ambient + specular) * Sun.color * intensity;

    return float4(finalColor, 1.0f);
}