#include "EnginePCH.h"
#include "PlayerController.h"
#include "DefaultPawn.h"
#include "Input/InputSystem.h"

APlayerController::APlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APlayerController::GetPlayerViewCamera(FVector& OutLocation, FRotator& OutRotation) const
{
	if(PossessedPawn)
	{
		OutLocation = PossessedPawn->GetActorLocation();
		OutRotation = PossessedPawn->GetActorRotation();
	}
	else
	{
		OutLocation = CameraLocation;
		OutRotation = CameraRotation;
	}
}

void APlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void APlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// 입력 처리 및 카메라 시점 업데이트
	float ForwardInput = 0.0f;
	float RightInput = 0.0f;

	if(FInputSystem::IsKeyDown(EKeyCode::W))
	{
		ForwardInput = 1.0f;
	}
	if(FInputSystem::IsKeyDown(EKeyCode::S))
	{
		ForwardInput = -1.0f;
	}
	if(FInputSystem::IsKeyDown(EKeyCode::A))
	{
		RightInput = -1.0f;
	}
	if(FInputSystem::IsKeyDown(EKeyCode::D))
	{
		RightInput = 1.0f;
	}

	float YawDelta = FInputSystem::GetMouseDeltaX() * LookSensitivity;
	float PitchDelta = FInputSystem::GetMouseDeltaY() * LookSensitivity;

	if (FInputSystem::IsMouseDown(EMouseButton::Right))
	{
		if (PossessedPawn)
		{
			PossessedPawn->AddControllerYawInput(YawDelta);
			PossessedPawn->AddControllerPitchInput(PitchDelta);
		}
	}

	if(PossessedPawn)
	{
		FRotator CurrentRotation = PossessedPawn->GetActorRotation();
		FVector Forward = CurrentRotation.Quaternion().GetForwardVector();
		FVector Right = CurrentRotation.Quaternion().GetRightVector();
		if(ForwardInput != 0.0f)
		{
			PossessedPawn->AddMovementInput(Forward, ForwardInput * DeltaTime);
		}
		if(RightInput != 0.0f)
		{
			PossessedPawn->AddMovementInput(Right, RightInput * DeltaTime);
		}

		CameraLocation = PossessedPawn->GetActorLocation();
	}
	else
	{
		// 폰이 없으면 카메라 위치와 회전을 업데이트
		FVector Forward = CameraRotation.Vector();
		FVector Right = FVector::Cross(FVector(0.0f, 0.0f, 1.0f), Forward).Normalized();
		FVector Up = FVector::Cross(Forward, Right).Normalized();
		
		CameraLocation += (Forward * ForwardInput + Right * RightInput) * DeltaTime;
		CameraRotation.Pitch += PitchDelta;
		CameraRotation.Yaw += YawDelta;
	}
}	

void APlayerController::Possess(APawn* InPawn)
{
	if (InPawn)
	{
		PossessedPawn = InPawn;
		// 폰의 소유자 설정
		InPawn->SetOwner(this);
	}
}

void APlayerController::UnPossess()
{
	if (PossessedPawn)
	{
		// 폰의 소유자 해제
		PossessedPawn->SetOwner(nullptr);
		PossessedPawn = nullptr;
	}
}

void APlayerController::DuplicateSubobjects(UObject* SourceObject)
{
	Super::DuplicateSubobjects(SourceObject);
	APlayerController* SourceController = static_cast<APlayerController*>(SourceObject);
	if (SourceController)
	{
		// 카메라 위치와 회전 복제
		CameraLocation = SourceController->CameraLocation;
		CameraRotation = SourceController->CameraRotation;
		FOV = SourceController->FOV;
		LookSensitivity = SourceController->LookSensitivity;
	}
}

