#pragma once

#include "ObjectSystem/Object.h"
#include "Component/PrimitiveComponent.h"

#include "ObjectSystem/Class.h"
#include "ObjectSystem/ObjectFactory.h"
#include "../Container/Set.h"
#include "Engine/EngineBaseTypes.h"

class UWorld;
class ULevel;

class AActor : public UObject
{
	DECLARE_CLASS(AActor, UObject)
	REFLECT_START(ClassName)
		REFLECT_END()
public:
	AActor();
	virtual ~AActor();

	// 컴포넌트들 복제 직후 필요한 값 초기화(액터의 무게 값 등)
	virtual void PreInitializeComponents();
	// 컴포넌트들을 게임 월드와 물리 엔진에 등록(메쉬 등록, 물리 충돌용 콜리전 등)
	virtual void InitializeComponents();
	// 최종적으로 컴포넌트 조립 및 검증
	virtual void PostInitializeComponents();

	virtual void BeginPlay(); // xx World->AddPrimitive 책임이동 필요
	// 액터 자신의 로직. 컴포넌트는 각자의 PrimaryComponentTick으로 따로 실행된다.
	virtual void Tick(float DeltaTime) {}
	// FActorTickFunction이 호출하는 진입점
	void TickActor(float DeltaTime) { Tick(DeltaTime); }

	UWorld* GetWorld() const { return World; }
	ULevel* GetLevel() const { return Level; }

	const TArray<UActorComponent*>& GetComponents() const { return Components; }
	USceneComponent* GetRootComponent() const { return RootComponent; }
	void SetRootComponent(USceneComponent* SceneComponent) { RootComponent = SceneComponent; }

	void RemoveOwnedComponent(UActorComponent* Component);
	
	FVector GetActorLocation() const;
	FRotator GetActorRotation() const;
	FVector GetActorScale3D() const;
	FQuat GetActorQuat() const;           //xx 타입명변경
	FTransform GetActorTransform() const;

	bool Destroy();
	
	//xx삭제 예정
	friend class UWorld;

	template <typename T>
	T* CreateDefaultSubobject(FName Name)
	{
		T* Component = CastChecked<T>(FObjectFactory::ConstructObject(T::StaticClass(), this, Name));
		Component->SetOwner(this);
		Components.Add(Component);
		return Component;
	}

	// bCanEverTick이 켜진 액터·컴포넌트의 Tick 함수만 World의 FTickTaskManager에 등록하거나 해제한다.
	void RegisterAllActorTickFunctions(bool bRegister);

	// UE와 같이 기본값은 bCanEverTick = false. 생성자에서 Target = this
	FActorTickFunction PrimaryActorTick;

	virtual void DuplicateSubobjects(UObject* SourceObject) override;
protected:
	//TSet<TObjectPtr<UActorComponent>> OwnedComponents;
	TArray<UActorComponent*> Components;
	USceneComponent* RootComponent = nullptr;

	UWorld* World = nullptr;
	ULevel* Level = nullptr;

private:

	

};