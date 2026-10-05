Texture2D ColorTarget : register(t0);
Texture2D DepthTarget : register(t1);

cbuffer constants : register(b0)
{
    float FogDensity;
    float FogHeightFalloff;
    float StartDistance;
    float FogCutoffDistance;
    float FogMaxOpacity;
    float Pad[3]
    {
    };

    float4 FogInscatteringColor;
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
    return float4(depth.x, depth.x, depth.x, 1.0f);
}
