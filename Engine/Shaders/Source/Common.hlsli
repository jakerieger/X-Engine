#ifndef GRAPHICS_COMMON_HLSL
#define GRAPHICS_COMMON_HLSL

static const uint MAX_POINT_LIGHTS = 16;
static const uint MAX_SPOT_LIGHTS = 16;
static const uint MAX_AREA_LIGHTS = 16;

struct TransformMatrices {
    matrix model;
    matrix view;
    matrix projection;
    matrix modelView;
    matrix viewProjection;
    matrix modelViewProjection;
};

struct DirectionalLight {
    float3 direction;
    float3 color;
    float intensity;
    bool castsShadow;
    bool enabled;
    float2 _pad;
};

struct PointLight {
    float3 position;
    float3 color;
    float intensity;
    float constant;
    float lin;
    float quadratic;
    float radius;
    bool castsShadow;
    bool enabled;
    float2 _pad;
};

struct SpotLight {
    float3 position;
    float3 direction;
    float3 color;
    float intensity;
    float innerAngle;
    float outerAngle;
    float range;
    bool castsShadow;
    bool enabled;
    float2 _pad;
};

struct AreaLight {
    float3 position;
    float3 direction;
    float3 color;
    float2 dimensions;
    float intensity;
    bool castsShadow;
    bool enabled;
    float2 _pad;
};

cbuffer TransformBuffer : register(b0) {
    TransformMatrices Transforms;
}

cbuffer LightBuffer : register(b1) {
    DirectionalLight Sun;
    PointLight PointLights[MAX_POINT_LIGHTS];
    SpotLight SpotLights[MAX_SPOT_LIGHTS];
    AreaLight AreaLights[MAX_AREA_LIGHTS];
}

float CalculateAttenuation(float distance, float constant, float lin, float quadratic) {
    return 1.0f / (constant + lin * distance + quadratic * distance * distance);
}

float CalculateSpotIntensity(float3 lightDir, float3 spotDir, float innerCos, float outerCos) {
    float cosAngle = dot(normalize(-lightDir), normalize(spotDir));
    return smoothstep(outerCos, innerCos, cosAngle);
}

#endif