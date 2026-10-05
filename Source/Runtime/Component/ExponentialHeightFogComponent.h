#pragma once
#include "ObjectSystem/Object.h"
#include "SceneComponent.h"

class UExponentialHeightFogComponent : public USceneComponent
{
	DECLARE_CLASS(UExponentialHeightFogComponent, USceneComponent)

	REFLECT_START(UExponentialHeightFogComponent)
		PROPERTY(FogDensity)
		PROPERTY(FogHeightFalloff)
		PROPERTY_TYPE(FogInscatteringColor, Color)
		PROPERTY(FogStartDistance)
		PROPERTY(FogCutoffDistance)
		PROPERTY(FogMaxOpacity)
		PROPERTY(bVisible)
		REFLECT_END()
public:
	UExponentialHeightFogComponent();
	virtual ~UExponentialHeightFogComponent() override;

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime) override;

	const bool IsVisible() const { return bVisible; }

	float GetFogDensity() const { return FogDensity; }
	float GetFogHeight() const { return GetWorldLocation().Z; }
	float GetFogHeightFalloff() const { return FogHeightFalloff; }
	const FVector4& GetFogInscatteringColor() const { return FogInscatteringColor; }
	float GetFogStartDistance() const { return FogStartDistance; }
	float GetFogCutoffDistance() const { return FogCutoffDistance; }
	float GetFogMaxOpacity() const { return FogMaxOpacity; }
private:
	// 안개 밀도
	float FogDensity = 0.02f;
	// 높이에 따른 지수 감쇄 계수
	float FogHeightFalloff = 0.01f;
	// 안개 색상
	FVector4 FogInscatteringColor = FVector4(0.5f, 0.5f, 0.5f, 1.0f);
	// 카메라 앞 안개 시작 최소 거리
	float FogStartDistance = 0.0f;
	// 안개 최대 차단 거리(0이면 무제한)
	float FogCutoffDistance = 0.0f;
	// 불투명도 클램프
	float FogMaxOpacity = 1.0f;

	bool bVisible = true;
};