#include "EnginePCH.h"
#include "Camera/CameraComponent.h"
#include "Component/StaticMeshComponent.h"
#include "Asset/AssetManager.h"
#include "ObjectSystem/ObjectFactory.h"
#include "DefaultPawn.h"

ADefaultPawn::ADefaultPawn()
{
	CameraComponent = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("StaticMeshComponent");
	SetRootComponent(StaticMeshComponent);

	StaticMeshComponent->SetStaticMesh(UAssetManager::GetMesh("Hitori"));
	CameraComponent->SetupAttachment(StaticMeshComponent);

	CameraComponent->SetRelativeLocation({ -5,0,8 });
	CameraComponent->SetRelativeRotation({ 45,0,0 });
	CameraComponent->SetExternalInputManaged(true);
}



void ADefaultPawn::AddMovementInput(const FVector& WorldDirection, float ScaleValue)
{
    FVector NewLocation = GetActorLocation() + WorldDirection * (ScaleValue * MoveSpeed);
    if (RootComponent)
    {
        RootComponent->SetRelativeLocation(NewLocation);
    }
}

void ADefaultPawn::AddControllerYawInput(float Value)
{
	FRotator NewRotation = GetActorRotation();
	NewRotation.Yaw += Value;
	if (RootComponent)
	{
		RootComponent->SetRelativeRotation(NewRotation);
	}
}

void ADefaultPawn::AddControllerPitchInput(float Value)
{
	FRotator NewRotation = GetActorRotation();
	NewRotation.Pitch += Value;
	NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch + Value, -89.0f, 89.0f);
	if (RootComponent)
	{
		RootComponent->SetRelativeRotation(NewRotation);
	}
}