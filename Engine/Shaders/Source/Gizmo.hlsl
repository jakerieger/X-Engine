cbuffer GizmoConstants : register(b0) {
    matrix WorldViewProj;
    float3 CameraPosition;
    float Padding;
    float4 AxisColor;
    float3 AxisDirection;
    float IsSelected;
}

struct VS_INPUT {
    float3 Position : POSITION;
    float2 TexCoord : TEXCOORD;
    float3 Normal : NORMAL;
};

struct VS_OUTPUT {
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
    float3 Normal : NORMAL;
    float4 Color : COLOR;
};