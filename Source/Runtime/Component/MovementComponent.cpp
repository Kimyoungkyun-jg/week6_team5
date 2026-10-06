#include "EnginePCH.h"
#include "MovementComponent.h"
#include "GameFramework/Actor.h"

UMovementComponent::UMovementComponent()
	: Velocity(FVector::ZeroVector)
	, UpdatedComponent(nullptr)
{
	PrimaryComponentTick.bCanEverTick = true;
}

UMovementComponent::~UMovementComponent()
{
}

UMovementComponent::UMovementComponent(float InMaxSpeed, float InAcceleration, float InDeceleration)
	: Velocity(FVector::ZeroVector)
	, UpdatedComponent(nullptr)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!UpdatedComponent && GetOwner())
	{
		UpdatedComponent = GetOwner()->GetRootComponent();
	}
}

void UMovementComponent::TickComponent(float DeltaTime)
{
	Super::TickComponent(DeltaTime);
	UpdateComponent(DeltaTime);
}

void UMovementComponent::UpdateComponent(float DeltaTime)
{
}