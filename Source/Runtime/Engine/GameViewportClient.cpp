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
	if (bDown && Key == static_cast<int32>(EKeyCode::Escape))
	{
		bExitRequested = true;
		return true;
	}

	if (bDown && Key == static_cast<int32>(EKeyCode::F8))
	{
		bSIEModeRequested = true;
		return true;
	}


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
	std::fill(std::begin(bSIEKeyDown), std::end(bSIEKeyDown), false);
	SIEMouseDeltaX = SIEMouseDeltaY = SIEWheelDelta = 0.0f;
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

	Resize(
		static_cast<uint32>(std::max(InViewRect.Width, 1.0f)),
		static_cast<uint32>(std::max(InViewRect.Height, 1.0f))
	); //여기서 바로 texture들 생성

	const float Aspect = static_cast<float>(Width) / static_cast<float>(Height);


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

bool FGameViewportClient::BeginSIEMode()
{
	if (bSIEMode)
		return true;
	if (!World || !World->GetMainCamera())
		return false;

	// 첫 렌더 이전에도 Pawn 카메라를 찾을 수 있도록 한다.
	if (!CameraComponent)
	{
		if (APawn* Pawn = World->GetPlayerPawn())
		{
			if (ADefaultPawn* DefaultPawn = Cast<ADefaultPawn>(Pawn))
				CameraComponent = DefaultPawn->GetCameraComponent();
			else
				for (UActorComponent* Component : Pawn->GetComponents())
					if (UCameraComponent* Camera = Cast<UCameraComponent>(Component))
					{
						CameraComponent = Camera;
						break;
					}
		}
	}
	if (!CameraComponent)
		return false;

	UCameraComponent* MainCamera = World->GetMainCamera()->GetCameraComponent();
	if (!MainCamera || MainCamera == CameraComponent)
		return false;

	MainCamera->SetRelativeLocation(CameraComponent->GetWorldLocation());
	MainCamera->SetRelativeRotation(CameraComponent->GetWorldRotation());
	MainCamera->SetFieldOfView(CameraComponent->GetFieldOfView());
	MainCamera->SetNearZ(CameraComponent->GetNearZ());
	MainCamera->SetFarZ(CameraComponent->GetFarZ());
	MainCamera->SetIsOrthogonal(CameraComponent->GetIsOrthogonal());
	MainCamera->SetOrthoWidth(CameraComponent->GetOrthoWidth());
	bPreviousMainCameraExternalInputManaged = MainCamera->IsExternalInputManaged();
	MainCamera->SetExternalInputManaged(true);
	CameraComponent = MainCamera;
	bSIEMode = true;
	std::fill(std::begin(bSIEKeyDown), std::end(bSIEKeyDown), false);
	SIEMouseDeltaX = SIEMouseDeltaY = SIEWheelDelta = 0.0f;
	LostFocus();
	return true;
}

void FGameViewportClient::EndSIEMode()
{
	if (!bSIEMode)
		return;
	if (World && World->GetMainCamera())
		if (UCameraComponent* MainCamera = World->GetMainCamera()->GetCameraComponent())
			MainCamera->SetExternalInputManaged(bPreviousMainCameraExternalInputManaged);
	CameraComponent = nullptr;
	if (World)
	{
		if (APawn* Pawn = World->GetPlayerPawn())
		{
			if (ADefaultPawn* DefaultPawn = Cast<ADefaultPawn>(Pawn))
				CameraComponent = DefaultPawn->GetCameraComponent();
			else
				for (UActorComponent* Component : Pawn->GetComponents())
					if (UCameraComponent* Camera = Cast<UCameraComponent>(Component))
					{
						CameraComponent = Camera;
						break;
					}
		}
	}
	bSIEMode = false;
	std::fill(std::begin(bSIEKeyDown), std::end(bSIEKeyDown), false);
	SIEMouseDeltaX = SIEMouseDeltaY = SIEWheelDelta = 0.0f;
}

void FGameViewportClient::GetKeyInputBySIEMode(int32 Key, bool bDown)
{
	if (bSIEMode && Key >= 0 && Key < 256)
		bSIEKeyDown[Key] = bDown;
}

