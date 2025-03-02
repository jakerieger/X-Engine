#ifndef GRAPHICS_COMMON_HLSL
#define GRAPHICS_COMMON_HLSL

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
static const uint MAX_POINT_LIGHTS = 16;
static const uint MAX_SPOT_LIGHTS  = 16;
static const uint MAX_AREA_LIGHTS  = 16;

//-----------------------------------------------------------------------------
// Structures
//-----------------------------------------------------------------------------
struct TransformMatrices {
    matrix model;
    matrix view;
    matrix projection;
    matrix modelView;
    matrix viewProjection;
    matrix modelViewProjection;
};

struct DirectionalLight {
    float4 direction;
    float4 color;
    float intensity;
    float3 _pad1;
    bool castsShadow;
    bool enabled;
    float2 _pad2;
    matrix lightViewProj;
};

struct PointLight {
    float3 position;
    float padding1;
    float3 color;
    float intensity;
    float constant;
    float lin;
    float quadratic;
    float radius;
    bool castsShadow;
    bool enabled;
    float2 padding2;
};

struct SpotLight {
    float3 position;
    float _padding1;

    float3 direction;
    float _padding2;

    float3 color;
    float intensity;

    float innerAngle;
    float outerAngle;
    float range;
    bool castsShadow;

    bool enabled;
    float3 _pad;
};

struct AreaLight {
    float3 position;
    float _pad1;
    float3 direction;
    float _pad2;
    float3 color;
    float _pad3;
    float2 dimensions;
    float intensity;
    bool castsShadow;
    bool enabled;
    float3 _pad;
};

//-----------------------------------------------------------------------------
// Buffers
//-----------------------------------------------------------------------------
cbuffer TransformBuffer : register(b0) { TransformMatrices Transforms; }

cbuffer LightBuffer : register(b1) {
    DirectionalLight Sun;
    PointLight PointLights[MAX_POINT_LIGHTS];
    SpotLight SpotLights[MAX_SPOT_LIGHTS];
    AreaLight AreaLights[MAX_AREA_LIGHTS];
}

cbuffer CameraBuffer : register(b2) { float4 CameraPosition; }

// Additional buffers can be bound to 3..n

//-----------------------------------------------------------------------------
// Samplers
//-----------------------------------------------------------------------------
Texture2D AlbedoMap : register(t0);
SamplerState AlbedoState : register(s0);

Texture2D MetallicMap : register(t1);
SamplerState MetallicState : register(s1);

Texture2D RoughnessMap : register(t2);
SamplerState RoughnessState : register(s2);

Texture2D AOMap : register(t3);
SamplerState AOState : register(s3);

Texture2D NormalMap : register(t4);
SamplerState NormalState : register(s4);

Texture2D ShadowZBuffer : register(t5);
SamplerComparisonState ShadowZBufferState : register(s5);
#endif