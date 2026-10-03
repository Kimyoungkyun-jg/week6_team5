#include "EnginePCH.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/DefaultPawn.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Math/Frustum.h"
#include "Render/RenderCommand.h"

FGameViewportClient::FGameViewportClient()
{
}

void FGameViewportClient::Init(FWorldContext& InWorldContext, UEngine* InEngine)
{
	World = InWorldContext.World();
	GameInstance = InWorldContext.OwningGameInstance;
	Engine = InEngine;
}

void FGameViewportClient::Draw(FViewport* Viewport)
{

}

void FGameViewportClient::Tick(float DeltaTime)
{
}

UWorld* FGameViewportClient::GetWorld() const
{
	return World;
}

void FGameViewportClient::MouseMove(FViewport* Viewport, int32 X, int32 Y)
{
}

bool FGameViewportClient::InputKey(FViewport* Viewport, int32 Key, bool bDown)
{
	return false;
}

bool FGameViewportClient::InputAxis(FViewport* Viewport, int32 ControllerId, float Delta, float DeltaTime)
{
	return false;
}

FSceneView FGameViewportClient::CalcSceneView(const FRect& InViewRect)
{
	FSceneView OutView;
	OutView.ViewIndex = 0;
	OutView.ViewRect = InViewRect;

	const float Width = InViewRect.Width > 0.0f ? InViewRect.Width : 1.0f;
	const float Height = InViewRect.Height > 0.0f ? InViewRect.Height : 1.0f;
	const float Aspect = Width / Height;


	// 카메라 컴포넌트 획득
	if (CameraComponent == nullptr && World)
	{
		if (APawn* Pawn = World->GetPlayerPawn())
		{
			if (ADefaultPawn* DefPawn = Cast<ADefaultPawn>(Pawn))
			{
				CameraComponent = DefPawn->GetCameraComponent();
			}
			if (CameraComponent == nullptr)
			{
				for (UActorComponent* Comp : Pawn->GetComponents())
				{
					if (UCameraComponent* Cam = Cast<UCameraComponent>(Comp))
					{
						CameraComponent = Cam;
						break;
					}
				}
			}
		}
		if (CameraComponent == nullptr && World->GetMainCamera())
		{
			CameraComponent = World->GetMainCamera()->GetCameraComponent();
		}
	}

	if (CameraComponent)
	{
		CameraComponent->SetAspectRatio(Aspect);

		OutView.ViewLocation = CameraComponent->GetWorldLocation();
		OutView.ViewRotation = CameraComponent->GetWorldRotation();
		OutView.ViewForward = OutView.ViewRotation.Quaternion().GetForwardVector();
		OutView.ViewMatrix = CameraComponent->GetViewMatrix();
		OutView.ProjectionMatrix = CameraComponent->GetProjectionMatrix();
		OutView.ViewProjectionMatrix = CameraComponent->GetViewProjectionMatrix();
		OutView.FieldOfView = CameraComponent->GetFieldOfView();
		OutView.AspectRatio = Aspect;
		OutView.NearClip = CameraComponent->GetNearZ();
		OutView.FarClip = CameraComponent->GetFarZ();
		OutView.OrthoWidth = CameraComponent->GetOrthoWidth();
		OutView.bIsPerspective = !CameraComponent->GetIsOrthogonal();
		OutView.bIsWireframe = false;
	}
	else
	{
		// 카메라 부재 시 기본값 설정
		OutView.ViewLocation = FVector(-5.0f, -10.0f, 10.0f);
		OutView.ViewRotation = FRotator(30.0f, 45.0f, 0.0f);
		OutView.ViewForward = OutView.ViewRotation.Quaternion().GetForwardVector();
		OutView.FieldOfView = 60.0f;
		OutView.AspectRatio = Aspect;
		OutView.NearClip = 0.1f;
		OutView.FarClip = 1000.0f;
		OutView.bIsPerspective = true;
		OutView.bIsWireframe = false;
	}

	// 절두체 평면 계산
	OutView.Frustum = ExtractFrustumPlanes(OutView.ViewProjectionMatrix);

	return OutView;
}

void FGameViewportClient::SetCameraComponent(UCameraComponent* InCameraComponent)
{
	CameraComponent = InCameraComponent;
}

// 뷰포트 크기 변경 및 타깃 생성
void FGameViewportClient::Resize(uint32 InWidth, uint32 InHeight)
{
	if (Width == InWidth && Height == InHeight && ColorTarget && DepthTarget)
	{
		return;
	}

	Width = InWidth;
	Height = InHeight;

	if (Width == 0 || Height == 0)
	{
		return;
	}

	D3D11_TEXTURE2D_DESC Desc{};
	Desc.Width = Width;
	Desc.Height = Height;
	Desc.MipLevels = 1;
	Desc.ArraySize = 1;
	Desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	Desc.SampleDesc.Count = 1;
	Desc.Usage = D3D11_USAGE_DEFAULT;
	Desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	ColorTarget = RenderCommand::CreateTexture2D(Desc);

	Desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	Desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	DepthTarget = RenderCommand::CreateTexture2D(Desc);
}

void FGameViewportClient::Reset()
{
	World = nullptr;
	GameInstance = nullptr;
	Engine = nullptr;
	CameraComponent = nullptr;

	// 타깃 버퍼 정리
	ColorTarget.reset();
	DepthTarget.reset();
	Width = 0;
	Height = 0;
}
