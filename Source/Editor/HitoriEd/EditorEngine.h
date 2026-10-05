#pragma once

#include "Engine/Engine.h"
#include "Core/Window.h"
#include "Core/Types.h"

#include "Render/Renderer.h"
#include "Render/RenderDevice.h"
#include "Render/Swapchain.h"
#include "Editor/EditorUI/ImGuiRenderer.h"
#include "Editor/Rendering/GridRenderer.h"
#include "Editor/Gizmo/GizmoRenderer.h"
#include "Render/LineBatcher.h"

#include "Editor/EditorUI/EditorUI.h"
#include "Editor/OutputLog/OutputLogPanel.h"
#include "Editor/Details/DetailsPanel.h"
#include "Editor/EditorControls/EditorControlsPanel.h"
#include "Editor/Settings/SettingsPanel.h"
#include "Editor/Viewports/ViewportsPanel.h"
#include "Editor/Viewports/ViewportLayout.h"
#include "Editor/ContentDrawer/ContentDrawerPanel.h"

#include "Editor/Rendering/Outline.h"
#include "Editor/Rendering/OutLineRenderer.h"
#include "Editor/Outliner/OutlinerPanel.h"

#include "Editor/Rendering/SceneDepthRenderer.h"
#include "Editor/Rendering/HeightFogRenderer.h"

#include "Render/SkyboxRenderer.h"

#include "Serialization/JsonArchive.h"
//Temp
#include "Text/Font.h"
#include "Text/TextRenderer.h"
#include <Editor\Viewports\EditorViewportClient.h>
#include <Container\Array.h>

class FSceneRenderer;

class UEditorEngine : public UEngine
{
	DECLARE_CLASS(UEditorEngine, UEngine)

public:
	FEngineConfig GetConfig() const override;
	bool Init() override;
	void Tick(float DeltaTime) override;
	void PreExit() override;

	// Active View의 입력과 Picking 결과만 Gizmo 및 선택 상태에 반영한다.
	void UpdateGizmoAndPicking();
	// 뷰포트 하나의 씬과 에디터 요소를 렌더링한다.
	void RenderFrame(FEditorViewportClient* ViewClient, const FSceneView& SceneView, FSceneRenderer& SceneRenderer, const bool bIsPIE);


	// 네 View 결과와 ImGui를 메인 Swapchain 백버퍼에 합성한다. Present는 FEngineLoop가 한다.
	void PresentFrame();
	void DeleteActor(AActor* Actor);

	EPIEState GetPIEState() const { return PIEState; }

private:
	// 이번 프레임 DeltaTime을 패널에 전달하고 에디터 단축키를 처리한다.
	void BeginFrame(float DeltaTime);
	// 패널 요청과 입력을 Core Adapter에 전달해 레이아웃·카메라 상태를 갱신한다.
	void UpdateViewportState(float DeltaTime);
	// 월드를 정확히 한 번 Tick·Capture한 뒤 에디터 상호작용을 갱신한다.
	void TickWorld(float DeltaTime);
	// 한 번 캡처한 월드 결과를 재사용해 현재 레이아웃의 각 View를 렌더한다.
	void RenderViewports();
	// 화면 합성과 View 설정 보관으로 프레임을 마무리한다.
	void EndFrame();

	


	// FEngineLoop 소유. OnInit에서 받아 둔다.
	FWindow* MainWindow = nullptr;
	FSwapchain* MainWindowSC = nullptr;
	FRenderer* Renderer = nullptr;

	TMap<UObject*, UObject*> OriginNewAnnotataion;
	TMap<ULevel*,json> LeveljsonMap;
	TMap<AActor*, json> ActorjsonMap;
	TMap<UActorComponent*, json> ActorCompjsonMap;

	TUniquePtr<FEditorUI> EditorUI;

	TUniquePtr<FImGuiRenderer> ImGuiRenderer;
	TUniquePtr<FGridRenderer> GridRenderer;
	TUniquePtr<FGizmoRenderer> GizmoRenderer;
	TUniquePtr<FTextRenderer> TextRenderer;
	TUniquePtr<FLineBatcher> LineBatcher;
	TUniquePtr<FGizmo> Gizmo;
	TUniquePtr<FOutline> Outline;
	TUniquePtr<FOutlineRenderer> OutlineRenderer;
	TUniquePtr<FSkyboxRenderer> SkyboxRenderer;
	TUniquePtr<FSceneDepthRenderer> SceneDepthRenderer;
	TUniquePtr<FHeightFogRenderer> HeightFogRenderer;

	UFont* SystemFont;

	FOutputLogPanel* OutputLogPanel = nullptr;

	FDetailsPanel* DetailsPanel = nullptr;
	FEditorControlsPanel* EditorControlsPanel = nullptr;
	FSettingsPanel* SettingsPanel = nullptr;
	FViewportsPanel* ViewportsPanel = nullptr;
	FSplitRatio ViewportSplitRatio{0.5f, 0.5f};
	ELayoutMode ViewportLayoutMode = ELayoutMode::QuadSplit;
	int32 SingleViewportIndex = 0;
	FRenderQueue RenderQueue;
	FOutlinerPanel* OutlinerPanel = nullptr;
	FContentDrawerPanel* ContentDrawerPanel = nullptr;

	TArray<FEditorViewportClient*> AllViewportClients;

	UGameInstance* GameInstance = nullptr;

	EPIEState PIEState = EPIEState::Stopped;
	bool bIsStep = false;

	void ResetSceneSelection();

	void CreateNewScene();
	void OpenScene();
	void SaveCurrentScene();
	void SaveSceneAs();

	// 액터 머리 위에 UUID 라벨 렌더링
	void RenderActorUUIDs(const FSceneView& View);
	void CreatePIESession();
	void StopPIESession();
	UWorld* CreatePIEWorld();
	virtual UWorld* CreatePIEWorldByDuplication(FWorldContext& WorldContext, UWorld* InWorld) override;
	void SerializeWorldForPIE(UWorld* editorWorld, UWorld* PIEWorld);
	UWorld* RecoverPIEWorldReferences(UWorld* editorWorlds, UWorld* PIEWorld);
};
