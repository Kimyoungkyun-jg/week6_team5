#include "EnginePCH.h"

#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Core/Stats/LightweightStats.h"

DECLARE_CYCLE_STAT("Register Component", STAT_RegisterComponent);

UActorComponent::~UActorComponent()
{
	if (bIsRegistered)
	{
		UnregisterComponent();
	}

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

void UActorComponent::RegisterComponent()
{
	AActor* MyOwner = GetOwner();
	UWorld* MyOwnerWorld = (MyOwner ? MyOwner->GetWorld() : nullptr);
	if (MyOwnerWorld)
	{
		RegisterComponentWithWorld(MyOwnerWorld);
	}
}

// 컴포넌트 월드 등록
void UActorComponent::RegisterComponentWithWorld(UWorld* InWorld)
{
	SCOPE_CYCLE_COUNTER(STAT_RegisterComponent);

	if (!InWorld)
	{
		HTR_LOG(Warning, "RegisterComponentWithWorld: InWorld is null");
		return;
	}

	if (bIsRegistered)
	{
		HTR_LOG(Warning, "RegisterComponentWithWorld: Component is already registered");
		return;
	}

	AActor* MyOwner = GetOwner();
	if (MyOwner && InWorld != MyOwner->GetWorld())
	{
		HTR_LOG(Warning, "RegisterComponentWithWorld: InWorld does not match Owner World");
	}

	if (!bHasBeenCreated)
	{
		OnComponentCreated();
		bHasBeenCreated = true;
	}

	World = InWorld;

	bIsRegistered = true;
	OnRegister();

	// 컴포넌트 틱 함수 등록
	if (World && PrimaryComponentTick.bCanEverTick)
	{
		PrimaryComponentTick.RegisterTickFunction(World->GetTickTaskManager());
	}

	InitializeComponent();
	if (bAutoActivate) Activate();
}

// 컴포넌트 월드 등록 해제
void UActorComponent::UnregisterComponent()
{
	if (!bIsRegistered)
	{
		return;
	}

	OnUnregister();

	// 컴포넌트 틱 함수 등록 해제
	if (PrimaryComponentTick.bCanEverTick)
	{
		PrimaryComponentTick.UnRegisterTickFunction();
	}

	World = nullptr;
	bIsRegistered = false;
}
void UActorComponent::SetComponentTickEnabled(bool bEnabled)
{
    if (!PrimaryComponentTick.bCanEverTick) return;
    // Registration uses this value; do not lose a pre-registration disable request.
    PrimaryComponentTick.bStartWithTickEnabled = bEnabled;
    PrimaryComponentTick.SetTickFunctionEnable(bEnabled);
}

void UActorComponent::Activate(bool bReset)
{
    if (!bReset && !ShouldActivate()) return;
    SetComponentTickEnabled(true);
    SetActiveFlag(true);
    OnComponentActivated.Broadcast(this, bReset);
}

void UActorComponent::Deactivate()
{
    if (ShouldActivate()) return;
    SetComponentTickEnabled(false);
    SetActiveFlag(false);
    OnComponentDeactivated.Broadcast(this);
}

void UActorComponent::SetAutoActivate(bool bNewAutoActivate)
{
    // This engine has no User Construction Script; allow configuration before registration.
    if (IsRegistered())
    {
        HTR_LOG(Warning, "SetAutoActivate: cannot change AutoActivate after component registration");
        return;
    }
    bAutoActivate = bNewAutoActivate;
}
