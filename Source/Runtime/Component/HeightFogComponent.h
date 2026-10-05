#pragma once
#include "EnginePCH.h"
#include "PrimitiveComponent.h"

class HeightFogComponent : public UPrimitiveComponent
{
    float FogDensity;
    float FogHeightFalloff;
    float StartDistance;
    float FogCutoffDistance;
    float FogMaxOpacity;

    float FogInscatteringColor[4];


};