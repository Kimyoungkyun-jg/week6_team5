#pragma pack_matrix(row_major)

Texture2D<float4> BaseColorMetallic : register(t0);
Texture2D<float4> NormalRoughness : register(t1);
Texture2D<float> SceneDepth : register(t2);

struct FPointLightData
{
    float4 PositionRadius;
    float4 ColorIntensity;
};

cbuffer DeferredLightConstants : register(b0)
{
    matrix InverseViewProjection;
    float4 ViewportLightCount;
    FPointLightData PointLights[32];
};

struct FPSInput
{
    float4 Position : SV_POSITION;
};

FPSInput mainVS(uint VertexID : SV_VertexID)
{
    FPSInput Output;
    float2 UV = float2((VertexID << 1) & 2, VertexID & 2);
    Output.Position = float4(UV * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    return Output;
}

float4 mainPS(FPSInput Input) : SV_Target
{
    int2 Pixel = int2(Input.Position.xy);
    float Depth = SceneDepth.Load(int3(Pixel, 0));
    if (Depth >= 1.0f)
        discard; // 배경은 먼저 그린 스카이박스를 유지한다.

    float3 Albedo = BaseColorMetallic.Load(int3(Pixel, 0)).rgb;
    float3 N = normalize(NormalRoughness.Load(int3(Pixel, 0)).xyz);

    // 첫 드로우는 기본 조명을 HDR에 쓰고, 두 번째 드로우는 포인트 라이트만 더한다.
    if (ViewportLightCount.w < 0.5f)
    {
        const float3 LightDir = normalize(float3(0.5f, 0.5f, -1.0f));
        const float3 AmbientColor = float3(0.5f, 0.5f, 0.5f);
        const float3 LightColor = float3(0.5f, 0.5f, 0.5f);
        float NdotL = saturate(dot(N, -LightDir));
        return float4(Albedo * (AmbientColor + LightColor * NdotL), 1.0f);
    }

    float2 UV = (float2(Pixel) + 0.5f) / ViewportLightCount.xy;
    float2 NDC = float2(UV.x * 2.0f - 1.0f, 1.0f - UV.y * 2.0f);
    float4 WorldH = mul(float4(NDC, Depth, 1.0f), InverseViewProjection);
    float3 WorldPosition = WorldH.xyz / WorldH.w;

    float3 PointLighting = 0.0f;

    [loop]
    for (uint Index = 0; Index < (uint)ViewportLightCount.z; ++Index)
    {
        FPointLightData Light = PointLights[Index];
        float3 ToLight = Light.PositionRadius.xyz - WorldPosition;
        float DistanceSquared = dot(ToLight, ToLight);
        float RadiusSquared = Light.PositionRadius.w * Light.PositionRadius.w;
        if (DistanceSquared >= RadiusSquared) continue;

        float Distance = sqrt(max(DistanceSquared, 1.0e-6f));
        float3 L = ToLight / Distance;
        float RadiusFade = saturate(1.0f - DistanceSquared / RadiusSquared);
        float Attenuation = RadiusFade * RadiusFade / (1.0f + DistanceSquared);
        PointLighting += Light.ColorIntensity.rgb * Light.ColorIntensity.a *
            saturate(dot(N, L)) * Attenuation;
    }

    return float4(Albedo * PointLighting, 0.0f);
}
