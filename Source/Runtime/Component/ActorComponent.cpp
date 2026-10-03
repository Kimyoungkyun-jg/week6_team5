#include "EnginePCH.h"

#include "GameFramework/Actor.h"

UActorComponent::~UActorComponent()
{
    if (Owner)
    {
        Owner->RemoveOwnedComponent(this);
    }
}

void UActorComponent::InitializeComponent()
{
    if (bIsInitialized)
        return;

    bIsInitialized = true;
}