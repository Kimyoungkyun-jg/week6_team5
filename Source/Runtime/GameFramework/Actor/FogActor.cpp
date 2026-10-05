#include "EnginePCH.h"
#include "FogActor.h"
#include "Component/ExponentialHeightFogComponent.h"

AFogActor::AFogActor()
{
	PrimaryActorTick.bCanEverTick = false;
	ExponentialHeightFogComponent = CreateDefaultSubobject<UExponentialHeightFogComponent>("UExponentialHeightFogComponent");
	BillboardComponent = CreateDefaultSubobject<UBillboardComponent>("UBillboardComponent");
	SetRootComponent(BillboardComponent);
	ExponentialHeightFogComponent->SetupAttachment(BillboardComponent);
}

void AFogActor::BeginPlay()
{
	Super::BeginPlay();
}