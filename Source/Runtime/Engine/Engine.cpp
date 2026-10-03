#include "EnginePCH.h"
#include "Engine.h"


#include "Core/Windows/WindowsPlatformTime.h"
#include "Core/EngineLog.h"
#include "ObjectSystem/ObjectFactory.h"

UEngine* GEngine = nullptr;

bool UEngine::Init()
{
	EditorWorld = FObjectFactory::ConstructObject<UWorld>();
	EditorWorld->GetWorldType() = EWorldType::Editor;

	if (!EditorWorld || !EditorWorld->Init())
	{
		return false;
	}
	
	// 에디터용 월드 컨텍스트 생성 및 등록
	FWorldContext& EditorContext = CreateNewWorldContext(EWorldType::Editor);
	EditorContext.SetCurrentWorld(EditorWorld);

	return true;
}

FWorldContext* UEngine::GetWorldContextFromPIEInstance(int32 PIEInstanceIndex)
{
	for (FWorldContext& WorldContext : WorldContextlist)
	{
		if (WorldContext.WorldType == EWorldType::PIE && WorldContext.PIEInstance == PIEInstanceIndex)
		{
			return &WorldContext;
		}
	}
	return nullptr;
}

UWorld* UEngine::CreatePIEWorldByDuplication(FWorldContext& WorldContext, UWorld* InWorld)
{
	// 시작 시간 기록
	double StartTime = FPlatformTime::Seconds();
	if (!InWorld)
	{
		HTR_LOG(Error, "InWorld is nullptr");
		return nullptr;
	}

	// 월드 생성 및 복제
	UWorld* NewPIEWorld = FObjectFactory::ConstructObject<UWorld>();
	NewPIEWorld->DuplicateWorld(InWorld);
	NewPIEWorld->GetWorldType() = EWorldType::PIE;

	// 월드 컨텍스트 갱신
	WorldContext.SetCurrentWorld(NewPIEWorld);
	WorldContext.WorldType = EWorldType::PIE;

	HTR_LOG(Info, "PIE: Created PIE world by copying editor world ({:.4f}s)", FPlatformTime::Seconds() - StartTime);
	return NewPIEWorld;
}

FWorldContext& UEngine::CreateNewWorldContext(EWorldType InWorldType)
{
	// 신규 컨텍스트 등록
	WorldContextlist.Add(FWorldContext(InWorldType));
	return WorldContextlist.Last();
}

