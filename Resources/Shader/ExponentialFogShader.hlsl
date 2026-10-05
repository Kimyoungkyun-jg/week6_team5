#pragma pack_matrix(row_major)

Texture2D ColorTarget : register(t0);
Texture2D DepthTarget : register(t1);

cbuffer constants : register(b0)
{
    float FogDensity;
    float FogHeightFalloff;
    float StartDistance;
    float FogCutoffDistance;
    float FogMaxOpacity;
    float3 CameraPosition;
    float4 FogInscatteringColor;
    matrix InverseViewProjection;
    float3 Component;
    float pad;
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
    float2 NDC = Input.UV * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f);
    float4 WorldPosition = mul(float4(NDC, depth, 1.0f), InverseViewProjection);
    WorldPosition /= WorldPosition.w;
    float Length = length(WorldPosition.xyz - CameraPosition);
    if ((FogCutoffDistance > 0 && FogCutoffDistance < Length) || Length<=StartDistance)
    {
        return ColorTarget.Load(int3(coords, 0));
    }
    float3 Direction = normalize(WorldPosition.xyz - CameraPosition);
    float3 StartFogPosition = CameraPosition + Direction * StartDistance;

    float FogDense = FogDensity * exp(-FogHeightFalloff * (StartFogPosition.z - Component.z));
    float kh = (FogHeightFalloff * (WorldPosition.z - StartFogPosition.z));
    float ratio;
    if (abs(kh) < 0.000001f)
    {
        ratio = 1;
    }
    else  ratio = ((1 - exp(-kh)) / kh); 
    float FogAmount = FogDense * (Length - StartDistance) * ratio;
    float Transmitrate = exp(-FogAmount);
    float4 FogColor = FogInscatteringColor;
    float4 Color = ColorTarget.Load(int3(coords, 0)) * (1 - min(1 - Transmitrate, FogMaxOpacity)) + FogColor * min(1 - Transmitrate, FogMaxOpacity);
    return Color;
}
