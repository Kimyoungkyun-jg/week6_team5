Texture2D DepthTarget : register(t0);

cbuffer DepthDisplayConstants : register(b0)
{
    float MinDepth;
    float MaxDepth;
    float2 Padding;
};

struct VSInput
{
    float3 Position : POSITION;
    float2 UV : TEXCOORD0;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
};

PSInput mainVS(VSInput Input)
{
    PSInput output;
    output.Position = float4(Input.Position, 1.0f);
    output.UV = Input.UV;
    return output;
}

float4 mainPS(PSInput Input) : SV_TARGET
{
    int2 coords = int2(Input.Position.xy);
    float depth = DepthTarget.Load(int3(coords, 0)).r;
    // Display raw depth; the selected range only adjusts grayscale contrast.
    float gray = saturate((depth - MinDepth) / max(MaxDepth - MinDepth, 0.000001f));
    return float4(gray, gray, gray, 1.0f);
}
