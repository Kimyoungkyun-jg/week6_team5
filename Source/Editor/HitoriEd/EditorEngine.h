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
#include "Editor/LevelEditor/MultipleViewports/Adapter/MultipleViewportsAdapter.h"
#include "Editor/ContentDrawer/ContentDrawerPanel.h"

#include "Editor/Rendering/Outline.h"
#include "Editor/Rendering/OutLineRenderer.h"
#include "Editor/Outliner/OutlinerPanel.h"

#include "Render/SkyboxRenderer.h"

//Temp
#include "Text/Font.h"
#include "Text/TextRenderer.h"

enum class EPIESessionState { Stopped, Starting, Running, Paused, Stopping };

class UEditorEngine : public UEngine
{
	DECLARE_CLASS(UEditorEngine, UEngine)

public:
	FEngineConfig GetConfig() const override;
	bool Init() override;
	void Tick(float DeltaTime) override;
	void PreExit() override;

	UWorld* CreatePIEWorldByDuplication();
	// true는 요청 접수 의미다. 실제 시작 결과는 다음 Tick에서 결정된다.
	bool RequestPlaySession(int32 ViewportSlot);
	void RequestEndPlayMap();
	EPIESessionState GetPIESessionState() const { return PIEState; }
	bool SetPIEWorldPaused(bool bPaused);
	void StopPlayInEditorSession();

	// Active View의 입력과 Picking 결과만 Gizmo 및 선택 상태에 반영한다.
	void UpdateGizmoAndPicking();
	// View 하나의 Scene·Grid·Gizmo·텍스트를 해당 ViewProjection으로 렌더한다.
	void RenderFrame(int32 ViewIndex, const FRenderingInfo& ViewRenderingInfo, const FMatrix& ViewProjection, const FVector& ViewCameraLocation, const FVector& ViewCameraForward, FRenderQueue& RenderQueue);
	// 네 View 결과와 ImGui를 메인 Swapchain 백버퍼에 합성한다. Present는 FEngineLoop가 한다.
	void PresentFrame();
	void DeleteActor(AActor* Actor);

	UWorld* GetEditorWorld() const { return EditorWorld; }
	UWorld* GetPlayWorld() const { return PlayWorld; }

private:
	void ProcessPlaySessionRequest();
	bool StartPlayInEditorSession(int32 ViewportSlot);
	void ReleasePIEContext();
	int32 PendingPlayViewportSlot = -1;
	bool bPendingEndPlay = false;
	bool bPendingPause = false;
	bool bRequestedPause = false;
	bool bGameInstanceInitialized = false;
	EPIESessionState PIEState = EPIESessionState::Stopped;
	FWorldContext* PIEContext = nullptr; // UEngine::WorldList 소유
	UGameInstance* PlayGameInstance = nullptr; // 명시적으로 생성/삭제한다. Outer는 소유권이 아니다.

	// 이번 프레임 DeltaTime을 패널에 전달하고 에디터 단축키를 처리한다.
	void BeginFrame(float DeltaTime);
	// 패널 요청과 입력을 Core Adapter에 전달해 레이아웃·카메라 상태를 갱신한다.
	void UpdateMultipleViewportState(float DeltaTime);
	// 월드를 정확히 한 번 Tick·Capture한 뒤 에디터 상호작용을 갱신한다.
	void TickWorldAndEditor(float DeltaTime);
	// 한 번 캡처한 월드 결과를 재사용해 현재 레이아웃의 각 View를 렌더한다.
	void RenderMultipleViewports();
	// 화면 합성과 View 설정 보관으로 프레임을 마무리한다.
	void EndFrame();

	// FEngineLoop 소유. OnInit에서 받아 둔다.
	FWindow* MainWindow = nullptr;
	FSwapchain* MainWindowSC = nullptr;
	FRenderer* Renderer = nullptr;

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

	UFont* SystemFont;

	FOutputLogPanel* OutputLogPanel = nullptr;

	FDetailsPanel* DetailsPanel = nullptr;
	FEditorControlsPanel* EditorControlsPanel = nullptr;
	FSettingsPanel* SettingsPanel = nullptr;
	FViewportsPanel* ViewportsPanel = nullptr;
	FMultipleViewportsAdapter MultipleViewportsAdapter;
	FRenderQueue RenderQueue;
	FOutlinerPanel* OutlinerPanel = nullptr;
	FContentDrawerPanel* ContentDrawerPanel = nullptr;


	UWorld* EditorWorld = nullptr;
	UWorld* PlayWorld = nullptr;


	void ResetSceneSelection();

	void CreateNewScene();
	void OpenScene();
	void SaveCurrentScene();
	void SaveSceneAs();
};
