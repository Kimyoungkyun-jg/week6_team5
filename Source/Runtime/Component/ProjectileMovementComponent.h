#pragma once

#include "MovementComponent.h"

class UProjectileMovementComponent : public UMovementComponent
{
	DECLARE_CLASS(UProjectileMovementComponent, UMovementComponent)
	REFLECT_START(ClassName)
		PROPERTY(InitialSpeed)
		PROPERTY(bShouldBounce)
		PROPERTY(Bounciness)
		PROPERTY(bRotationFollowsVelocity)
		PROPERTY(GravityScale)
		PROPERTY(bSweep)
		REFLECT_END()
public:
	UProjectileMovementComponent();
	UProjectileMovementComponent(float InInitialSpeed, float InMaxSpeed, bool bInShouldBounce, float InBounciness, float InGravityScale);
	virtual ~UProjectileMovementComponent() override;

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime) override;

	FVector ComputeAcceleration(const FVector& InVelocity, float DeltaTime) const;
	virtual void HandleImpact(const FHitResult& Hit, float TimeSlice = 0.f, const FVector& MoveDelta = FVector::ZeroVector) override;
	FVector LimitVelocity(FVector NewVelocity) const;
	virtual FVector ComputeVelocity(FVector InitialVelocity, float DeltaTime) const;
	virtual FVector ComputeMoveDelta(const FVector& InVelocity, float DeltaTime) const;
	virtual FVector ComputeBounceResult(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta);
	void StopSimulating(const FHitResult& HitResult);

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
	bool bShouldBounce = true;
	float Bounciness = 0.6f;
	bool bRotationFollowsVelocity = true;
	float GravityScale = 1.0f;
	bool bSweep = true;
};