#pragma once

#include "Pawn.h"
#include "PlayerController.h"


class UCameraComponent;
class UStaticMeshComponent;

class ADefaultPawn : public APawn
{
	DECLARE_CLASS(ADefaultPawn, APawn)
	REFLECT_START(ClassName)
		REFLECT_END()
public:
	ADefaultPawn();
	virtual ~ADefaultPawn() override = default;

	// 이동 및 회전
	virtual void AddMovementInput(const FVector& WorldDirection, float ScaleValue) override;
	virtual void AddControllerYawInput(float Value) override;
	virtual void AddControllerPitchInput(float Value) override;

	// 카메라 컴포넌트 반환
	UCameraComponent* GetCameraComponent() const { return CameraComponent; }

protected:
	float MoveSpeed = 20.0f;
	UCameraComponent* CameraComponent = nullptr;
	UStaticMeshComponent* StaticMeshComponent = nullptr;


};