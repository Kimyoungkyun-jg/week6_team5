#pragma once

#include "ObjectSystem/Object.h"
#include "ObjectSystem/Class.h"
#include "Engine/WorldContext.h"

class UGameInstance : public UObject
{
	DECLARE_CLASS(UGameInstance, UObject)

public:
	// 향후 LocalPlayer 등 세션 동안 유지할 객체를 관리한다.
	// World 교체와 수명을 분리하며, 현재는 Context에 대한 비소유 참조만 둔다.
	UWorld* GetWorld() const { return WorldContext ? WorldContext->GetWorld() : nullptr; }
	FWorldContext* GetWorldContext() const { return WorldContext; }
	void SetWorldContext(FWorldContext* InWorldContext) { WorldContext = InWorldContext; }

	// 복제 World의 실행 준비가 완료된 뒤 OnStart/Actor 초기화/BeginPlay를 연결할 자리다.
	// 필수 생명주기가 없는 지금은 성공을 공개하지 않는다(4단계 구현).
	bool StartPlayInEditorGameInstance() { return false; }


	// PIE World 연결 이후 호출합니다.
	// 향후 GameInstance 서브시스템을 초기화하며 현재 추가 작업은 없습니다.
	virtual void Init() {}

	// 게임 실행을 시작할 때, Actor 초기화와 BeginPlay 전에 호출합니다.
	virtual void OnStart() {}

	// World의 EndPlay 이후, CleanupWorld 전에 호출합니다.
	// 향후 LocalPlayer와 GameInstance 서브시스템을 정리합니다.
	virtual void Shutdown() {}

private:
	FWorldContext* WorldContext = nullptr;
};
