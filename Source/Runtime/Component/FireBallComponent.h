#pragma once

#include "PrimitiveComponent.h"

struct FFireBallMaterialParams
{
	FVector Center;
	float Radius = 1.0f;
	FVector4 Color;
	float RadiusFalloff = 0.5f;
	float Intensity = 1.0f;
	float Padding[2];
};

class UFireBallComponent : public UPrimitiveComponent
{
	DECLARE_CLASS(UFireBallComponent, UPrimitiveComponent)

	REFLECT_START(ClassName)
		PROPERTY_RANGE(Radius, 0.0f, 100.0f)
		PROPERTY_TYPE(Color, Color)
		PROPERTY_RANGE(RadiusFalloff, 0.0f, 5.0f)
		PROPERTY_RANGE(Intensity, 0.0f, 10.0f)
	REFLECT_END()
public:
	UFireBallComponent();
	virtual ~UFireBallComponent() override;

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime) override;

	virtual void SubmitToRenderQueue(FRenderQueue& RenderQueue, const FViewContext& ViewContext) override;

private:
	float Radius = 1.0f;
	FVector4 Color = FVector4(1.0f, 0.5f, 0.0f, 1.0f);
	float RadiusFalloff = 0.5f;
	float Intensity = 1.0f;
	FFireBallMaterialParams MaterialParams;
};
