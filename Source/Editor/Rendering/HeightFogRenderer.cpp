#include "EnginePCH.h"
#include "HeightFogRenderer.h"
#include "Render/RenderCommand.h"
#include "Render/RenderResourceManager.h"
#include "Render/Texture2D.h"

void FHeightFogRenderer::Init(FRenderer* InRenderer)
{
	Renderer = InRenderer;
	Shader = FRenderResourceManager::GetShaderProgram("Resources/Shader/ExponentialHeightFogShader.hlsl");
	HeightFogPipelineState.Shader = Shader;
	HeightFogPipelineState.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	HeightFogPipelineState.RasterizerState = ERasterizerState::SolidNone;
	HeightFogPipelineState.BlendState = EBlendState::Opaque;
	HeightFogPipelineState.DepthStencilState = EDepthStencilState::Disabled;

	static_assert(sizeof(FHeightFogData) == 128, "Height fog constant buffer layout mismatch");
	ConstantBuffer = RenderCommand::CreateConstantBuffer(sizeof(FHeightFogData));
}

void FHeightFogRenderer::OnRender(FTexture2D* ColorTarget, FTexture2D* DepthTarget, const FSceneView& SceneView, const FViewportSettings& Viewport, UExponentialHeightFogComponent* FogComponent)
{
	if (!ColorTarget || !DepthTarget || Viewport.Width == 0 || Viewport.Height == 0 || !FogComponent)
	{
		return;
	}
	if (TempColorTarget == nullptr || TempColorTarget->GetWidth() != Viewport.Width || TempColorTarget->GetHeight() != Viewport.Height)
	{
		D3D11_TEXTURE2D_DESC Desc{};
		Desc.Width = Viewport.Width;
		Desc.Height = Viewport.Height;
		Desc.MipLevels = 1;
		Desc.ArraySize = 1;
		Desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		Desc.SampleDesc.Count = 1;
		Desc.Usage = D3D11_USAGE_DEFAULT;
		Desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

		TempColorTarget = RenderCommand::CreateTexture2D(Desc);
	}

	FHeightFogData HeightFogData;
	HeightFogData.InvViewProjection = SceneView.ViewProjectionMatrix.Inverse();
	HeightFogData.CameraWorldPosition = SceneView.ViewLocation;
	HeightFogData.FogDensity = FogComponent->GetFogDensity();
	HeightFogData.FogHeight = FogComponent->GetFogHeight();
	HeightFogData.FogHeightFalloff = FogComponent->GetFogHeightFalloff();
	HeightFogData.FogStartDistance = FogComponent->GetFogStartDistance();
	HeightFogData.FogCutoffDistance = FogComponent->GetFogCutoffDistance();
	HeightFogData.FogInscatteringColor = FogComponent->GetFogInscatteringColor();
	HeightFogData.FogMaxOpacity = FogComponent->GetFogMaxOpacity();

	// 상수 버퍼에 값 업데이트
	RenderCommand::UpdateBufferData(ConstantBuffer.get(), &HeightFogData, sizeof(FHeightFogData));
	// 임시 컬러 타겟으로 렌더링
	ID3D11RenderTargetView* TempRTV = TempColorTarget->GetRTV();
	RenderCommand::GetContext()->OMSetRenderTargets(1, &TempRTV, nullptr);
	// 파이프라인 상태
	RenderCommand::BindPipelineState(HeightFogPipelineState);
	// 상수 버퍼 바인딩
	RenderCommand::BindConstantBuffer(0, ConstantBuffer.get(), EShaderBindFlagBits::Pixel);
	// 컬러 타겟 바인딩
	RenderCommand::BindShaderResource(0, ColorTarget, EShaderBindFlagBits::Pixel);
	RenderCommand::BindSamplerState(0, ESamplerState::LinearClamp, EShaderBindFlagBits::Pixel);
	// 깊이 타겟 바인딩
	RenderCommand::BindShaderResource(1, DepthTarget, EShaderBindFlagBits::Pixel);
	RenderCommand::BindSamplerState(1, ESamplerState::LinearClamp, EShaderBindFlagBits::Pixel);
	// 큰 삼각형 그리기
	RenderCommand::Draw(3, 0);
	// 리소스 언바인딩
	ID3D11ShaderResourceView* NullSRV[2] = { nullptr, nullptr };
	RenderCommand::GetContext()->PSSetShaderResources(0, 2, NullSRV);
	// 안개 결과물을 원본 ColorTarget에 복사
	RenderCommand::GetContext()->CopyResource(ColorTarget->GetRawPtr(), TempColorTarget->GetRawPtr());
}