#pragma once

#include "Core/Types.h"

class FViewport;
class UWorld;


enum class ELevelViewportType : uint8
{
	Perspective,
	Top,
	Front,
	Right
};

enum class EViewportMode : uint8
{
	Solid,
	Wireframe,
	SceneDepth,
	None
};

// 뷰포트 클라이언트 인터페이스
class FViewportClient
{
public:
	virtual ~FViewportClient() = default;

	// 렌더링
	virtual void Draw(FViewport* Viewport) {}

	// 갱신
	virtual void Tick(float DeltaTime) {}

	// 월드 조회
	virtual UWorld* GetWorld() const { return nullptr; }

	// 마우스 이동
	virtual void MouseMove(FViewport* Viewport, int32 X, int32 Y) {}

	// 키 입력
	virtual bool InputKey(FViewport* Viewport, int32 Key, bool bDown) { return false; }

	// 축 입력
	virtual bool InputAxis(FViewport* Viewport, int32 ControllerId, float Delta, float DeltaTime) { return false; }

	// 포커스 획득
	virtual void ReceivedFocus(FViewport* Viewport) {}

	// 포커스 해제
	virtual void LostFocus(FViewport* Viewport) {}

	// 크기 변경
	virtual void Resize(FViewport* Viewport, uint32 NewWidth, uint32 NewHeight) {}
};
