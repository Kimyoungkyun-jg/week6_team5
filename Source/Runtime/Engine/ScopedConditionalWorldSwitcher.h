#pragma once

class UWorld;

// 현재 실행 중인 World를 가리키는 비소유 포인터. 명시적인 World 전달을 대체하지 않는다.
extern UWorld* GWorld;

// UE FScopedConditionalWorldSwitcher의 명시적 World 지정 경로에 대응한다.
// 현재 엔진에는 ViewportClient/PIE 전환 delegate가 없어 GWorld 저장·복원만 구현한다.
// 게임 스레드에서만 사용한다. 스코프 내부에서 현재/이전 World를 삭제하면 안 된다.
class FScopedConditionalWorldSwitcher
{
public:
	explicit FScopedConditionalWorldSwitcher(UWorld* InWorld)
	{
		if (InWorld && GWorld != InWorld)
		{
			OldWorld = GWorld;
			bSwitched = true;
			GWorld = InWorld;
		}
	}
	~FScopedConditionalWorldSwitcher()
	{
		if (bSwitched) GWorld = OldWorld;
	}
	FScopedConditionalWorldSwitcher(const FScopedConditionalWorldSwitcher&) = delete;
	FScopedConditionalWorldSwitcher& operator=(const FScopedConditionalWorldSwitcher&) = delete;

private:
	UWorld* OldWorld = nullptr;
	bool bSwitched = false;
};
