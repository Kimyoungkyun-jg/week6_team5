#include "EnginePCH.h"
#include "Engine.h"

UEngine* GEngine = nullptr;

bool UEngine::Init()
{
	World = CreateNewWorldContext(EWorldType::WorldType_Editor).World;
	if(World == nullptr)
		return false;
	return true;
}

FWorldContext& UEngine::CreateNewWorldContext(EWorldType WorldType)
{
	for (FWorldContext& Context : WorldContexts)
	{
		if (Context.WorldType == WorldType)
		{
			return Context;
		}
	}
	FWorldContext NewContext;
	NewContext.WorldType = WorldType;
	NewContext.World = FObjectFactory::ConstructObject<UWorld>();
	if (NewContext.World)
	{
		NewContext.World->Init();
		NewContext.World->SetWorldType(WorldType);
	}
	WorldContexts.Add(NewContext);
	return WorldContexts.Last();
}

FWorldContext* UEngine::GetWorldContextFromType(EWorldType WorldType) const
{
	for (const FWorldContext& Context : WorldContexts)
	{
		if (Context.WorldType == WorldType)
		{
			return const_cast<FWorldContext*>(&Context);
		}
	}
	return nullptr;
}

void UEngine::DestroyWorldContext(EWorldType WorldType)
{
	for (int32 i = 0; i < WorldContexts.Num(); ++i)
	{
		if (WorldContexts[i].WorldType == WorldType)
		{
			if (WorldContexts[i].World)
			{
				WorldContexts[i].World->ClearWorld();
				delete WorldContexts[i].World;
				WorldContexts[i].World = nullptr;
			}
			WorldContexts.RemoveAtSwap(i);
			return;
		}
	}
}