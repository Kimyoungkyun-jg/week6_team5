// 다중 뷰포트 레이아웃 분할 계산
#include "EnginePCH.h"
#include "Editor/LevelEditor/MultipleViewports/Core/MultipleViewports.h"
#include "Math/EngineMath.h"
#include <cassert>

// 분할 비율을 대칭 범위로 제한한다
FSplitRatio ClampSplitRatio(const FSplitRatio& Raw, const float MinRatio)
{
    assert(MinRatio >= 0.0f && MinRatio <= 0.5f);
    return {
        FMath::Clamp(Raw.Horizontal, MinRatio, 1.0f - MinRatio),
        FMath::Clamp(Raw.Vertical, MinRatio, 1.0f - MinRatio)};
}

// 드래그 이동량을 분할 비율에 반영한다
FSplitRatio ApplySplitterDrag(const FSplitRatio& Current, const EDragAxis Axis, const float DeltaPixels, const FVector2 WindowSize, const float MinRatio)
{
    FSplitRatio Result = Current;
    if (Axis == EDragAxis::Horizontal)
    {
        assert(WindowSize.X > 0.0f);
        Result.Horizontal += DeltaPixels / WindowSize.X;
    }
    else
    {
        assert(WindowSize.Y > 0.0f);
        Result.Vertical += DeltaPixels / WindowSize.Y;
    }
    return ClampSplitRatio(Result, MinRatio);
}

// 분할 비율과 윈도우 크기로 네 뷰포트 사각형을 계산한다
void ComputeViewRects(const FSplitRatio& Ratio, const FVector2 WindowSize, FRect OutRects[4])
{
    assert(WindowSize.X >= 0.0f && WindowSize.Y >= 0.0f);
    assert(Ratio.Horizontal >= 0.0f && Ratio.Horizontal <= 1.0f);
    assert(Ratio.Vertical >= 0.0f && Ratio.Vertical <= 1.0f);
    const float LeftWidth = WindowSize.X * Ratio.Horizontal;
    const float TopHeight = WindowSize.Y * Ratio.Vertical;
    const float RightWidth = WindowSize.X - LeftWidth;
    const float BottomHeight = WindowSize.Y - TopHeight;
    OutRects[0] = {0.0f, 0.0f, LeftWidth, TopHeight};
    OutRects[1] = {LeftWidth, 0.0f, RightWidth, TopHeight};
    OutRects[2] = {0.0f, TopHeight, LeftWidth, BottomHeight};
    OutRects[3] = {LeftWidth, TopHeight, RightWidth, BottomHeight};
}
