struct VSInput {
    float3 position : POSITION;
    float4 color : COLOR;
};

struct PSInput {
    float4 position : SV_Position;
    float4 color : COLOR;
};

PSInput VS_Main(VSInput input) {
    PSInput output;

    output.position = float4(input.position, 1.0f);
    output.color = input.color;

    return output;
}

float4 PS_Main(PSInput input) : SV_Target {
    return input.color;
}