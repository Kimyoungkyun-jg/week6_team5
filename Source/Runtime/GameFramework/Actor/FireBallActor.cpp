#include "EnginePCH.h"
#include "FireBallActor.h"
#include "Component/FireBallComponent.h"

AFireBallActor::AFireBallActor()
{
	PrimaryActorTick.bCanEverTick = false;
	FireBallComponent = CreateDefaultSubobject<UFireBallComponent>("UFireBallComponent");
	SetRootComponent(FireBallComponent);
}

void AFireBallActor::BeginPlay()
{
	Super::BeginPlay();
}