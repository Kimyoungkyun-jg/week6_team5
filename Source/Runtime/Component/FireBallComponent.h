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
		PROPERTY(Radius)
		PROPERTY_TYPE(Color, Color)
		PROPERTY(RadiusFalloff)
		PROPERTY(Intensity)
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
