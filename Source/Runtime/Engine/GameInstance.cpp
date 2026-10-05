#include "EnginePCH.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "ObjectSystem/ObjectFactory.h"

void UGameInstance::Init()
{
	// 세션 초기화 작업 수행
}

void UGameInstance::Shutdown()
{
	// 세션 정리 작업 수행
	if (WorldContext)
	{
		// 게임 뷰포트 클라이언트 해제
		if (WorldContext->GameViewport)
		{
			WorldContext->GameViewport->Reset();
			WorldContext->GameViewport.reset();
		}
		WorldContext->OwningGameInstance = nullptr;
		WorldContext = nullptr;
	}
}

UWorld* UGameInstance::GetWorld() const
{
	return WorldContext ? WorldContext->World() : nullptr;
}

void UGameInstance::SetWorldContext(FWorldContext* InWorldContext)
{
	WorldContext = InWorldContext;
	if (WorldContext)
	{
		WorldContext->OwningGameInstance = this;
	}
}

void UGameInstance::InitializeForPlayInEditor(int32 PIEInstanceIndex)
{
	UEngine* Engine = GetEngine();
	if (!Engine)
	{
		return;
	}

	// 기존 컨텍스트 검색 또는 신규 생성
	WorldContext = Engine->GetWorldContextFromPIEInstance(PIEInstanceIndex); 
	if (!WorldContext)
	{
		return;
	}
	WorldContext->OwningGameInstance = this;

	// 에디터 월드 참조
	UWorld* EditorWorld = Engine->GetEditorWorld();
	if (!EditorWorld)
	{
		return;
	}

	// 에디터 월드 복제
	Engine->CreatePIEWorldByDuplication(*WorldContext, EditorWorld);
	
	// 세션 초기화
	Init();
}

bool UGameInstance::StartPlayInEditorGameInstance()
{
	UWorld* PlayWorld = GetWorld();
	
	if (!PlayWorld)
	{
		HTR_LOG(Error, "StartPlayInEditorGameInstance: PlayWorld is nullptr");
		return false;
	}
	
	// 엔진의 활성 플레이 월드 설정
	if (UEngine* Engine = GetEngine())
	{
		Engine->SetPlayWorld(*PlayWorld);
	}

	// 게임 뷰포트 클라이언트 생성 및 초기화
	if (WorldContext)
	{
		WorldContext->GameViewport = MakeShared<FGameViewportClient>();
		WorldContext->GameViewport->Init(*WorldContext, GetEngine());
	}

	// 월드 재생 시작
	PlayWorld->BeginPlay();
	HTR_LOG(Info, "PIE: Game instance started successfully");
	return true;
}

UEngine* UGameInstance::GetEngine() const
{
	return GEngine;
}
