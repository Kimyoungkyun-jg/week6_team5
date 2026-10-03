#include "EnginePCH.h"
#include "Component/SceneComponent.h"
#include "DefaultPawn.h"

ADefaultPawn::ADefaultPawn()
{
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