#include "EnginePCH.h"
#include "ProjectileMovementComponent.h"
#include "MovementComponent.h"

UProjectileMovementComponent::UProjectileMovementComponent()
	: InitialSpeed(20.0f)
	, bShouldBounce(true)
	, bRotationFollowsVelocity(true)
	, Bounciness(0.6f)
	, GravityScale(1.0f)
{
	PrimaryComponentTick.bCanEverTick = true;
}

UProjectileMovementComponent::~UProjectileMovementComponent()
{
}

UProjectileMovementComponent::UProjectileMovementComponent(float InInitialSpeed, float InMaxSpeed, bool bInShouldBounce, float InBounciness, float InGravityScale)
	: InitialSpeed(InInitialSpeed)
	, bShouldBounce(bInShouldBounce)
	, bRotationFollowsVelocity(true)
	, Bounciness(InBounciness)
	, GravityScale(InGravityScale)
{
	MaxSpeed = InMaxSpeed;
	PrimaryComponentTick.bCanEverTick = true;
}

void UProjectileMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	Velocity = UpdatedComponent ? UpdatedComponent->GetWorldRotation().Quaternion().GetForwardVector() * InitialSpeed : FVector::ZeroVector;
}

void UProjectileMovementComponent::TickComponent(float DeltaTime)
{
	Super::TickComponent(DeltaTime);

	if (!UpdatedComponent || DeltaTime <= 0.0f) return;

	float RemainingTime = DeltaTime;
	int32 Iterations = 0;

	while (RemainingTime > 0.0001f && Iterations < MaxSimulationIterations)
	{
		Iterations++;
		float TimeTick = RemainingTime;

		const FVector InitialVelocity = Velocity;
		// 이동량 계산
		FVector MoveDelta = ComputeMoveDelta(Velocity, TimeTick);
		// 속도 갱신
		FVector NewVelocity = ComputeVelocity(Velocity, TimeTick);
		Velocity = NewVelocity;
		// 회전 계산
		FRotator NewRotation = UpdatedComponent ? UpdatedComponent->GetWorldRotation() : FRotator();
		if (bRotationFollowsVelocity && NewVelocity.Size() > 0)
		{
			float HoizontalSpeed = sqrt(NewVelocity.X * NewVelocity.X + NewVelocity.Y * NewVelocity.Y);
			float Pitch = atan2(-NewVelocity.Z, HoizontalSpeed);
			float Yaw = atan2(NewVelocity.Y, NewVelocity.X);
			Pitch = FMath::RadiansToDegrees(Pitch);
			Yaw = FMath::RadiansToDegrees(Yaw);
			NewRotation = FRotator(Pitch, Yaw, 0);
		}
		FHitResult HitResult;

		MoveUpdatedComponent(MoveDelta, NewRotation, bSweep, &HitResult);
		// 실제 이동 시도
		if (HitResult.IsValidBlockingHit())
		{
			Velocity = ComputeVelocity(InitialVelocity, TimeTick * HitResult.Time);
			RemainingTime -= TimeTick * HitResult.Time;
			// 충돌 발생 시 연결
			HandleImpact(HitResult, RemainingTime, MoveDelta);
			if (!UpdatedComponent) break;
			if (Velocity.Size() < BounceVelocityStopSimulatingThreshold)
			{
				//FRotator FlatRotation(0.0f, UpdatedComponent->GetRelativeRotation().Yaw, 0.0f);
				//UpdatedComponent->SetRelativeRotation(FlatRotation);

				StopSimulating(HitResult);
				break;
			}
		}
		else
		{
			RemainingTime = 0.f;
			break;
		}
	}
}

FVector UProjectileMovementComponent::ComputeVelocity(FVector InitialVelocity, float DeltaTime) const
{
	const FVector Acceleration = ComputeAcceleration(InitialVelocity, DeltaTime);
	FVector NewVelocity = InitialVelocity + Acceleration * DeltaTime;

	return LimitVelocity(NewVelocity);
}

FVector UProjectileMovementComponent::LimitVelocity(FVector NewVelocity) const
{
	const float CurrentMaxSpeed = GetMaxSpeed();
	if (CurrentMaxSpeed > 0.f)
	{
		NewVelocity = NewVelocity.GetClampedToMaxSize(CurrentMaxSpeed);
	}

	// ToDo: 2D 환경에서 PlaneConstraint를 적용하려면 여기서 처리해야 한다.
	return NewVelocity;
}

FVector UProjectileMovementComponent::ComputeMoveDelta(const FVector& InVelocity, float DeltaTime) const
{
	const FVector NewVelocity = ComputeVelocity(InVelocity, DeltaTime);
	// 등가속도 운동 공식 : s = v0 * t + 0.5 * a * t^2
	const FVector MoveDelta = (InVelocity * DeltaTime) + (NewVelocity - InVelocity) * (0.5f * DeltaTime);
	return MoveDelta;
}

FVector UProjectileMovementComponent::ComputeAcceleration(const FVector& InVelocity, float DeltaTime) const
{
	FVector Acceleration = FVector::ZeroVector;

	if (bIsGravityEnabled)
		Acceleration.Z -= 9.8f * GravityScale; // 중력 가속도 적용

	// 호밍 및 다른 외부 힘을 적용하려면 여기서 PendingForceThisUpdate를 사용하여 가속도를 계산할 수 있다.
	// Acceleration += PendingForceThisUpdate;

	return Acceleration;
}

void UProjectileMovementComponent::StopSimulating(const FHitResult& HitResult)
{
	Velocity = FVector::ZeroVector;
	//PendingForce = FVector::ZeroVector;
	//PendingForceThisUpdate = FVector::ZeroVector;
	//UpdateComponentVelocity();
	SetUpdatedComponent(NULL);
}

void UProjectileMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	bool bStopSimulating = false;

	if (bShouldBounce)
	{
		const FVector OldVelocity = Velocity;
		Velocity = ComputeBounceResult(Hit, TimeSlice, MoveDelta);

		Velocity = LimitVelocity(Velocity);
	}
	else
	{
		bStopSimulating = true;
	}

	if(bStopSimulating)
	{
		StopSimulating(Hit);
	}
}

FVector UProjectileMovementComponent::ComputeBounceResult(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	FVector TempVelocity = Velocity;
	const FVector Normal = Hit.ImpactNormal;
	const float VDotNormal = TempVelocity.Dot(Normal);

	if (VDotNormal <= 0.0f)
	{
		const FVector ProjectedNormal = Normal * -VDotNormal;

		TempVelocity += ProjectedNormal;

		TempVelocity *= bIsFrictionEnabled ? FMath::Clamp(1.f - Friction, 0.f, 1.f) : 1.f;

		TempVelocity += ProjectedNormal * std::max(0.0f, Bounciness);

		TempVelocity = LimitVelocity(TempVelocity);
	}
	return TempVelocity;
}
