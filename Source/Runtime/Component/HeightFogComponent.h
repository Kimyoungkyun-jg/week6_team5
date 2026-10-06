#pragma once
#include "EnginePCH.h"
#include "PrimitiveComponent.h"

class UHeightFogComponent : public UPrimitiveComponent
{
    DECLARE_CLASS(UHeightFogComponent, UPrimitiveComponent)
    REFLECT_START(UHeightFogComponent)
        PROPERTY(FogDensity)
        PROPERTY(FogHeightFalloff)
        PROPERTY(StartDistance)
        PROPERTY(FogCutoffDistance)
        PROPERTY(FogMaxOpacity)
        PROPERTY_TYPE(FogInscatteringColor, Color)
    REFLECT_END()
public: 
    float FogDensity = 0.02f;
    float FogHeightFalloff = 0.2f;
    float StartDistance = 0.0f;
    float FogCutoffDistance = 0.0f; // 0 disables the distance cutoff.
    float FogMaxOpacity = 1.0f;
    FVector4 FogInscatteringColor = FVector4(0.5f, 0.6f, 0.7f, 1.0f);
};
