#pragma once

#include "SceneComponent.h"
#include "Math/Vector4.h"

// 한 층의 높이 안개 설정을 보관한다. 렌더러가 매 뷰에서 현재 값을 읽는다.
class UExponentialHeightFogComponent : public USceneComponent
{
	DECLARE_CLASS(UExponentialHeightFogComponent, USceneComponent)

	REFLECT_START(ClassName)
		PROPERTY(bVisible)
		PROPERTY(FogDensity)
		PROPERTY(FogHeightFalloff)
		PROPERTY_TYPE(FogColor, Color)
		PROPERTY(StartDistance)
		PROPERTY(FogMaxOpacity)
		PROPERTY(FogCutoffDistance)
	REFLECT_END()

public:
	UExponentialHeightFogComponent() = default;
	~UExponentialHeightFogComponent() override = default;

	bool IsVisible() const { return bVisible; }
	void SetVisible(bool bInVisible) { bVisible = bInVisible; }

	float GetFogDensity() const { return FogDensity; }
	void SetFogDensity(float InDensity) { FogDensity = InDensity; }

	float GetFogHeightFalloff() const { return FogHeightFalloff; }
	void SetFogHeightFalloff(float InFalloff) { FogHeightFalloff = InFalloff; }

	const FVector4& GetFogColor() const { return FogColor; }
	void SetFogColor(const FVector4& InColor) { FogColor = InColor; }

	float GetStartDistance() const { return StartDistance; }
	void SetStartDistance(float InDistance) { StartDistance = InDistance; }

	float GetFogMaxOpacity() const { return FogMaxOpacity; }
	void SetFogMaxOpacity(float InOpacity) { FogMaxOpacity = InOpacity; }

	float GetFogCutoffDistance() const { return FogCutoffDistance; }
	void SetFogCutoffDistance(float InDistance) { FogCutoffDistance = InDistance; }

	// 부모 변환까지 반영된 현재 높이를 사용하므로 별도 갱신이 필요 없다.
	float GetFogHeight() const { return GetWorldLocation().Z; }

private:
	bool bVisible = true;

	// 단위는 월드 길이의 역수. 현재 씬에서 조절할 초기값이다.
	float FogDensity = 0.02f;
	float FogHeightFalloff = 0.2f;

	// RGB만 사용한다. 합성 알파는 렌더러가 투과율로 계산한다.
	FVector4 FogColor = FVector4(0.5f, 0.6f, 0.7f, 1.0f);
	float StartDistance = 0.0f;
	float FogMaxOpacity = 1.0f;
	// 0이면 차단 거리 제한을 사용하지 않는다.
	float FogCutoffDistance = 0.0f;
};
