#ifndef GRAPHICS_COMMON_HLSL
#define GRAPHICS_COMMON_HLSL

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
static const uint MAX_POINT_LIGHTS = 16;
static const uint MAX_SPOT_LIGHTS  = 16;
static const uint MAX_AREA_LIGHTS  = 16;

static const float PI              = 3.141592653589793;
static const float EPSILON         = 1e-6;

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
};

struct PointLight {
    float3 position;  // Takes 12 bytes, needs 4-byte padding
    float padding1;   // Add 4 bytes to align next float3

    float3 color;     // Takes 12 bytes, needs 4-byte padding
    float intensity;  // We can use this as natural padding

    float constant;  // These scalars can be packed together
    float lin;
    float quadratic;
    float radius;  // All floats align naturally

    bool castsShadow;  // Bools in HLSL typically use 4 bytes
    bool enabled;      // Need 2 bytes padding to maintain alignment
    float2 padding2;   // Add explicit padding to reach 16-byte alignment
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
    float3 direction;
    float3 color;
    float2 dimensions;
    float intensity;
    bool castsShadow;
    bool enabled;
    float2 _pad;
};

struct PBRMaterial {
    float3 albedo;
    float metallic;
    float roughness;
    float ao;
    float3 emissive;
    float emissiveStrength;
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

cbuffer PBRMaterialBuffer : register(b2) { PBRMaterial Material; }

cbuffer CameraBuffer : register(b3) { float4 CameraPosition; }

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

//-----------------------------------------------------------------------------
// Helper Functions
//-----------------------------------------------------------------------------
float CalculateAttenuation(float distance, float constant, float lin, float quadratic) {
    return 1.0f / (constant + lin * distance + quadratic * distance * distance);
}

float CalculateSpotIntensity(float3 lightDir, float3 spotDir, float innerCos, float outerCos) {
    float cosAngle = dot(normalize(-lightDir), normalize(spotDir));
    return smoothstep(outerCos, innerCos, cosAngle);
}

float3 FresnelSchlick(float cosTheta, float3 F0) { return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0); }

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness) {
    float r1 = 1.0 - roughness;
    return F0 + (max(float3(r1, r1, r1), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

// Distribution Function - GGX/Trowbridge-Reitz
float DistributionGGX(float3 N, float3 H, float roughness) {
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom    = a2;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    denom        = PI * denom * denom;

    return nom / max(denom, EPSILON);
}

// Geometry Function - Smith's method with Schlick-GGX
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

// Combined Geometry term
float GeometrySmith(float3 N, float3 V, float3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1  = GeometrySchlickGGX(NdotV, roughness);
    float ggx2  = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

// Calculate normal distribution for specular BRDF
float3 SpecularBRDF(float3 N, float3 V, float3 L, float roughness, float3 F0) {
    float3 H = normalize(V + L);

    // Calculate all BRDF terms
    float D  = DistributionGGX(N, H, roughness);
    float G  = GeometrySmith(N, V, L, roughness);
    float3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    // Combine terms
    return (F * G * D) / max(4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0), EPSILON);
}

// Energy conservation helper
float3 EnergyConservation(float3 specular, float metallic, float3 albedo) {
    return (1.0 - specular) * (1.0 - metallic) * albedo;
}

// Calculate point light contribution with PBR
float3 CalculatePointLightPBR(PointLight light, float3 worldPos, float3 normal, float3 viewDir, PBRMaterial material) {
    float3 lightDir = light.position - worldPos;
    float distance  = length(lightDir);
    lightDir        = normalize(lightDir);

    // Early out if beyond radius
    if (distance > light.radius) return float3(0, 0, 0);

    // Calculate attenuation
    float attenuation = CalculateAttenuation(distance, light.constant, light.lin, light.quadratic);

    // Calculate base reflectivity (F0)
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), material.albedo, material.metallic);

    // Calculate specular BRDF
    float3 specular = SpecularBRDF(normal, viewDir, lightDir, material.roughness, F0);

    // Calculate diffuse BRDF
    float3 diffuse = EnergyConservation(specular, material.metallic, material.albedo);

    // Combine everything
    float NdotL     = max(dot(normal, lightDir), 0.0);
    float3 radiance = light.color * light.intensity * attenuation;

    return (diffuse / PI + specular) * radiance * NdotL;
}

// Area light helper functions
float3 ClosestPointRect(float3 pos, float3 rectPos, float3 rectRight, float3 rectUp, float2 dimensions) {
    float3 rectCenter = rectPos;
    float3 toPoint    = pos - rectCenter;

    // Project point onto rectangle's plane
    float2 projected;
    projected.x = dot(toPoint, rectRight);
    projected.y = dot(toPoint, rectUp);

    // Clamp to rectangle bounds
    float2 halfDim = dimensions * 0.5;
    projected      = clamp(projected, -halfDim, halfDim);

    // Calculate world space point
    return rectCenter + rectRight * projected.x + rectUp * projected.y;
}

// Calculate area light contribution
float3 CalculateAreaLightPBR(AreaLight light, float3 worldPos, float3 normal, float3 viewDir, PBRMaterial material) {
    // Calculate basis vectors for the area light
    float3 right = normalize(cross(light.direction, float3(0, 1, 0)));
    float3 up    = normalize(cross(right, light.direction));

    // Find closest point on area light
    float3 closestPoint = ClosestPointRect(worldPos, light.position, right, up, light.dimensions);

    float3 lightDir     = normalize(closestPoint - worldPos);
    float distance      = length(closestPoint - worldPos);

    // Calculate area light influence
    float area        = light.dimensions.x * light.dimensions.y;
    float attenuation = 1.0 / (distance * distance);

    // Rest of PBR calculation similar to point light...
    float3 F0       = lerp(float3(0.04, 0.04, 0.04), material.albedo, material.metallic);
    float3 specular = SpecularBRDF(normal, viewDir, lightDir, material.roughness, F0);
    float3 diffuse  = EnergyConservation(specular, material.metallic, material.albedo);

    float NdotL     = max(dot(normal, lightDir), 0.0);
    float3 radiance = light.color * light.intensity * attenuation * area;

    return (diffuse / PI + specular) * radiance * NdotL;
}

float3 CalculateIBLDiffuse(float3 N, float3 albedo, float ao) {
    // This would normally sample from an irradiance cubemap
    // Simplified version shown here
    float3 irradiance = float3(0.03, 0.03, 0.03);  // Ambient light level
    return irradiance * albedo * ao;
}

float3 CalculateIBLSpecular(float3 N, float3 V, float roughness, float3 F0) {
    float3 R = reflect(-V, N);

    // This would normally sample from a prefiltered environment cubemap
    // and use a BRDF LUT texture
    // Simplified version shown here
    float3 prefilteredColor = float3(0.01, 0.01, 0.01);
    float2 envBRDF          = float2(1.0, 0.0);

    return prefilteredColor * (F0 * envBRDF.x + envBRDF.y);
}

float3 TonemapACES(float3 color) {
    float3 x = color * 0.6f;
    float a  = 2.51f;
    float b  = 0.03f;
    float c  = 2.43f;
    float d  = 0.59f;
    float e  = 0.14f;
    color    = (x * (a * x + b)) / (x * (c * x + d) + e);

    // Gamma correction
    color = pow(saturate(color), float3(1.0f / 2.2f, 1.0f / 2.2f, 1.0f / 2.2f));

    return color;
}
#endif