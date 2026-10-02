#include "EnginePCH.h"
#include "Engine.h"

UEngine* GEngine = nullptr;

bool UEngine::Init()
{
	WorldEditor = FObjectFactory::ConstructObject<UWorld>();
	
	if (!WorldEditor || !WorldEditor->Init(EWorldType::WorldEditor)) return false;

 	return true;
}
