#include "EnginePCH.h"
#include "ProjectileMovementComponent.h"

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
	, MaxSpeed(InMaxSpeed)
	, bShouldBounce(bInShouldBounce)
	, Bounciness(InBounciness)
	, GravityScale(InGravityScale)
	, bRotationFollowsVelocity(false)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UProjectileMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	Velocity = UpdatedComponent ? UpdatedComponent->GetTransform().GetForward() * InitialSpeed : FVector::ZeroVector;
}

void UProjectileMovementComponent::UpdateComponent(float DeltaTime)
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
}
