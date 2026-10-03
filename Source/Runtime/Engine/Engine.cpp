#include "EnginePCH.h"
#include "Engine.h"
#include "Engine/ScopedConditionalWorldSwitcher.h"

UEngine* GEngine = nullptr;
UWorld* GWorld = nullptr;

bool UEngine::Init()
{
	World = FObjectFactory::ConstructObject<UWorld>();
	
	if (!World || !World->Init()) return false;

	GWorld = World;
	return true;
}

void UEngine::PreExit()
{
	// 이후 FEngineLoop가 UObject들을 삭제하므로 비소유 참조를 먼저 해제한다.
	GWorld = nullptr;
	WorldList.Reset();
	World = nullptr;
}


FWorldContext& UEngine::CreateNewWorldContext(EWorldType InWorldType)
{
	WorldList.Add(MakeUnique<FWorldContext>(InWorldType));

	return *WorldList.Last();
}
void UEngine::DestroyWorldContext(FWorldContext& TargetContext)
{
	for (uint32 Index = 0; Index < WorldList.Num(); ++Index)
	{
		if (WorldList[Index].get() == &TargetContext)
		{
			WorldList.RemoveAt(Index, 1);
			return;
		}
	}
}

FWorldContext* UEngine::GetWorldContextFromWorld(const UWorld* InWorld) const
{
	if (!InWorld) return nullptr;
	for (const auto& Context : WorldList)
		if (Context->GetWorld() == InWorld) return Context.get();
	return nullptr;
}
