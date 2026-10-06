#include "EnginePCH.h"
#include "ProjectileMovementComponent.h"
#include "MovementComponent.h"

UProjectileMovementComponent::UProjectileMovementComponent()
	: InitialSpeed(1000.0f)
	, bShouldBounce(false)
	, bRotationFollowsVelocity(false)
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
	, bRotationFollowsVelocity(false)
	, Bounciness(InBounciness)
	, GravityScale(InGravityScale)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UProjectileMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	Velocity = UpdatedComponent ? UpdatedComponent->GetTransform().GetForward() * InitialSpeed : FVector::ZeroVector;
}

void UProjectileMovementComponent::TickComponent(float DeltaTime)
{
	Super::TickComponent(DeltaTime);
	// 이동량 계신
	FVector MoveDelta = ComputeMoveDelta(Velocity, DeltaTime);
	// 속도 갱신
	FVector NewVelocity = ComputeVelocity(Velocity, DeltaTime);
	Velocity = NewVelocity;
	// 회전 계산
	FRotator NewRotation = UpdatedComponent ? UpdatedComponent->GetRelativeRotation() : FRotator();
	if (bRotationFollowsVelocity && NewVelocity.Size() > 0)
	{
		float HoizontalSpeed = sqrt(NewVelocity.X * NewVelocity.X + NewVelocity.Y * NewVelocity.Y);
		float Pitch = atan2(NewVelocity.Z, HoizontalSpeed);
		float Yaw = atan2(NewVelocity.Y, NewVelocity.X);
		Pitch = FMath::RadiansToDegrees(Pitch);
		Yaw = FMath::RadiansToDegrees(Yaw);
		NewRotation = FRotator(Pitch, Yaw, 0);
	}
	FHitResult HitResult;

	MoveUpdatedComponent(MoveDelta, NewRotation, bSweep, &HitResult);
	// 실제 이동 시도
	if(HitResult.IsValidBlockingHit())
	{
		// 충돌 발생 시 연결
		HandleImpact(HitResult, 0.f, MoveDelta);
	}
}

/*void UProjectileMovementComponent::UpdateComponent(float DeltaTime)
{
	if (!UpdatedComponent) return;
	Velocity.Z -= 9.8f * GravityScale * DeltaTime; // 중력 가속도 적용
	
	// 초과 잘라주기
	if (Velocity.Size() > MaxSpeed)
	{
		Velocity = Velocity.Normalized() * MaxSpeed;
	}

	// 위치 업데이트
	UpdatedComponent->SetRelativeLocation(UpdatedComponent->GetRelativeLocation() + Velocity * DeltaTime);

	// 이동 방향으로 회전 적용
	if (bRotationFollowsVelocity && Velocity.Size() > 0)
	{
		float HoizontalSpeed = sqrt(Velocity.X * Velocity.X + Velocity.Y * Velocity.Y);
		float Pitch = atan2(Velocity.Z, HoizontalSpeed);
		float Yaw = atan2(Velocity.Y, Velocity.X);
		Pitch = FMath::RadiansToDegrees(Pitch);
		Yaw = FMath::RadiansToDegrees(Yaw);
		UpdatedComponent->SetRelativeRotation(FRotator(Pitch, Yaw, 0));
	}
}*/

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
	//OnProjectileStop.Broadcast(HitResult);
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
	FVector NewVelocity = Velocity;
	// 충돌한 표면의 법선 벡터를 기준으로 반사 벡터 계산
	FVector Normal = Hit.ImpactNormal;
	float VDotN = NewVelocity.Dot(Normal);
	NewVelocity = NewVelocity - Normal * 2.0f * VDotN;
	// 반사 후 속도에 Bounciness 적용
	NewVelocity *= Bounciness;
	return NewVelocity;
}
