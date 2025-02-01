#include "Common.hlsli"
#include "IO.hlsli"

VSOutputPBR VS_Main(VSInputPBR input) {
    VSOutputPBR output;

    float4 pos = float4(input.position, 1.0f);
    float4 mvp = mul(pos, Transforms.modelViewProjection);

    output.position = mvp;
    output.positionCS = output.position;
    output.worldPos = mul(pos, Transforms.model).xyz;

    output.texCoord0 = input.texCoord0;
    output.texCoord1 = input.texCoord1;
    
    output.normal = TransformNormal(input.normal, Transforms.model);
    output.normal = normalize(output.normal);

    output.tangent = TransformNormal(input.tangent, Transforms.model);
    output.tangent = normalize(output.tangent);

    output.bitangent = CalculateBitangent(output.normal, output.tangent, 1.0f);

    return output;
}

float3 CalculateWorldNormal(float3 tangent, float3 bitangent, float3 normal, float3 normalMap) {
    float3x3 TBN = float3x3(
        normalize(tangent),
        normalize(bitangent),
        normalize(normal)
    );
    
    float3 worldNormal = mul(normalMap, TBN);
    worldNormal = normalize(worldNormal);

    return worldNormal;
}

float4 PS_Main(VSOutputPBR input) : SV_Target {
    float3 normalMap = NormalMap.Sample(NormalState, input.texCoord0).rgb;
    normalMap = normalMap * 2.0f - 1.0f; // convert from [0,1] to [-1,1]
    float3 N = CalculateWorldNormal(input.tangent, input.bitangent, input.normal, normalMap);

    float4 albedoSample = AlbedoMap.Sample(AlbedoState, input.texCoord0);
    float metallicSample = MetallicMap.Sample(MetallicState, input.texCoord0).r;
    float roughnessSample = RoughnessMap.Sample(RoughnessState, input.texCoord0).r;
    float ao = 1.0f;

    float3 V = normalize(CameraPosition.xyz - input.worldPos);
    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), albedoSample.rgb, metallicSample);
    float3 Lo = albedoSample.rgb * 0.01f;

    PBRMaterial mat;
    mat.albedo = albedoSample.rgb;
    mat.metallic = metallicSample;
    mat.roughness = roughnessSample;
    mat.ao = ao;
    mat.emissive = float3(0.0, 0.0, 0.0);
    mat.emissiveStrength = 0.0;

    if (Sun.enabled) {
        float3 L = normalize(-Sun.direction);
        float3 specular = SpecularBRDF(N, V, L, roughnessSample, F0);
        float3 diffuse = EnergyConservation(specular, metallicSample, albedoSample.rgb);
        float NdotL = max(dot(N, L), 0.01f);
        float3 radiance = Sun.color * Sun.intensity;
        Lo += (diffuse / PI + specular) * radiance * NdotL;
    }

    // for (uint i = 0; i < MAX_POINT_LIGHTS; ++i) {
    //     PointLight light = PointLights[i];
    //     if (light.enabled) {
    //         Lo += CalculatePointLightPBR(light, input.worldPos, N, V, mat);
    //     }
    // }
    //
    // for (uint j = 0; j < MAX_AREA_LIGHTS; ++j) {
    //     AreaLight light = AreaLights[j];
    //     if (light.enabled) {
    //         Lo += CalculateAreaLightPBR(light, input.worldPos, N, V, mat);
    //     }
    // }

    // Final color calculation
        float3 color = Lo * ao;

    float exposure = 1.0f;
    color *= exposure;
    color = TonemapACES(color);
    
    return float4(color, albedoSample.a);
}