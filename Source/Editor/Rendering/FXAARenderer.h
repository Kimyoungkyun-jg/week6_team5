#pragma once

#include "Render/Renderer.h"
#include "Render/RenderingInfo.h"
#include "Render/PipelineState.h"

// FXAA 처리를 하기 위해 필요한 값을 넘긴다.
struct alignas(16) FFXAAData
{
	// todo
};

class FFXAARenderer
{
public:
	// Shader와 상수 버퍼를 준비한다.
	void Init(FRenderer* InRenderer);
	// Color Buffer에 있는 값을 바탕으로 FXAA를 처리한다.
	bool OnRender(const FSceneView& View, FTexture2D* DepthTarget, FTexture2D* ColorTarget);

private:
	FRenderer* Renderer;

	FShaderProgram* Shader;
	FPipelineState FXAAPipelineState;

	TUniquePtr<FConstantBuffer> ConstantBuffer;
};
