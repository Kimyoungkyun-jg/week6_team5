#pragma once

#include "Engine/EngineBaseTypes.h"
#include "Core/Delegate.h"
#include "ObjectSystem/Class.h"
#include "ObjectSystem/Object.h"


class AActor;
class UWorld;

class UActorComponent : public UObject {
  DECLARE_CLASS(UActorComponent, UObject)

  REFLECT_START(ClassName)
    PROPERTY(bAutoActivate)
  REFLECT_END()

public:
  UActorComponent() { PrimaryComponentTick.Target = this; }
  virtual ~UActorComponent() override;

  virtual void InitializeComponent();

  virtual void BeginPlay() {};
  virtual void TickComponent(float DeltaTime) {};

  virtual void Activate(bool bReset = false);
  virtual void Deactivate();
  virtual bool ShouldActivate() const { return !bIsActive; }

  bool IsActive() const { return bIsActive; }
  void SetAutoActivate(bool bNewAutoActivate);
  bool IsAutoActivate() const { return bAutoActivate; }

  void SetComponentTickEnabled(bool bEnabled);
  bool IsComponentTickEnabled() const
  { return PrimaryComponentTick.bCanEverTick && PrimaryComponentTick.IsTickFunctionEnabled(); }

  TMulticastDelegate<UActorComponent*, bool> OnComponentActivated;
  TMulticastDelegate<UActorComponent*> OnComponentDeactivated;

  // 컴포넌트 등록 및 해제
  void RegisterComponent();
  void RegisterComponentWithWorld(UWorld *InWorld);
  void UnregisterComponent();

  virtual void OnComponentCreated() {}
  virtual void OnRegister() {}
  virtual void OnUnregister() {}

  bool IsRegistered() const { return bIsRegistered; }
  UWorld *GetWorld() const { return World; }

  void SetOwner(AActor *InOwner) { Owner = InOwner; }
  AActor *GetOwner() const { return Owner; }

  // UE와 같이 기본값은 bCanEverTick = false. Tick이 필요한 컴포넌트만
  // 생성자에서 켠다.
  FActorComponentTickFunction PrimaryComponentTick;

protected:
  void SetActiveFlag(bool bNewActive) { bIsActive = bNewActive; }
  bool bAutoActivate = true;
  bool bIsActive = false;

  // 소속 월드
  UWorld *World = nullptr;
  // 등록 플래그
  bool bIsRegistered = false;
  // 생성 플래그
  bool bHasBeenCreated = false;

private:
  AActor *Owner = nullptr;
  bool bIsInitialized = false;
};