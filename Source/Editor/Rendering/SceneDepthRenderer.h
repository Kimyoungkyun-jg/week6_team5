#pragma once

#include "Render/Renderer.h"
#include "Render/RenderingInfo.h"
#include "Render/PipelineState.h"

class FTexture2D;

struct FSceneDepthData
{
	float DepthA;
	float DepthB;
	float InvMaxVisualizeDist;
	float Padding;
};

class FSceneDepthRenderer
{
public:
	void Init(FRenderer* InRenderer);
	void OnRender(FTexture2D* DepthTarget, const FSceneView& SceneView, const FViewportSettings& Viewport);

private:
	FRenderer* Renderer;

	FShaderProgram* Shader;
	FPipelineState SceneDepthPipelineState;

	TUniquePtr<FConstantBuffer> ConstantBuffer;
};
