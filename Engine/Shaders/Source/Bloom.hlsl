cbuffer BloomParams : register(b0) {
    float threshold;
    float intensity;
    float screenWidth;
    float screenHeight;
};

Texture2D<float4> InputTexture : register(t0);
RWTexture2D<float4> OutputTexture : register(u0);

[numthreads(8,8,1)]
void CS_Main(uint3 DTid : SV_DispatchThreadID) {
    if (DTid.x >= screenWidth || DTid.y >= screenHeight)
        return;

    float4 color = InputTexture[DTid.xy];
    float luminance = dot(color.rgb, float3(0.2126, 0.7152, 0.0722));

    if (luminance > threshold) {
        color.rgb *= intensity;
    } else {
        color.rgb = 0;
    }

    OutputTexture[DTid.xy] = color;
}