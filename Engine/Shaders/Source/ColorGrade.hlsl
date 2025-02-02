cbuffer ColorGradeParams : register(b0) {
    float saturation;
    float contrast;
    float temperature;
    float exposureAdjustment;
}

Texture2D InputTexture : register(t0);
RWTexture2D<float4> OutputTexture : register(u0);

static const float3 RGB_6500K = float3(1.0, 0.9687, 0.9228);

float3 KelvinToRGB(float temperature) {
    temperature = clamp(temperature, 1000.0, 40000.0);
    temperature /= 100.0;

    float3 color;

    // Red adjustment
    if (temperature <= 66.0) {
        color.r = 1.0;
    } else {
        float t = temperature - 60.0;
        color.r = 1.29293618606274509804 * pow(t, -0.1332047592);
    }

    // Green adjustment
    if (temperature <= 66.0) {
        color.g = 0.39008157876901960784 * log(temperature) - 0.63184144378862745098;
    } else {
        float t = temperature - 60.0;
        color.g = 1.12989086089529411765 * pow(t, -0.0755148492);
    }

    // Blue adjustment
    if (temperature <= 19.0) {
        color.b = 0.0;
    } else if (temperature <= 66.0) {
        color.b = 0.54320678911019607843 * log(temperature - 10.0) - 1.19625408914;
    } else {
        color.b = 1.0;
    }

    return max(color / RGB_6500K, 0.0001);  // Prevent division by zero in HDR
}

[numthreads(8, 8, 1)] void CS_Main(uint3 DTid : SV_DispatchThreadID) {
    uint width, height;
    OutputTexture.GetDimensions(width, height);
    if (DTid.x >= width || DTid.y >= height) return;

    float4 color      = InputTexture[DTid.xy];

    float3 tempAdjust = KelvinToRGB(temperature);
    color.rgb *= tempAdjust;

    float luminance        = dot(color.rgb, float3(0.2126, 0.7152, 0.0722));
    color.rgb              = lerp(luminance.xxx, color.rgb, saturation);

    OutputTexture[DTid.xy] = color;
}