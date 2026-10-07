#include "EnginePCH.h"
#include "Editor/Rendering/FireballRenderer.h"
#include "Render/RenderCommand.h"
#include "Render/RenderResourceManager.h"

// Shader와 상수 버퍼를 준비한다.
void FFireBallRenderer::Init(FRenderer* InRenderer)
{
	Renderer = InRenderer;
	Shader = FRenderResourceManager::GetShaderProgram("Resources/Shader/FireBallShader.hlsl");

	PipelineState.Shader = Shader;
	PipelineState.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	PipelineState.RasterizerState = ERasterizerState::SolidNone;
	PipelineState.BlendState = EBlendState::Additive;
	PipelineState.DepthStencilState = EDepthStencilState::Disabled;

	ConstantBuffer = RenderCommand::CreateConstantBuffer(sizeof(FFireBallData));
}

bool FFireBallRenderer::OnRender(const FSceneView& View, FTexture2D* DepthTarget, FTexture2D* ColorTarget, const FFireBallData& InFireBallData)
{
	if (!DepthTarget || !ColorTarget || !Shader || !Shader->VertexShader ||
		!Shader->PixelShader || !ConstantBuffer)
		return false;

	const uint32 Width = ColorTarget->GetWidth();
	const uint32 Height = ColorTarget->GetHeight();
	if (Width == 0 || Height == 0 || DepthTarget->GetWidth() != Width ||
		DepthTarget->GetHeight() != Height)
		return false;

	FFireBallData Data = InFireBallData;
	Data.InverseViewProjection = View.ViewProjectionMatrix.Inverse();
	Data.CameraPosition = View.ViewLocation;
	Data.Center = InFireBallData.Center;
	Data.Color = InFireBallData.Color;
	Data.Intensity = InFireBallData.Intensity;
	Data.Radius = InFireBallData.Radius;
	Data.RadiusFallOff = InFireBallData.RadiusFallOff;

	RenderCommand::BindPipelineState(PipelineState);
	RenderCommand::SetRenderTargets(ColorTarget, nullptr);
	RenderCommand::SetViewport(0, 0, Width, Height);
	RenderCommand::BindVertexBuffer(nullptr);
	RenderCommand::UpdateBufferData(ConstantBuffer.get(), &Data, sizeof(Data));
	RenderCommand::BindConstantBuffer(0, ConstantBuffer.get(), EShaderBindFlagBits::Pixel);
	RenderCommand::BindShaderResource(0, DepthTarget, EShaderBindFlagBits::Pixel);
	RenderCommand::Draw(6);
	RenderCommand::BindShaderResource(0, static_cast<FTexture2D*>(nullptr), EShaderBindFlagBits::Pixel);
	RenderCommand::SetRenderTargets(ColorTarget, DepthTarget);
	return true;
}