#pragma once

#include "PrimitiveComponent.h"

struct FFireBallMaterialParams
{
	FVector Center;
	float Radius = 1.0f;
	FVector4 Color;
	FVector CameraPosition;
	float RadiusFalloff = 0.5f;
	float Intensity = 1.0f;
	float Padding[3];
};

static_assert(sizeof(FFireBallMaterialParams) == 64);

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
	virtual void SubmitToRenderQueue(FRenderQueue& RenderQueue, const FViewContext& ViewContext) override;
	virtual FBox CalcLocalBounds() const override;

private:
	float Radius = 1.0f;
	FVector4 Color = FVector4(1.0f, 0.5f, 0.0f, 1.0f);
	float RadiusFalloff = 0.5f;
	float Intensity = 1.0f;
	FFireBallMaterialParams MaterialParams{};
};
