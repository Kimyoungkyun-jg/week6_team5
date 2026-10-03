#pragma once

class UWorld;
class UGameInstance;
class UObject;

enum class EWorldType
{
	WorldEditor,
	WorldPIE
};

class FWorldContext
{
public:
	explicit FWorldContext(EWorldType InWorldType) : WorldType(InWorldType) {}
	EWorldType GetWorldType() const { return WorldType; }
	// 현재는 단일 PIE 세션이므로 0을 재사용한다. Editor는 -1이다.
	int GetPIEInstance() const { return WorldType == EWorldType::WorldPIE ? 0 : -1; }
	void SetCurrentWorld(UWorld* InWorld) { World = InWorld; }
	UWorld* GetWorld() const { return World; }
	void SetGameInstance(UGameInstance* InGameInstance) { OwningGameInstance = InGameInstance; }
	UGameInstance* GetGameInstance() const { return OwningGameInstance; }

	// 실제 Viewport 바인딩은 후속 단계에서 수행한다. Runtime은 Editor 패널을 알지 않는다.
	void SetViewportTarget(UObject* InHost, int InSlot) { ViewportHost = InHost; ViewportSlot = InSlot; }
	UObject* GetViewportHost() const { return ViewportHost; }
	int GetViewportSlot() const { return ViewportSlot; }

private:
	EWorldType WorldType;
	UWorld* World = nullptr;
	UGameInstance* OwningGameInstance = nullptr;
	UObject* ViewportHost = nullptr;
	int ViewportSlot = -1;
};
