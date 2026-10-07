#pragma once

#include "GameFramework/Actor.h"
#include "Component/FireBallComponent.h"
#include "Component/ProjectileMovementComponent.h"

// 투사체 액터 클래스
class AProjectileActor : public AActor
{
	DECLARE_CLASS(AProjectileActor, AActor)
	REFLECT_START(ClassName)
	REFLECT_END()

public:
	AProjectileActor();
	virtual ~AProjectileActor() override = default;

	UFireBallComponent* GetFireBallComponent() const { return FireBallComponent; }
	UProjectileMovementComponent* GetProjectileMovementComponent() const { return ProjectileMovementComponent; }

private:
	// 파이어볼 컴포넌트
	UFireBallComponent* FireBallComponent = nullptr;
	// 투사체 이동 컴포넌트
	UProjectileMovementComponent* ProjectileMovementComponent = nullptr;
};
