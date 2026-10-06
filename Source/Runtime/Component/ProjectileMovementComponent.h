#pragma once

#include "MovementComponent.h"

class UProjectileMovementComponent : public UMovementComponent
{
	DECLARE_CLASS(UProjectileMovementComponent, UMovementComponent)
	REFLECT_START(ClassName)
		PROPERTY(InitialSpeed)
		PROPERTY(MaxSpeed)
		PROPERTY(bShouldBounce)
		PROPERTY(Bounciness)
		PROPERTY(bRotationFollowsVelocity)
		PROPERTY(GravityScale)
		REFLECT_END()
public:
	UProjectileMovementComponent();
	UProjectileMovementComponent(float InInitialSpeed, float InMaxSpeed, bool bInShouldBounce, float InBounciness, float InGravityScale);
	virtual ~UProjectileMovementComponent() override;

	virtual void BeginPlay() override;
	virtual void UpdateComponent(float DeltaTime) override;

	float GetInitialSpeed() const { return InitialSpeed; }
	void SetInitialSpeed(float InInitialSpeed) { InitialSpeed = InInitialSpeed; }

	float GetMaxSpeed() const { return MaxSpeed; }
	void SetMaxSpeed(float InMaxSpeed) { MaxSpeed = InMaxSpeed; }

	bool GetShouldBounce() const { return bShouldBounce; }
	void SetShouldBounce(bool bInShouldBounce) { bShouldBounce = bInShouldBounce; }

	bool GetRotationFollowsVelocity() const { return bRotationFollowsVelocity; }
	void SetRotationFollowsVelocity(bool bInRotationFollowsVelocity) { bRotationFollowsVelocity = bInRotationFollowsVelocity; }

	float GetBounciness() const { return Bounciness; }
	void SetBounciness(float InBounciness) { Bounciness = InBounciness; }

	float GetGravityScale() const { return GravityScale; }
	void SetGravityScale(float InGravityScale) { GravityScale = InGravityScale; }

private:
	float InitialSpeed = 20.0f;
	float MaxSpeed = 30.0f;
	bool bShouldBounce = false;
	float Bounciness = 0.6f;
	bool bRotationFollowsVelocity = false;
	float GravityScale = 1.0f;
};