#include "EnginePCH.h"
#include "Engine.h"

UEngine* GEngine = nullptr;

bool UEngine::Init()
{
	EditorWorld = FObjectFactory::ConstructObject<UWorld>();
	
	if (!EditorWorld || !EditorWorld->Init()) return false;

 	return true;
}
