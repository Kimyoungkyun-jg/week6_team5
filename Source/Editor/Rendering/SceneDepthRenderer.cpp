#include "EnginePCH.h"
#include "SceneDepthRenderer.h"
#include "Asset/AssetManager.h"
#include "Render/RenderCommand.h"
#include "Render/RenderResourceManager.h"

void FSceneDepthRenderer::Init(FRenderer* InRenderer)
{
	Renderer = InRenderer;
	Shader = FRenderResourceManager::GetShaderProgram("Resources/Shader/SceneDepthShader.hlsl");

	SceneDepthPipelineState.Shader = Shader;
	SceneDepthPipelineState.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	SceneDepthPipelineState.RasterizerState = ERasterizerState::SolidNone;
	SceneDepthPipelineState.BlendState = EBlendState::Opaque;
	SceneDepthPipelineState.DepthStencilState = EDepthStencilState::Disabled;

	static_assert(sizeof(FSceneDepthData) == 16, "Scene depth constant buffer layout mismatch");
	ConstantBuffer = RenderCommand::CreateConstantBuffer(sizeof(FSceneDepthData));
}

void FSceneDepthRenderer::OnRender(FTexture2D* DepthTarget, const FSceneView& SceneView, const FViewportSettings& Viewport, const float MaxVisualizeDist)
{
	if (!DepthTarget || Viewport.Width == 0 || Viewport.Height == 0)
	{
		return;
	}


	FSceneDepthData SceneDepthData;
	SceneDepthData.DepthA = 1.0f / SceneView.FarClip - 1.0f / SceneView.NearClip;
	SceneDepthData.DepthB = 1.0f / SceneView.NearClip;
	SceneDepthData.InvMaxVisualizeDist = 1.0f / MaxVisualizeDist;
	SceneDepthData.Padding = 0.0f;
	// 상수 버퍼에 값 업데이트
	RenderCommand::UpdateBufferData(ConstantBuffer.get(), &SceneDepthData, sizeof(FSceneDepthData));
	// 파이프라인 상태
	RenderCommand::BindPipelineState(SceneDepthPipelineState);
	// 상수 버퍼 바인딩
	RenderCommand::BindConstantBuffer(0, ConstantBuffer.get(), EShaderBindFlagBits::Pixel);
	// 깊이 텍스처 바인딩
	RenderCommand::BindShaderResource(0, DepthTarget, EShaderBindFlagBits::Pixel);
	RenderCommand::BindSamplerState(0, ESamplerState::LinearClamp, EShaderBindFlagBits::Pixel);
	// 큰 삼각형 그리기
	RenderCommand::Draw(3, 0);
	// 리소스 언바인딩
	ID3D11ShaderResourceView* NullSRV = nullptr;
	RenderCommand::GetContext()->PSSetShaderResources(0, 1, &NullSRV);
}