#include "EnginePCH.h"
#include "Editor/Rendering/FXAARenderer.h"
#include "Render/RenderCommand.h"
#include "Render/RenderResourceManager.h"

// Shader와 상수 버퍼를 준비한다.
void FFXAARenderer::Init(FRenderer* InRenderer)
{
	return;

	// todo
	Renderer = InRenderer;
	Shader = FRenderResourceManager::GetShaderProgram("Resources/Shader/FXAAShader.hlsl");

	// depth buffer를 사용하지 않고, 별도의 shader를 통해 렌더링을 처리한다.
	FXAAPipelineState.Shader = Shader;
	FXAAPipelineState.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	FXAAPipelineState.RasterizerState = ERasterizerState::SolidNone;
	FXAAPipelineState.BlendState = EBlendState::AlphaBlend;
	FXAAPipelineState.DepthStencilState = EDepthStencilState::Disabled;

	ConstantBuffer = RenderCommand::CreateConstantBuffer(sizeof(FFXAAData));
}

// Color Buffer에 있는 값을 바탕으로 FXAA를 처리한다.
bool FFXAARenderer::OnRender(const FSceneView& View, FTexture2D* DepthTarget, FTexture2D* ColorTarget)
{
	return false;

	// todo
	FFXAAData Data{};

	RenderCommand::BindPipelineState(FXAAPipelineState);
	RenderCommand::SetRenderTargets(ColorTarget, nullptr);
	RenderCommand::UpdateBufferData(ConstantBuffer.get(), &Data, sizeof(Data));
	RenderCommand::BindConstantBuffer(0, ConstantBuffer.get(), EShaderBindFlagBits::Pixel);
	RenderCommand::BindShaderResource(0, DepthTarget, EShaderBindFlagBits::Pixel);
	RenderCommand::Draw(6);
	RenderCommand::BindShaderResource(0, static_cast<FTexture2D*>(nullptr), EShaderBindFlagBits::Pixel);
	RenderCommand::SetRenderTargets(ColorTarget, DepthTarget);
	return true;
}