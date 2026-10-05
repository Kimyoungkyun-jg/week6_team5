#pragma once

#include "Render/Renderer.h"
#include "Render/RenderingInfo.h"
#include "Render/PipelineState.h"
#include "Component/ExponentialHeightFogComponent.h"

class FTexture2D;

struct FHeightFogData
{
	FMatrix InvViewProjection;
	FVector CameraWorldPosition;
	float FogDensity;
	FVector4 FogInscatteringColor;
	float FogHeight;
	float FogHeightFalloff;
	float FogStartDistance;
	float FogCutoffDistance;
	float FogMaxOpacity;
	float Padding[3];
};

class FHeightFogRenderer
{
public:
	void Init(FRenderer* InRenderer);
	void OnRender(FTexture2D* ColorTarget, FTexture2D* DepthTarget, const FSceneView& SceneView, const FViewportSettings& Viewport, UExponentialHeightFogComponent* FogComponent);

private:
	FRenderer* Renderer;

	FShaderProgram* Shader;
	FPipelineState HeightFogPipelineState;

	TUniquePtr<FConstantBuffer> ConstantBuffer;
	TUniquePtr<FTexture2D> TempColorTarget;
};
