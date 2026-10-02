#pragma once

#include "Core/Types.h"
#include "Render/SceneView.h"
#include "Render/RenderPacket.h"

class UWorld;
class FRenderer;

// 씬 렌더링 조율자
class FSceneRenderer
{
public:
	FSceneRenderer(UWorld* InWorld, const FSceneView& InView);
	~FSceneRenderer() = default;

	// 가시성 검사 및 렌더 큐 수집
	void InitViews(FRenderer* Renderer);

	// 불투명 렌더링
	void RenderOpaque(FRenderer* Renderer);

	// 반투명 렌더링
	void RenderTranslucent(FRenderer* Renderer);

	// 전체 렌더 큐 조회
	FRenderQueue& GetRenderQueue() { return RenderQueue; }
	const FRenderQueue& GetRenderQueue() const { return RenderQueue; }

	// 뷰 정보 조회
	const FSceneView& GetView() const { return View; }

private:
	UWorld* World = nullptr;
	FSceneView View;
	FRenderQueue RenderQueue;
	FViewContext CachedViewContext;
};
