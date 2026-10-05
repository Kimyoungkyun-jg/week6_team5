#pragma once
#include "EnginePCH.h"
#include "PrimitiveComponent.h"

class UHeightFogComponent : public UPrimitiveComponent
{
public: 
    float FogDensity;
    float FogHeightFalloff;
    float StartDistance;
    float FogCutoffDistance;
    float FogMaxOpacity;
    float FogInscatteringColor[4];
};