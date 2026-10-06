#include "EnginePCH.h"
#include "Render/SceneRenderer.h"
#include "Render/Renderer.h"
#include "Render/RenderCommand.h"
#include "Render/Buffer.h"
#include "Render/DeferredViewTargets.h"
#include "Engine/World.h"

#include <cmath>

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
	VisiblePointLights.Reset();

	if (!World)
	{
		return;
	}

	// 월드 프리미티브 수집 및 절두체 컬링
	World->GatherRenderPackets(RenderQueue, &CachedViewContext, &View.Frustum, Renderer);

	// 렌더 큐 자체 정렬
	RenderQueue.Sort();

	for (const FLightSceneProxy* Light : World->GetScene().GetLights()) // 월드에 있는 모든 라이트 정보 가져오기
	{
		if (!Light->IsEnabled()) continue;
		const FPointLightRenderData& Data = Light->GetRenderData();
		const float Radius = Data.PositionRadius.W;
		bool bInView = true;
		for (const FPlane& Plane : View.Frustum.Planes)
		{
			const float Distance = Plane.Normal.X * Data.PositionRadius.X +
				Plane.Normal.Y * Data.PositionRadius.Y +
				Plane.Normal.Z * Data.PositionRadius.Z + Plane.Distance;
			const float NormalLength = std::sqrt(Plane.Normal.X * Plane.Normal.X +
				Plane.Normal.Y * Plane.Normal.Y + Plane.Normal.Z * Plane.Normal.Z);
			if (Distance < -Radius * NormalLength)
			{
				bInView = false;
				break;
			}
		}
		if (!bInView) continue;
		if (VisiblePointLights.Num() >= MaxDeferredPointLights) break;
		VisiblePointLights.Add(Data);
	}
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

bool FSceneRenderer::RenderGBuffer(FRenderer* Renderer, const FDeferredViewTargets& Targets, uint32 Width, uint32 Height)
{
	if (!Renderer || !RenderCommand::BeginGBufferPass(Targets, Width, Height))
		return false;

	Renderer->RenderOpaque(View, RenderQueue, true);
	RenderCommand::EndRenderPass();
	return true;
}

void FSceneRenderer::RenderDeferredLighting(FRenderer* Renderer, const FDeferredViewTargets& Targets)
{
	if (Renderer)
		Renderer->DrawDeferredLighting(View, Targets, VisiblePointLights);
}

void FSceneRenderer::RenderToneMap(FRenderer* Renderer, const FDeferredViewTargets& Targets)
{
	if (Renderer)
		Renderer->DrawToneMap(Targets);
}

// 반투명 렌더링
void FSceneRenderer::RenderTranslucent(FRenderer* Renderer, FConstantBuffer* FogConstants)
{
	if (!Renderer)
	{
		return;
	}

	ID3D11Buffer* FogBuffer = FogConstants ? FogConstants->GetBuffer() : nullptr;
	RenderCommand::GetContext()->PSSetConstantBuffers(3, 1, &FogBuffer);
	Renderer->RenderTranslucent(View, RenderQueue);
	FogBuffer = nullptr;
	RenderCommand::GetContext()->PSSetConstantBuffers(3, 1, &FogBuffer);
}