void FGameViewportClient::GetAxisInputBySIEMode(EGameInputAxis Key, float Delta)
{
	if (!bSIEMode)
		return;
	switch (Key)
	{
	case EGameInputAxis::MouseX: SIEMouseDeltaX += Delta; break;
	case EGameInputAxis::MouseY: SIEMouseDeltaY += Delta; break;
	case EGameInputAxis::MouseWheel: SIEWheelDelta += Delta; break;
	}
}

void FGameViewportClient::TickSIEInput(float DeltaTime, float MoveSpeed, float MouseSensitivity)
{
	if (!bSIEMode || !CameraComponent)
		return;
	const bool bCaptured = bSIEKeyDown[static_cast<int32>(EKeyCode::RButton)];
	const bool bPerspective = !CameraComponent->GetIsOrthogonal();
	FRotator Rotation = CameraComponent->GetRelativeRotation();
	FVector Location = CameraComponent->GetRelativeLocation();

	if (bCaptured)
	{
		if (bPerspective)
		{
			Rotation.Yaw += SIEMouseDeltaX * MouseSensitivity;
			Rotation.Pitch = FMath::Clamp(Rotation.Pitch + SIEMouseDeltaY * MouseSensitivity, -89.0f, 89.0f);
			CameraComponent->SetRelativeRotation(Rotation);
		}
		else
		{
			const float ViewWidth = Width > 0 ? static_cast<float>(Width) : 800.0f;
			const float WorldUnitsPerPixel = CameraComponent->GetOrthoWidth() / ViewWidth;
			const FQuat RotationQuat = Rotation.Quaternion();
			Location += RotationQuat.GetRightVector() * (-SIEMouseDeltaX * WorldUnitsPerPixel);
			Location += RotationQuat.GetUpVector() * (SIEMouseDeltaY * WorldUnitsPerPixel);
		}

		const FQuat RotationQuat = Rotation.Quaternion();
		const FVector Forward = RotationQuat.GetForwardVector();
		const FVector Right = RotationQuat.GetRightVector();
		const FVector Up = RotationQuat.GetUpVector();
		FVector MoveDir = FVector::ZeroVector;
		if (bSIEKeyDown[static_cast<int32>(EKeyCode::W)]) MoveDir += bPerspective ? Forward : Up;
		if (bSIEKeyDown[static_cast<int32>(EKeyCode::S)]) MoveDir -= bPerspective ? Forward : Up;
		if (bSIEKeyDown[static_cast<int32>(EKeyCode::D)]) MoveDir += Right;
		if (bSIEKeyDown[static_cast<int32>(EKeyCode::A)]) MoveDir -= Right;
		if (bPerspective && bSIEKeyDown[static_cast<int32>(EKeyCode::E)]) MoveDir += Up;
		if (bPerspective && bSIEKeyDown[static_cast<int32>(EKeyCode::Q)]) MoveDir -= Up;
		if (MoveDir.Size() > 0.0001f)
			Location += MoveDir.Normalized() * (MoveSpeed * DeltaTime);
	}

	if (SIEWheelDelta != 0.0f)
	{
		if (bPerspective)
			Location += Rotation.Quaternion().GetForwardVector() * (SIEWheelDelta * 0.01f * MoveSpeed);
		else
		{
			const float ZoomFactor = SIEWheelDelta > 0.0f ? 0.9f : 1.1f;
			CameraComponent->SetOrthoWidth(FMath::Clamp(CameraComponent->GetOrthoWidth() * ZoomFactor, 0.1f, 100000.0f));
		}
	}
	CameraComponent->SetRelativeLocation(Location);
	SIEMouseDeltaX = SIEMouseDeltaY = SIEWheelDelta = 0.0f;
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
	EndSIEMode();
	bExitRequested = false;
	bSIEModeRequested = false;
	World = nullptr;
	GameInstance = nullptr;
	Engine = nullptr;
	CameraComponent = nullptr;

	// 타깃 버퍼 정리
	ViewTargets.Reset();
	Width = 0;
	Height = 0;
}
