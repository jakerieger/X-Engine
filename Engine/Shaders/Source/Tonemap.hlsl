cbuffer TonemapParams : register(b0) {
    float exposure;
    uint op; // 0: ACES, 1: Reinhard, 2: Filmic, 3: Linear
    float2 _pad;
}

Texture2D<float4> InputTexture : register(t0);
RWTexture2D<float4> OutputTexture : register(u0);

float4 TonemapACES(float3 color) {
    float3 x = color * 0.6f;
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    color = (x * (a * x + b)) / (x * (c * x + d) + e);
    color = pow(saturate(color), float3(1.0f / 2.2f, 1.0f / 2.2f, 1.0f / 2.2f));

    return float4(color, 1.0f);
}

float4 TonemapReinhard(float3 color) {
    const float WHITE_POINT = 4.0f;
    color *= (1.0f + (color / (WHITE_POINT * WHITE_POINT)));
    color /= (1.0f + color);
    color = pow(saturate(color), float3(1.0f / 2.2f, 1.0f / 2.2f, 1.0f / 2.2f));
    
    return float4(color, 1.0f);
}

// Based on John Hable's Uncharted 2 tonemapping operator
float4 TonemapFilmic(float3 color) {
    const float A = 0.15f; // Shoulder strength
    const float B = 0.50f; // Linear strength
    const float C = 0.10f; // Linear angle
    const float D = 0.20f; // Toe strength
    const float E = 0.02f; // Toe numerator
    const float F = 0.30f; // Toe denominator
    const float W = 11.2f; // Linear white point value

    float3 curr = ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
    float3 whiteScale = ((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F;
    
    color = curr / whiteScale;
    color = pow(saturate(color), float3(1.0f / 2.2f, 1.0f / 2.2f, 1.0f / 2.2f));
    
    return float4(color, 1.0f);
}

float4 TonemapLinear(float3 color) {
    color = pow(saturate(color), float3(1.0f / 2.2f, 1.0f / 2.2f, 1.0f / 2.2f));
    return float4(color, 1.0f);
}

[numthreads(8,8,1)]
void CS_Main(uint3 DTid : SV_DispatchThreadID) {
    float3 color = InputTexture[DTid.xy].rgb * exposure;
    float4 finalColor = float4(1.0f, 0.0f, 0.0f, 1.0f); // default is red to signal error

    if (op == 0) {
        finalColor = TonemapACES(color);
    } else if (op == 1) {
        finalColor = TonemapReinhard(color);
    } else if (op == 2) {
        finalColor = TonemapReinhard(color);
    } else if (op == 3) {
        finalColor = TonemapLinear(color);
    }

    OutputTexture[DTid.xy] = finalColor;
}