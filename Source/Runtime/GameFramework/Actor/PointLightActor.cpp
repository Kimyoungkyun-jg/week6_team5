#include "EnginePCH.h"
#include "GameFramework/Actor/PointLightActor.h"

APointLightActor::APointLightActor()
{
	BillboardComponent = CreateDefaultSubobject<UBillboardComponent>("UBillboardComponent");
	SetRootComponent(BillboardComponent);
	PointLightComponent = CreateDefaultSubobject<UPointLightComponent>("UPointLightComponent");
	PointLightComponent->SetupAttachment(BillboardComponent);
}
