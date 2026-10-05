#pragma once

#include "GameFramework/Actor.h"
#include "Component/FireBallComponent.h"

class AFireBallActor : public AActor
{
	DECLARE_CLASS(AFireBallActor, AActor)
	REFLECT_START(ClassName)
		REFLECT_END()

public:
	AFireBallActor();
	virtual ~AFireBallActor() = default;

	virtual void BeginPlay() override;

	UFireBallComponent* GetFireBallComponent() const { return FireBallComponent; }

private:
	UFireBallComponent* FireBallComponent;
};
