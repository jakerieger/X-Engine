#include "Common.hlsli"
#include "IO.hlsli"

cbuffer WaterParameters : register(b3) {
    float Time;
    float WaterDepth;
    float Transparency;
    float2 WindDirection;
    float WindStrength;
}

cbuffer WaveParameters : register(b4) {
    float4 WaveAmplitudes[2];
    float4 WaveFrequencies[2];
    float4 WaveSpeeds[2];
    float4 WaveDirectionX[2];
    float4 WaveDirectionZ[2];
    float4 WavePhases[2];
    int ActiveWaveCount;
}

struct VSOuputWater {
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 WorldPosition : TEXCOORD1;
    float3 OriginalPosition : TEXCOORD2;  // Pre-displacement position for calculations
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
    float3 ViewDirection : TEXCOORD3;
    float WaterHeight : TEXCOORD4;
};

float CalculateWaveHeight(float3 position, out float3 normal, out float3 tangent) {
    // Initialize outputs
    normal = float3(0, 1, 0);
    tangent = float3(1, 0, 0);
    
    // Total height accumulator
    float totalHeight = 0.0;
    
    // Normal delta accumulator
    float3 normalDelta = float3(0, 0, 0);
    
    if (true) {
        float amplitude = 0.5;
        float frequency = 0.2;
        float speed = 1.0;
        float2 direction = float2(1.0, 0.0);
        
        float projection = dot(position.xz, direction);
        float wavePhase = frequency * projection + speed * Time;
        
        float waveHeight = amplitude * sin(wavePhase);
        totalHeight += waveHeight;
        
        normalDelta.x -= amplitude * frequency * direction.x * cos(wavePhase);
        normalDelta.z -= amplitude * frequency * direction.y * cos(wavePhase);
    }
    
    int wavesToUse = min(max(0, ActiveWaveCount), 8);
    
    for (int i = 0; i < wavesToUse; i++) {
        // Get parameters with safety checks
        float amplitude = 0.0;
        float frequency = 0.0;
        float speed = 0.0;
        float dirX = 0.0;
        float dirZ = 0.0;
        float phase = 0.0;
        
        if (i < 4) {
            amplitude = WaveAmplitudes[0][i];
            frequency = WaveFrequencies[0][i];
            speed = WaveSpeeds[0][i];
            dirX = WaveDirectionX[0][i];
            dirZ = WaveDirectionZ[0][i];
            phase = WavePhases[0][i];
        } else {
            amplitude = WaveAmplitudes[1][i-4];
            frequency = WaveFrequencies[1][i-4];
            speed = WaveSpeeds[1][i-4];
            dirX = WaveDirectionX[1][i-4];
            dirZ = WaveDirectionZ[1][i-4];
            phase = WavePhases[1][i-4];
        }
        
        // Skip waves with zero amplitude or invalid directions
        float dirLength = length(float2(dirX, dirZ));
        if (amplitude <= 0.001 || dirLength <= 0.001)
            continue;
            
        // Normalize direction vector
        float2 direction = float2(dirX, dirZ) / dirLength;
        
        // Calculate wave
        float projection = dot(position.xz, direction);
        float wavePhase = frequency * projection + speed * Time + phase;
        
        float waveHeight = amplitude * sin(wavePhase);
        totalHeight += waveHeight;
        
        normalDelta.x -= amplitude * frequency * direction.x * cos(wavePhase);
        normalDelta.z -= amplitude * frequency * direction.y * cos(wavePhase);
    }
    
    // Finalize normal
    normal = normalize(float3(normalDelta.x, 1.0, normalDelta.z));
    
    // Tangent calculation
    tangent = normalize(float3(1.0, 0.0, 0.0)); // Default
    if (wavesToUse > 0) {
        float2 primaryDir = normalize(float2(WaveDirectionX[0][0], WaveDirectionZ[0][0]));
        if (length(primaryDir) > 0.001) {
            tangent = normalize(float3(-primaryDir.y, 0, primaryDir.x));
        }
    }
    
    return totalHeight;
}

VSOuputWater VS_Main(VSInputPBR input) {
    VSOuputWater output;

    output.OriginalPosition = input.position;

    float3 normal = float3(0, 1, 0);
    float3 tangent = float3(1, 0, 0);
    float waveHeight = CalculateWaveHeight(input.position, normal, tangent);

    float3 displacedPosition = input.position;
    displacedPosition.y += waveHeight;

    output.WaterHeight = waveHeight;

    float4 worldPosition = mul(float4(displacedPosition, 1.0f),Transforms.model);
    output.WorldPosition = worldPosition.xyz;

    float3 binormal = cross(normal, tangent);

    output.Normal = normalize(mul(normal, (float3x3)Transforms.model));
    output.Tangent = normalize(mul(tangent, (float3x3)Transforms.model));
    output.Binormal = normalize(mul(binormal, (float3x3)Transforms.model));

    output.ViewDirection = normalize(CameraPosition.xyz - worldPosition.xyz);
    output.Position = mul(worldPosition, mul(Transforms.view, Transforms.projection));
    output.TexCoord = input.texCoord0;

    return output;
}

float CalculateFresnelFactor(float3 normal, float3 viewDirection, float R0) {
    float fresnelFactor = R0 + (1.0f - R0) * pow(1.0f - saturate(dot(normal, viewDirection)), 5);
    return fresnelFactor;
}

float4 PS_Main(VSOuputWater input) : SV_Target {
    // Water colors
    float3 shallowColor = float3(0.1f, 0.5f, 0.8f);
    float3 deepColor = float3(0.0f, 0.2f, 0.4f);
    
    // Use the normal for lighting
    float3 worldNormal = normalize(input.Normal);
    
    // Get view direction from camera to surface
    float3 viewDirection = normalize(CameraPosition.xyz - input.WorldPosition);
    
    // Calculate view-dependent color (water looks darker when viewed from above)
    float viewDependentFactor = pow(max(dot(worldNormal, viewDirection), 0.0), 0.5);
    float3 waterColor = lerp(deepColor, shallowColor, viewDependentFactor);
    
    // Add simple sun lighting
    float sunFactor = max(dot(worldNormal, -normalize(Sun.direction.xyz)), 0.0);
    float3 diffuse = Sun.color.rgb * sunFactor * Sun.intensity;
    
    // Add simple specular highlight
    float3 halfVector = normalize(-normalize(Sun.direction.xyz) + viewDirection);
    float specular = pow(max(dot(worldNormal, halfVector), 0.0), 64.0) * 0.5;
    
    // Calculate simple "fake" fresnel effect for reflectivity
    float fresnel = pow(1.0 - max(dot(worldNormal, viewDirection), 0.0), 5.0);
    
    // Final color combines water color with lighting and reflectivity
    float3 finalColor = waterColor * (0.2 + 0.8 * diffuse) + specular * Sun.color.rgb;
    
    // Add blue-sky reflection at glancing angles using fresnel
    finalColor = lerp(finalColor, float3(0.3, 0.5, 0.9), fresnel * 0.7);
    
    return float4(finalColor, 0.67);
}