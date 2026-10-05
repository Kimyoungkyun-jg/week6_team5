#pragma once

#include "Render/SceneView.h"
#include "Math/Vector4.h"

class FRenderer;
class FTexture2D;

// 컴포넌트에서 복사한 CPU 설정값. GPU 상수 버퍼 배치는 셰이더 구현 단계에서 정한다.
struct FHeightFogSetting
{
	float FogDensity = 0.0f;
	float FogHeightFalloff = 0.0f;
	float FogHeight = 0.0f;
	FVector4 FogColor = FVector4(0.0f, 0.0f, 0.0f, 1.0f);
	float StartDistance = 0.0f;
	float FogMaxOpacity = 1.0f;
	float FogCutoffDistance = 0.0f;
};

class FHeightFogRenderer
{
public:
	void Init(FRenderer* InRenderer);

	// 현재는 호출 흐름만 연결하며 실제 안개 합성이 없으면 false를 반환한다.
	bool OnRender(const FSceneView& View, FTexture2D* DepthTarget,
		FTexture2D* ColorTarget, const FHeightFogSetting& InHeightFogSetting);

private:
	FRenderer* Renderer = nullptr;
};
