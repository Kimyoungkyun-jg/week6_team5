#pragma once

#include "Engine/ViewportClient.h"
#include "Math/Vector.h"
#include "Math/Rotator.h"
#include "Math/Matrix.h"
#include "Render/SceneView.h"

class UWorld;
class UGameInstance;
class UEngine;
class UCameraComponent;

struct FWorldContext;

// 게임 뷰포트 클라이언트 클래스
class FGameViewportClient : public FViewportClient
{
public:
	FGameViewportClient();
	virtual ~FGameViewportClient() override = default;

	// 월드 및 게임 인스턴스 초기화
	void Init(FWorldContext& InWorldContext, UEngine* InEngine);

	virtual void Draw(FViewport* Viewport) override;
	virtual void Tick(float DeltaTime) override;
	virtual UWorld* GetWorld() const override;

	virtual void MouseMove(FViewport* Viewport, int32 X, int32 Y) override;
	virtual bool InputKey(FViewport* Viewport, int32 Key, bool bDown) override;
	virtual bool InputAxis(FViewport* Viewport, int32 ControllerId, float Delta, float DeltaTime) override;

	// 게임 카메라 시점 계산
	FSceneView CalcSceneView(const FRect& InViewRect);

	void SetGameInstance(UGameInstance* InGameInstance) { GameInstance = InGameInstance; }
	UGameInstance* GetGameInstance() const { return GameInstance; }

	void SetWorld(UWorld* InWorld) { World = InWorld; }

	void SetCameraComponent(UCameraComponent& CameraComponent);

	void Reset();
private:
	UWorld* World = nullptr;
	UGameInstance* GameInstance = nullptr;
	UEngine* Engine = nullptr;
	UCameraComponent* CameraComponent = nullptr;
};
