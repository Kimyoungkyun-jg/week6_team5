#include "EnginePCH.h"
#include "Editor/Rendering/HeightFogRenderer.h"

void FHeightFogRenderer::Init(FRenderer* InRenderer)
{
	Renderer = InRenderer;
	// 셰이더와 상수 버퍼는 안개 계산을 구현하는 단계에서 준비한다.
}

bool FHeightFogRenderer::OnRender(const FSceneView& View, FTexture2D* DepthTarget,
	FTexture2D* ColorTarget, const FHeightFogSetting& InHeightFogSetting)
{
	// 아직 합성을 구현하지 않았으므로 GPU 상태와 기존 화면을 변경하지 않는다.
	return false;
}
