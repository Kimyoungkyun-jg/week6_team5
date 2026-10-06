#pragma pack_matrix(row_major)

cbuffer Viewconstants : register(b0)
{
    matrix VP;
};

cbuffer Worldconstants : register(b2)
{
    matrix World;
};


cbuffer PerObjectConstants : register(b1)
{
    float3 Center;
    float Radius;
    float4 Color;
    float3 CameraPosition;
    float RadiusFallOff;
    float Intensity;
};

struct VS_INPUT
{
    float3 p : POSITION;
    float3 n : NORMAL;
    float4 c : COLOR;
    float2 t : TEXCOORD;
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float3 WorldPos : TEXCOORD0;
};

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;

    output.Position = mul(mul(float4(input.p, 1.0f), World), VP);
    output.WorldPos = mul(float4(input.p, 1.0f), World).xyz;
    return output;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    float3 ViewDir = normalize(CameraPosition - input.WorldPos);
    float3 Normal = normalize(input.WorldPos - Center);
    float NdotV = saturate(dot(Normal, ViewDir));
    float Distance = length(input.WorldPos - Center);
    float x = Radius > 0.0f ? Distance / Radius : 1.0f;
    float Attenuation = pow(NdotV, RadiusFallOff);
    
    return Color * Attenuation * Intensity;
}