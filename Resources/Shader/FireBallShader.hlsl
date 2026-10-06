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

cbuffer TranslucentFogConstants : register(b3)
{
    float4x4 FogInverseViewProjection;
    float4 FogCameraPosition;
    float4 FogColor;
    float4 FogDensityHeight;
    float4 FogDistanceViewport;
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

float FogTransmittance(float3 Position)
{
    if (FogDensityHeight.x <= 0.0f || FogDistanceViewport.x <= 0.0f)
        return 1.0f;

    float3 Ray = Position - FogCameraPosition.xyz;
    float Distance = length(Ray);
    float Start = FogDensityHeight.w;
    if (Distance <= Start ||
        (FogDistanceViewport.y > 0.0f && Distance > FogDistanceViewport.y))
        return 1.0f;

    float Length = Distance - Start;
    float StartHeight = FogCameraPosition.z + Ray.z * (Start / Distance);
    float Falloff = FogDensityHeight.y;
    float A = abs(Falloff * (Position.z - StartHeight));
    float HeightExponent = -Falloff *
        (min(StartHeight, Position.z) - FogDensityHeight.z);
    float G;
    if (A < 0.01f)
        G = 1.0f - A * 0.5f + A * A / 6.0f;
    else
        G = (1.0f - exp(-A)) / A;
    float LogTau = log(FogDensityHeight.x) + log(Length) +
        HeightExponent + log(G);
    if (!isfinite(LogTau))
        return 1.0f;

    float T = LogTau >= 4.382026635f ? 0.0f
            : LogTau <= -80.0f ? 1.0f
            : exp(-exp(LogTau));
    return max(saturate(T), 1.0f - FogDistanceViewport.x);
}

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
    float Attenuation = pow(NdotV, RadiusFallOff);
    float FogVisibility = FogTransmittance(input.WorldPos);
    return Color * (Attenuation * Intensity * FogVisibility);
}
