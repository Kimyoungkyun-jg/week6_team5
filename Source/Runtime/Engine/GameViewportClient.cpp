#include "EnginePCH.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/DefaultPawn.h"
#include "GameFramework/PlayerController.h"
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
	HandleUIMouseMove(X, Y);
}

bool FGameViewportClient::InputKey(FViewport* Viewport, int32 Key, bool bDown)
{
	if (HandleUIKey(Key, bDown))
		return true;

	if (World)
	{
		if (APlayerController* PC = World->GetPlayerController())
		{
			PC->InputKey(Key, bDown);
			return true;
		}
	}
	return false;
}

bool FGameViewportClient::InputAxis(FViewport* Viewport, int32 ControllerId, EGameInputAxis AxisKey, float Delta, float DeltaTime)
{
	if (HandleUIAxis(AxisKey, Delta))
		return true;

	if (World)
	{
		if (APlayerController* PC = World->GetPlayerController())
		{
			PC->InputAxis(AxisKey, Delta);
			return true;
		}
	}
	return false;
}

void FGameViewportClient::LostFocus()
{
	if (World)
	{
		if (APlayerController* PC = World->GetPlayerController())
			PC->ClearInputState();
	}
}

bool FGameViewportClient::HandleUIKey(int32 Key, bool bDown)
{
	return false;
}

bool FGameViewportClient::HandleUIAxis(EGameInputAxis AxisKey, float Delta)
{
	return false;
}

bool FGameViewportClient::HandleUIMouseMove(int32 X, int32 Y)
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
	if (Width == InWidth && Height == InHeight && ViewTargets.IsValidFor(InWidth, InHeight))
	{
		return;
	}

	Width = InWidth;
	Height = InHeight;

	if (Width == 0 || Height == 0)
	{
		ViewTargets.Reset();
		return;
	}

	ViewTargets.Resize(Width, Height);
}

void FGameViewportClient::Reset()
{
	World = nullptr;
	GameInstance = nullptr;
	Engine = nullptr;
	CameraComponent = nullptr;

	// 타깃 버퍼 정리
	ViewTargets.Reset();
	Width = 0;
	Height = 0;
}
