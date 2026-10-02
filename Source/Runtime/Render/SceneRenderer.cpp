#include "EnginePCH.h"
#include "Render/SceneRenderer.h"
#include "Render/Renderer.h"
#include "Engine/World.h"

// 생성자
FSceneRenderer::FSceneRenderer(UWorld* InWorld, const FSceneView& InView)
	: World(InWorld), View(InView)
{
	CachedViewContext = View.ToViewContext();
}

// 가시성 검사 및 렌더 큐 수집
void FSceneRenderer::InitViews(FRenderer* Renderer)
{
	RenderQueue.Reset();

	if (!World)
	{
		return;
	}

	// 월드 프리미티브 수집 및 절두체 컬링
	World->GatherRenderPackets(RenderQueue, &CachedViewContext, &View.Frustum, Renderer);

	// 렌더 큐 자체 정렬
	RenderQueue.Sort();
}

// 불투명 렌더링
void FSceneRenderer::RenderOpaque(FRenderer* Renderer)
{
	if (!Renderer)
	{
		return;
	}

	Renderer->RenderOpaque(View, RenderQueue);
}

// 반투명 렌더링
void FSceneRenderer::RenderTranslucent(FRenderer* Renderer)
{
	if (!Renderer)
	{
		return;
	}

	Renderer->RenderTranslucent(View, RenderQueue);
}
