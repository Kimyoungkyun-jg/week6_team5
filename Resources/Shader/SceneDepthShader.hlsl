#pragma pack_matrix(row_major)

cbuffer DepthConstants : register(b0)
{
    float DepthA;
    float DepthB;
    float InvMaxVisualizeDist;
    float Padding;
};

Texture2D SceneDepthTexture : register(t0);
SamplerState PointSampler : register(s0);

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float2 UV       : TEXCOORD0;
};

VS_OUTPUT mainVS(uint VertexID : SV_VertexID)
{
    VS_OUTPUT Output;
    
    Output.UV = float2((VertexID << 1) & 2, VertexID & 2);
    
    Output.Position = float4(Output.UV * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);

    return Output;
}

float4 mainPS(VS_OUTPUT Input) : SV_TARGET
{
    float DeviceZ = SceneDepthTexture.Sample(PointSampler, Input.UV).r;

    if (DeviceZ >= 0.999999f)
    {
        return float4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    
    float LinearDepth = rcp(DepthA * DeviceZ + DepthB);

    float GrayScale = saturate(LinearDepth * InvMaxVisualizeDist);
    
    return float4(GrayScale, GrayScale, GrayScale, 1.0f);
}