#pragma once

#include "GameFramework/Actor.h"
#include "Component/ExponentialHeightFogComponent.h"
#include "Component/BillboardComponent.h"

class AFogActor : public AActor
{
	DECLARE_CLASS(AFogActor, AActor)
	REFLECT_START(ClassName)
		REFLECT_END()
public:
	AFogActor();
	virtual ~AFogActor() override = default;

	virtual void BeginPlay() override;

	UExponentialHeightFogComponent* GetExponentialHeightFogComponent() const { return ExponentialHeightFogComponent; }

private:
	UExponentialHeightFogComponent* ExponentialHeightFogComponent = nullptr;
	UBillboardComponent* BillboardComponent = nullptr;
};