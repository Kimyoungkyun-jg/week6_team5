#pragma once
#include "EnginePCH.h"
#include "PrimitiveComponent.h"

class UHeightFogComponent : public UPrimitiveComponent
{
    DECLARE_CLASS(UHeightFogComponent, UPrimitiveComponent)
public: 
    float FogDensity;
    float FogHeightFalloff;
    float StartDistance;
    float FogCutoffDistance;
    float FogMaxOpacity;
    float FogInscatteringColor[4];
};