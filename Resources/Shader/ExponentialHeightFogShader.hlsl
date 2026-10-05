#pragma pack_matrix(row_major)

Texture2D SceneColorTexture : register(t0);
Texture2D SceneDepthTexture : register(t1);
SamplerState LinearSampler : register(s0);

cbuffer ExponentialHeightFogConstants : register(b0)
{
    matrix InvViewProjection;
    float3 CameraWorldPosition;
    float FogDensity;
    float4 FogInscatteringColor;
    float FogHeight;
    float FogHeightFalloff;
    float FogStartDistance;
    float FogCutoffDistance;
    float FogMaxOpacity;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
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
    // 0. 깊이 샘플링
    float DeviceZ = SceneDepthTexture.Sample(LinearSampler, Input.UV).r;
    //if (DeviceZ >= 0.9999f)
    //    return SceneColorTexture.Sample(LinearSampler, Input.UV);

    // 1. 월드 좌표 복원에 필요한 변수
    float4 ClipPosition = float4(Input.UV * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), DeviceZ, 1.0f); // UV와 DeviceZ로 만든 4차원 동차 클립 좌표 (-1~1 범위)
    float4 WorldPosition4D = mul(ClipPosition, InvViewProjection); // InvViewProjection을 곱한 동차 월드 좌표
    float3 WorldPosition = WorldPosition4D.xyz / WorldPosition4D.w; // 원근 나눗셈(/ W)을 거친 최종 픽셀의 3D 월드 좌표 (P)

    // 2. 가시선 벡터 변수
    float3 CameraToPixel = WorldPosition - CameraWorldPosition; // 카메라 위치에서 픽셀 위치로 향하는 방향 벡터 (P - C)
    float RayLength = length(CameraToPixel); // 카메라와 픽셀 사이의 총 거리 (D)
    RayLength = max(RayLength - FogStartDistance, 0.0f);
    float DeltaZ = WorldPosition.z - CameraWorldPosition.z; // 카메라와 픽셀 사이의 높이 차이 (P.z - C.z)

    // 3. 지수 높이 포그 적분 계산 변수
    float CameraFogDensity = FogDensity * exp(-FogHeightFalloff * (CameraWorldPosition.z - FogHeight)); // 카메라 높이(C.z)에서의 안개 밀도
    float HeightFalloffTerm = max(-80.0f, FogHeightFalloff * DeltaZ); // 높이 감쇄와 높이차를 곱한 경사도 항
    float LineIntegral = (1.0f - exp(-HeightFalloffTerm)) / HeightFalloffTerm;
    float LinearIntegralTaylor = 1 - HeightFalloffTerm * 0.5f;
    float FinalLineIntegral = (abs(HeightFalloffTerm) < 0.0001f) ? LinearIntegralTaylor : LineIntegral; // 높이 감쇄가 거의 없는 경우 테일러 근사 사용
    float OpticalDepth = CameraFogDensity * FinalLineIntegral * RayLength; // 최종 광학 두께

    // 4. 투과율 및 포그 팩터 변수
    float Transmittance = exp(-OpticalDepth); // 빛의 투과율
    float FogFactor = saturate(1.0f - Transmittance); // 최종 안개 블렌딩 비율 (0.0 ~ 1.0)
    FogFactor *= FogMaxOpacity; // 안개 최대 불투명도 적용
    if (FogCutoffDistance > 0.0f)
    {
        float FadeStartDistance = FogCutoffDistance * 0.85f;
        float FadeRange = FogCutoffDistance - FadeStartDistance;
        float FadeFactor = saturate(((FogCutoffDistance - RayLength)) / FadeRange);
        FogFactor *= FadeFactor;
    }

    // 5. 원본 색상 샘플링 및 최종 출력 변수
    float4 SceneColor = SceneColorTexture.Sample(LinearSampler, Input.UV); // SceneColorTexture에서 샘플링한 원래 화면 색상
    float4 FinalColor = lerp(SceneColor, FogInscatteringColor, FogFactor); // 안개 색상과 SceneColor를 lerp한 최종 색상

    return FinalColor;
}