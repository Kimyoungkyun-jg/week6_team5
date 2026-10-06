cbuffer FXAAConstants : register(b0)
{
    float InvWidth;
    float InvHeight;
    float2 Padding;
};

Texture2D<float4> OriginalColorBufferTexture : register(t0);
SamplerState LinearClampSampler : register(s0);

struct VSOutput
{
    float4 Position : SV_Position;
};

VSOutput mainVS(uint VertexID : SV_VertexID)
{
    float2 UV = float2((VertexID << 1) & 2, VertexID & 2);
    VSOutput Output;
    Output.Position = float4(UV * float2(2, -2) + float2(-1, 1), 0, 1);
    return Output;
}

float4 mainPS(VSOutput Input) : SV_Target
{
    float2 UV = Input.Position.xy * float2(InvWidth, InvHeight);
    return OriginalColorBufferTexture.SampleLevel(LinearClampSampler, UV, 0);
}
