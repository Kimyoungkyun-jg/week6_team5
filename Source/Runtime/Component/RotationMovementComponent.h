#pragma once
#include "EnginePCH.h"
#include "ActorComponent.h"

class AActor;

class RotatingMovementComponent : public UActorComponent
{
	DECLARE_CLASS(RotatingMovementComponent, UActorComponent)
	REFLECT_START(RotatingMovementComponent)
		PROPERTY(RotationRate)
		PROPERTY(PivotTranslation)
		PROPERTY(bRotateLocal)
	REFLECT_END()

public:
	RotatingMovementComponent();

	void BeginPlay();
	void TickComponent(float DeltaTime) override;
	void SetRotationRate(FRotator Rotate);
	void SetPivotTranslation(FVector Pivot);
	void SetbRotationInLocalSpace(bool bRotationInLocal) { bRotateLocal = bRotationInLocal; }
	const FRotator GetRotationRate() { return RotationRate; }
	const FVector GetPivotTranslation() { return PivotTranslation; }
	const bool GetbRotationInLocalSpace() { return bRotateLocal; }

private:
	FRotator RotationRate;
	FVector PivotTranslation;
	bool bRotateLocal= true;
};
