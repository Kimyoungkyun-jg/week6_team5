#include "EnginePCH.h"
#include "ProjectileActor.h"

AProjectileActor::AProjectileActor()
{
	PrimaryActorTick.bCanEverTick = true;
	FireBallComponent = CreateDefaultSubobject<UFireBallComponent>("UFireBallComponent");
	SetRootComponent(FireBallComponent);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>("UProjectileMovementComponent");
	ProjectileMovementComponent->SetUpdatedComponent(FireBallComponent);
}
