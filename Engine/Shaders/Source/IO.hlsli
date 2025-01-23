#ifndef SHADER_IO_HLSL
#define SHADER_IO_HLSL

//-----------------------------------------------------------------------------
// Inputs
//-----------------------------------------------------------------------------
struct VSInputPos {
    float3 position : POSITION;
};

struct VSInputPosTex {
    float3 position : POSITION;
    float2 texCoord : TEXCOORD0;
};

struct VSInputPosTexNormal {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD0;
};

struct VSInputPBR {
    float3 position : POSITION; // Object space position
    float3 normal : NORMAL; // Object space normal
    float3 tangent : TANGENT; // Object space tangent
    float2 texCoord0 : TEXCOORD0; // Diffuse/normal map coordinates
    float2 texCoord1 : TEXCOORD1; // Additional texture coordinates (lightmap UVs, etc)
};

struct VSInputSkinned {
    float3 position : POSITION; // Object space position
    float3 normal : NORMAL; // Object space normal
    float3 tangent : TANGENT; // Object space tangent
    float2 texCoord : TEXCOORD0; // Texture coordinates
    uint4 boneIds : BLENDINDICES; // Bone indices for skinning
    float4 weights : BLENDWEIGHT; // Bone weights for skinning
};

//-----------------------------------------------------------------------------
// Outputs
//-----------------------------------------------------------------------------
struct VSOutputPos {
    float4 position : SV_POSITION; // Projected position
};

struct VSOutputPosColor {
    float4 position : SV_POSITION; // Projected position
    float4 color : COLOR; // Interpolated color
};

struct VSOutputPosTex {
    float4 position : SV_POSITION; // Projected position
    float2 texCoord : TEXCOORD0; // Texture coordinates
    float4 positionCS : TEXCOORD1; // Clip space position for post-process effects
};

struct VSOutputLighting {
    float4 position : SV_POSITION; // Projected position
    float2 texCoord : TEXCOORD0; // Texture coordinates
    float3 normal : NORMAL; // World space normal
    float3 worldPos : TEXCOORD1; // World space position
};

struct VSOutputPBR {
    float4 position : SV_POSITION; // Projected position
    float2 texCoord0 : TEXCOORD0; // Main texture coordinates
    float2 texCoord1 : TEXCOORD1; // Secondary texture coordinates
    float3 normal : NORMAL; // World space normal
    float3 tangent : TANGENT; // World space tangent
    float3 bitangent : BINORMAL; // World space bitangent
    float3 worldPos : TEXCOORD2; // World space position
    float4 positionCS : TEXCOORD3; // Clip space position for post-process effects
};

// For deferred pipeline
struct VSOutputGBuffer {
    float4 position : SV_POSITION; // Projected position
    float2 texCoord : TEXCOORD0; // Texture coordinates
    float3 normal : NORMAL; // World space normal
    float3 tangent : TANGENT; // World space tangent
    float3 bitangent : BINORMAL; // World space bitangent
    float3 worldPos : TEXCOORD1; // World space position
    float4 positionCS : TEXCOORD2; // Clip space position
};

//-----------------------------------------------------------------------------
// Helpers
//-----------------------------------------------------------------------------
float3 CalculateBitangent(float3 normal, float3 tangent, float handedness) {
    return cross(normal, tangent) * handedness;
}

// Transforms normal from object to world space
float3 TransformNormal(float3 normal, matrix worldMatrix) {
    return normalize(mul(normal, (float3x3) worldMatrix));
}

// Transforms a position and its derivatives (useful for normal mapping)
void TransformPosition(float3 position, matrix worldMatrix, matrix viewProjectionMatrix,
                      out float4 positionCS, out float3 positionWS) {
    positionWS = mul(float4(position, 1.0f), worldMatrix).xyz;
    positionCS = mul(float4(positionWS, 1.0f), viewProjectionMatrix);
}

#endif