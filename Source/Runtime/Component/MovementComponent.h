#pragma once

#include "ActorComponent.h"
#include "SceneComponent.h"

class UMovementComponent : public UActorComponent
{
	DECLARE_CLASS(UMovementComponent, UActorComponent)
	REFLECT_START(ClassName)
		REFLECT_END()
public:
	UMovementComponent();
	UMovementComponent(float InMaxSpeed, float InAcceleration, float InDeceleration);
	virtual ~UMovementComponent() override;

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime) override;
	virtual void UpdateComponent(float DeltaTime);

	FVector GetVelocity() const { return Velocity; }
	void SetVelocity(const FVector& InVelocity) { Velocity = InVelocity; }

	USceneComponent* GetUpdatedComponent() const { return UpdatedComponent; }
	void SetUpdatedComponent(USceneComponent* InUpdatedComponent) { UpdatedComponent = InUpdatedComponent; }

protected:
	FVector Velocity = FVector::ZeroVector;
	USceneComponent* UpdatedComponent;
};