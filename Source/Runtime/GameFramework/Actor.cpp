#include "EnginePCH.h"
#include "Actor.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "ObjectSystem/ObjectFactory.h"
#include "Component/SceneComponent.h"
#include "Component/ActorComponent.h"
#include "Component/StaticMeshComponent.h"

AActor::AActor()
{
    PrimaryActorTick.Target = this;
}

AActor::~AActor()
{
    TArray<UActorComponent*> ToDelete = Components;
    Components.Reset();
    RootComponent = nullptr;

    for (UActorComponent* Component : ToDelete)
    {
        delete Component;
    }
}

void AActor::PreInitializeComponents()
{
	// 액터의 사전 정보를 초기화
}

void AActor::InitializeComponents()
{
	PreInitializeComponents();
    // 컴포넌트를 순회하면서 초기화
	for (UActorComponent* Component : Components)
	{
		Component->InitializeComponent();
	}
	PostInitializeComponents();
}

void AActor::PostInitializeComponents()
{
	// 포인터 등과 같은 의존성 초기화 및 검증, 물리 데이터 생성, 이벤트 바인딩
}

void AActor::BeginPlay()
{
	//if (UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(RootComponent))
	//{
	//	World->AddPrimitive(Cast<UPrimitiveComponent>(RootComponent));
	//}

	for (UActorComponent* Component : Components)
	{
		Component->BeginPlay();
	}

	RegisterAllActorTickFunctions(true);
}

void AActor::RegisterAllActorTickFunctions(bool bRegister)
{
	if (bRegister && !World)
		return;

	// bCanEverTick이 꺼진 함수는 등록하지 않으므로 정적 메시 액터는 매 프레임 순회 대상에서 빠진다.
	auto Apply = [&](FTickFunction& Function)
	{
		if (bRegister)
			Function.RegisterTickFunction(World->GetTickTaskManager());
		else
			Function.UnRegisterTickFunction();
	};

	Apply(PrimaryActorTick);
	for (UActorComponent* Component : Components)
	{
		if (Component)
			Apply(Component->PrimaryComponentTick);
	}
}

void AActor::RemoveOwnedComponent(UActorComponent* Component)
{
    for (uint32 i = 0; i < Components.Num(); ++i)
    {
        if (Components[i] == Component)
        {
            Components.RemoveAt(i, 1);
            break;
        }
    }

    if (RootComponent == Component)
    {
        RootComponent = nullptr;
    }
}

FVector AActor::GetActorLocation() const
{
    if (RootComponent)
    {
        return RootComponent->GetWorldLocation();
    }
    return FVector::ZeroVector;
}

//FRotator AActor::GetActorRotation() const
//{
//    if (RootComponent)
//    {
//        // USceneComponent의 GetWorldRotation() 호출
//        return RootComponent->GetWorldRotation();
//    }
//    return FRotator::ZeroRotator;
//}

FVector AActor::GetActorScale3D() const
{
    if (RootComponent)
    {
        return RootComponent->GetWorldScale3D();
    }
    return FVector::OneVector;
}

//FQuat AActor::GetActorQuat() const
//{
//    if (RootComponent)
//    {
//        return FQuat(RootComponent->GetWorldRotation());
//    }
//    return FQuat::Identity;
//}

FTransform AActor::GetActorTransform() const
{
    if (RootComponent)
    {
        return FTransform(
            RootComponent->GetWorldRotation(),
            RootComponent->GetWorldLocation(),
            RootComponent->GetWorldScale3D()
        );

        // return FTransform(RootComponent->GetWorldMatrix());
    }
    return FTransform::Identity;
}

bool AActor::Destroy()
{
    if (!World)
        return false;

    return World->DestroyActor(this);
}

void AActor::DuplicateSubobjects(UObject* SourceObject)
{
	Super::DuplicateSubobjects(SourceObject);

	AActor* SourceActor = Cast<AActor>(SourceObject);
    if (!SourceActor) return;

	// 복제할 액터의 컴포넌트들을 순회하며 이름이 같은 컴포넌트를 찾아서 복제한다.
    for (UActorComponent* SrcComp : SourceActor->GetComponents())
    {
		if (!SrcComp) continue;

        UActorComponent* DstComp = nullptr;
		// 이름이 같은 컴포넌트를 찾아서 DstComp에 할당
        for(UActorComponent* Comp : Components)
        {
            if (Comp && Comp->GetFName() == SrcComp->GetFName())
            {
                DstComp = Comp;
                break;
            }
        }

		// 이름이 같은 컴포넌트가 존재하면 속성을 복제한다.
        if(DstComp)
        {
			CopyProperties(SrcComp, DstComp);
        }
        else
        {
			DstComp = Cast<UActorComponent>(SrcComp->Duplicate(this));
            if (DstComp)
            {
                DstComp->SetOwner(this);
                Components.Add(DstComp);
            }
        }

		// 스태틱 메시 컴포넌트의 경우, 원본과 동일한 메시를 사용하도록 설정
        if(UStaticMeshComponent* SrcMesh = Cast<UStaticMeshComponent>(SrcComp))
        {
            if (UStaticMeshComponent* DstMesh = Cast<UStaticMeshComponent>(DstComp))
            {
				DstMesh->SetStaticMesh(SrcMesh->GetStaticMesh());
            }
        }
        else if (USceneComponent* DstScene = Cast<USceneComponent>(DstComp))
        {
			DstScene->MarkTransformDirty();
        }
    }

    if (SourceActor->GetRootComponent())
    {
        for (UActorComponent* Comp : Components)
        {
            if (Comp && Comp->GetFName() == SourceActor->GetRootComponent()->GetFName())
            {
                RootComponent = Cast<USceneComponent>(Comp);
                break;
            }
        }
    }
}