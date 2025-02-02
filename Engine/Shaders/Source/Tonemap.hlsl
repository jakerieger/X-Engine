Texture2D<float4> InputTexture : register(t0);
RWTexture2D<float4> OutputTexture : register(u0);

[numthreads(8,8,1)]
void CS_Main(uint3 DTid : SV_DispatchThreadID) {
    // if (DTid.x >= screenWidth || DTid.y >= screenHeight)
    //     return;

    float3 color = InputTexture[DTid.xy].rgb;
    float3 x = color * 0.6f;
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    color = (x * (a * x + b)) / (x * (c * x + d) + e);
    
    // Gamma correction
    color = pow(saturate(color), float3(1.0f / 2.2f, 1.0f / 2.2f, 1.0f / 2.2f));

    OutputTexture[DTid.xy] = float4(color, 1.0f);
}