#pragma once

#include "Render/SceneView.h"
#include "Math/Vector4.h"
#include "Render/Buffer.h"
#include "Render/PipelineState.h"

class FRenderer;
class FTexture2D;

struct alignas(16) FFireBallData
{
	FMatrix InverseViewProjection;
	FVector Center;
	float Radius;
	FVector4 Color;
	FVector CameraPosition;
	float RadiusFallOff;
	float Intensity;
	float Padding[3]; // 16바이트 정렬을 위해 패딩 추가
};

class FFireBallRenderer
{
public:
	void Init(FRenderer* InRenderer);

	// 지원하는 뷰에서 깊이를 읽어 한 층 안개를 합성한다.
	bool OnRender(const FSceneView& View, FTexture2D* DepthTarget, FTexture2D* ColorTarget, const FFireBallData& InFireBallData);

private:
	FRenderer* Renderer = nullptr;
	FShaderProgram* Shader = nullptr;
	FPipelineState PipelineState;
	TUniquePtr<FConstantBuffer> ConstantBuffer;
};
