#include "EnginePCH.h"

#include "Editor/HitoriEd/EditorEngine.h"

#include "Core/EngineStatics.h"
#include "Core/EngineTimer.h"
#include "Core/StatOverlay.h"
#include "Input/InputSystem.h"
#include "Launch/LaunchEngineLoop.h"


#include "ObjectSystem/ObjectFactory.h"

#include "Render/GeometryGenerator.h"

#include "Engine/Level.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"


#include "Render/Renderer.h"

#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Actor/LightActor.h"

#include "Asset/AssetManager.h"
#include "Render/RenderResourceManager.h"
#include "Render/SceneRenderer.h"

#include "Editor/HitoriEd/EditorFileUtils.h"
#include "Editor/Outliner/OutlinerPanel.h"
#include "Render/RenderCommand.h"
#include "UObject/UObjectIterator.h"


#include "Core/EngineLog.h"
#include "Core/Stats/LightweightStats.h"

namespace {
DECLARE_CYCLE_STAT("Viewport Update", STAT_ViewportUpdate);
DECLARE_CYCLE_STAT("World Tick", STAT_WorldTick);
DECLARE_CYCLE_STAT("Editor Tick", STAT_EditorTick);
DECLARE_CYCLE_STAT("Capture World", STAT_CaptureWorld);
DECLARE_CYCLE_STAT("Build Render Queue", STAT_BuildRenderQueue);
DECLARE_CYCLE_STAT("ImGui", STAT_ImGui);
} // namespace

#include "Core/SplashScreen.h"

FEngineConfig UEditorEngine::GetConfig() const {
	FEngineConfig Desc;
	Desc.Title = L"Hitori Engine";
	Desc.Width = 1920;
	Desc.Height = 1080;
	Desc.bBorderless = false;
	Desc.SyncInterval = 0;
	// View는 각자 깊이 버퍼를 쓰고 백버퍼에는 ImGui만 그린다.
	Desc.bCreateDepthBuffer = false;
	Desc.bExitOnEscape = false;
	// 파일이 없으면 검은 배경에 상태 텍스트만 표시된다.
	Desc.SplashImage = "Resources/Splash.png";
	return Desc;
}

// 렌더 자원·월드·에디터와 MultipleViewports 연결을 초기화한다.
// Device·Window·Swapchain·AssetManager는 FEngineLoop가 먼저 만들어 둔다.
bool UEditorEngine::Init() {
	if (!Super::Init())
		return false;

	MainWindow = GetEngineLoop().GetMainWindow();
	MainWindowSC = GetEngineLoop().GetSwapchain();
	Renderer = GetEngineLoop().GetRenderer();
	FRenderDevice *RenderDevice = GetEngineLoop().GetRenderDevice();

	EditorUI = MakeUnique<FEditorUI>();
	EditorUI->Init();

	EditorUI->SetNewSceneCallback([this]() { CreateNewScene(); });
	EditorUI->SetOpenSceneCallback([this]() { OpenScene(); });
	EditorUI->SetSaveSceneCallback([this]() { SaveCurrentScene(); });
	EditorUI->SetSaveSceneAsCallback([this]() { SaveSceneAs(); });
	EditorUI->SetCreatePIECallback([this]() { CreatePIESession(); });
	EditorUI->SetStopPIECallback([this]() { StopPIESession(); });
	EditorUI->SetPIEStateGetter([this]() { return PIEState; });
	EditorUI->SetPIEActionCallback([this](EPIEAction Action) {
		switch (Action) {
		case EPIEAction::Play:
			CreatePIESession();
			break;
		case EPIEAction::Pause:
			if (PlayWorld) {
				PlayWorld->GetbIsTickEnable() = false;
				PIEState = EPIEState::Paused;
			}
			break;
		case EPIEAction::Resume:
			if (PlayWorld) {
				PlayWorld->GetbIsTickEnable() = true;
				PIEState = EPIEState::Playing;
			}
			break;
		case EPIEAction::Step:
			if (PlayWorld) {
				bIsStep = true;
				PlayWorld->GetbIsTickEnable() = true;
			}
			break;
		case EPIEAction::Stop:
			StopPIESession();
			break;
		default:
			break;
		}
	});

	OutputLogPanel = EditorUI->AddEditorPanel<FOutputLogPanel>();
	FLog::AddSink(OutputLogPanel);
	HTR_LOG(Info, "Editor Initialize...");

	HTR_LOG(Info, "Initialize ImGui...");
	ImGuiRenderer = MakeUnique<FImGuiRenderer>();
	if (!ImGuiRenderer->Init(MainWindow->GetHandle(), RenderDevice->GetDevice(),
			RenderDevice->GetContext())) {
		HTR_LOG(Error, "Failed To Initialize ImGui!");
	}
	HTR_LOG(Info, "Initialize ImGui Success!");

	GridRenderer = MakeUnique<FGridRenderer>();
	GridRenderer->Init(Renderer);

	GizmoRenderer = MakeUnique<FGizmoRenderer>();
	GizmoRenderer->Init(Renderer);

	Gizmo = MakeUnique<FGizmo>();

	// 필요한 Panel들 추가후 raw pointer 반환(소유권 = EditorUI)
	DetailsPanel = EditorUI->AddEditorPanel<FDetailsPanel>();
	EditorControlsPanel = EditorUI->AddEditorPanel<FEditorControlsPanel>();
	ViewportsPanel = EditorUI->AddEditorPanel<FViewportsPanel>();
	ViewportsPanel->SetPIEStateGetter([this]() { return PIEState; });
	ContentDrawerPanel = EditorUI->AddEditorPanel<FContentDrawerPanel>();

	// OutLine
	OutlineRenderer = MakeUnique<FOutlineRenderer>();
	OutlineRenderer->Init(Renderer);

	SettingsPanel = EditorUI->AddEditorPanel<FSettingsPanel>();

	Outline = MakeUnique<FOutline>();

	SystemFont = UAssetManager::GetAssetByPath<UFont>("Assets/Fonts/Pretendard.json");

	TextRenderer = MakeUnique<FTextRenderer>();
	TextRenderer->Init();

	// 화면 나눔 비율 설정 가져오기
	ViewportSplitRatio = {SettingsPanel->GetSettings().MultipleViewportsHorizontal,
			SettingsPanel->GetSettings().MultipleViewportsVertical};
	// SingleView에 사용할 인덱스 설정
	SingleViewportIndex =
			SettingsPanel->GetSettings().MultipleViewportsSingleViewIndex;
	// 뷰포트 레이아웃 설정
	ViewportLayoutMode = SettingsPanel->GetSettings().bMultipleViewportsSingle
			? ELayoutMode::Single
			: ELayoutMode::QuadSplit;
	EditorWorld->GetMainCamera()->GetCameraComponent()->SetExternalInputManaged(
			true);

	OutlinerPanel = EditorUI->AddEditorPanel<FOutlinerPanel>();
	OutlinerPanel->SetWorld(EditorWorld);
	OutlinerPanel->SetSelectionCallback([this](UPrimitiveComponent *Primitive) {
		Gizmo->SetTarget(Primitive);
		Outline->SetTarget(Primitive);
		DetailsPanel->SetTarget(Primitive);
	});

	OutlinerPanel->SetDeleteActorCallback(
			[this](AActor *Actor) { DeleteActor(Actor); });

	LineBatcher = MakeUnique<FLineBatcher>();
	LineBatcher->Init(Renderer, EditorWorld);

	DetailsPanel->SetWorld(EditorWorld);

	EditorControlsPanel->SetWorld(EditorWorld);
	EditorControlsPanel->SetGizmo(Gizmo.get());
	EditorControlsPanel->SetViewportsPanel(ViewportsPanel);

	SettingsPanel->SetWorld(EditorWorld);
	SettingsPanel->SetViewportsPanel(ViewportsPanel);
	ViewportsPanel->SetLayoutMode(ViewportLayoutMode, SingleViewportIndex);

	for (int32 i = 0; i < 4; ++i) {
		if (FEditorViewportClient *Client = ViewportsPanel->GetViewportClient(i)) {
			Client->SetWorld(EditorWorld);
			AllViewportClients.Add(Client);
		}
	}
	SettingsPanel->ApplyViewportSettings();

	SkyboxRenderer = MakeUnique<FSkyboxRenderer>();
	SkyboxRenderer->Init("Assets/SkySphere/Sky.jpg");

	return true;
}

// 프레임 시작·View 상태·월드 갱신 후 View별 오프스크린 렌더와 UI 합성을
// 진행한다. 입력·창 메시지는 FEngineLoop가 먼저 처리하고 Present는 호출 직후에
// 한다.
void UEditorEngine::Tick(const float DeltaTime) {
	BeginFrame(DeltaTime);
	UpdateViewportState(DeltaTime);

	TickWorld(DeltaTime);
	RenderViewports();
	EndFrame();
}

// DeltaTime을 패널에 전달하고 에디터 단축키를 처리한다.
void UEditorEngine::BeginFrame(const float DeltaTime) {
	FStatOverlay::Tick(DeltaTime);
	EditorControlsPanel->FEditorControlsPanel::DeltaTime = DeltaTime;

	if (!ImGui::GetIO().WantTextInput &&
			FInputSystem::IsKeyPressed(EKeyCode::Delete)) {
		DeleteActor(OutlinerPanel->GetSelectedActor());
	}
}

// 패널의 Layout·Preset 요청과 입력을 Adapter에 반영한다.
void UEditorEngine::UpdateViewportState(const float DeltaTime) {
	SCOPE_CYCLE_COUNTER(STAT_ViewportUpdate);

	const FVector2 ViewportSize = ViewportsPanel->GetContentSize();
	const FVector2 LocalMousePosition = ViewportsPanel->GetLocalMousePosition();

	ELayoutMode RequestedLayout{};
	int32 RequestedSingleViewIndex = SingleViewportIndex;

	if (ViewportsPanel->ConsumeLayoutRequest(RequestedLayout,
			RequestedSingleViewIndex)) {
		if (RequestedLayout == ELayoutMode::Single)
			SingleViewportIndex = RequestedSingleViewIndex;
		ViewportLayoutMode = RequestedLayout;

		FEditorSettings &Settings = SettingsPanel->GetMutableSettings();
		Settings.bMultipleViewportsSingle = RequestedLayout == ELayoutMode::Single;
		Settings.MultipleViewportsSingleViewIndex = RequestedSingleViewIndex;
	}

	// PIE 액션 처리
	switch (ViewportsPanel->ConsumePIEAction()) {
	case EPIEAction::Play:
		// 피아이이 세션 생성
		CreatePIESession();
		break;
	case EPIEAction::Pause:
		if (PlayWorld) {
			PlayWorld->GetbIsTickEnable() = false;
			PIEState = EPIEState::Paused;
		}
		break;
	case EPIEAction::Resume:
		if (PlayWorld) {
			PlayWorld->GetbIsTickEnable() = true;
			PIEState = EPIEState::Playing;
		}
		break;
	case EPIEAction::Step:
		if (PlayWorld) {
			bIsStep = true;
			PlayWorld->GetbIsTickEnable() = true;
		}
		break;
	case EPIEAction::Stop:
		// 피아이이 세션 정리
		StopPIESession();
		break;
	default:
		break;
	}

	const float HorizontalDrag = ViewportsPanel->ConsumeHorizontalDrag();
	const float VerticalDrag = ViewportsPanel->ConsumeVerticalDrag();
	if (HorizontalDrag != 0.0f)
		ViewportSplitRatio = ApplySplitterDrag(ViewportSplitRatio, EDragAxis::Horizontal,
			HorizontalDrag, ViewportSize, 0.1f);
	if (VerticalDrag != 0.0f)
		ViewportSplitRatio = ApplySplitterDrag(ViewportSplitRatio, EDragAxis::Vertical,
			VerticalDrag, ViewportSize, 0.1f);
	if (HorizontalDrag != 0.0f || VerticalDrag != 0.0f) {
		SettingsPanel->GetMutableSettings().MultipleViewportsHorizontal =
				ViewportSplitRatio.Horizontal;
		SettingsPanel->GetMutableSettings().MultipleViewportsVertical =
				ViewportSplitRatio.Vertical;
	}

	FRect ViewRects[4]{};
	if (ViewportLayoutMode == ELayoutMode::Single) {
		for (int32 i = 0; i < 4; ++i) {
			ViewRects[i] = (i == SingleViewportIndex)
			? FRect{0.0f, 0.0f, ViewportSize.X, ViewportSize.Y}
			: FRect{};
		}
	} else {
		ComputeViewRects(ViewportSplitRatio, ViewportSize, ViewRects);
	}

	for (int32 i = 0; i < 4; ++i) {
		const bool bSlotActive = (ViewportLayoutMode == ELayoutMode::Single)
			? (i == SingleViewportIndex)
			: true;
		ViewportsPanel->SetView(i, ViewRects[i], bSlotActive);
	}

	// 마우스 오버 감지
	int32 HoveredViewIndex = -1;
	if (ViewportsPanel->IsHovered()) {
		for (int32 i = 0; i < 4; ++i) {
			if (ViewRects[i].Width > 0.0f && ViewRects[i].Height > 0.0f &&
					LocalMousePosition.X >= ViewRects[i].X &&
					LocalMousePosition.X < ViewRects[i].X + ViewRects[i].Width &&
					LocalMousePosition.Y >= ViewRects[i].Y &&
					LocalMousePosition.Y < ViewRects[i].Y + ViewRects[i].Height) {
				HoveredViewIndex = i;
				break;
			}
		}
	}

	// 마우스 캡처 및 활성 뷰 결정
	static int32 CapturedViewportIndex = -1;
	if (FInputSystem::IsMouseReleased(EMouseButton::Right)) {
		CapturedViewportIndex = -1;
	}
	if (FInputSystem::IsMousePressed(EMouseButton::Right) && HoveredViewIndex != -1) {
		CapturedViewportIndex = HoveredViewIndex;
	}
	if (CapturedViewportIndex != -1) {
		ViewportsPanel->SetActiveViewIndex(CapturedViewportIndex);
	} else if (FInputSystem::IsMousePressed(EMouseButton::Left) && HoveredViewIndex != -1) {
		ViewportsPanel->SetActiveViewIndex(HoveredViewIndex);
	}

	const int32 WheelDelta = FInputSystem::GetWheelDelta();
	const float MoveSpeed = 10.0f * SettingsPanel->GetSettings().CameraSpeed;
	const float MouseSens = 0.2f * SettingsPanel->GetSettings().MouseSensitivity;

	// 뷰포트 입력 처리
	for (int32 i = 0; i < 4; ++i) {
		if (FEditorViewportClient *Client = ViewportsPanel->GetViewportClient(i)) {
			if (Client->IsActive()) {
				Client->TickInput(DeltaTime, i == CapturedViewportIndex, i == HoveredViewIndex, WheelDelta, MoveSpeed, MouseSens);
			}
		}
	}

	// 활성 뷰포트 시점 동기화
	const int32 ActiveIndex = ViewportsPanel->GetActiveViewIndex();
	if (FEditorViewportClient *ActiveClient = ViewportsPanel->GetViewportClient(ActiveIndex)) {
		if (ACameraActor *MainCam = EditorWorld->GetMainCamera()) {
			if (UCameraComponent *CamCom = MainCam->GetCameraComponent()) {
				CamCom->SetRelativeLocation(ActiveClient->GetViewLocation());
				CamCom->SetRelativeRotation(ActiveClient->GetViewRotation());
				CamCom->SetFieldOfView(ActiveClient->GetViewFOV());
				CamCom->SetOrthoWidth(ActiveClient->GetOrthoWidth());
				CamCom->SetIsOrthogonal(!ActiveClient->IsPerspective());
			}
		}
	}
}

// 월드를 한 번 Tick·Capture한 뒤 에디터와 피킹을 갱신한다.
void UEditorEngine::TickWorld(const float DeltaTime) {
	if (PlayWorld) {
		{
			SCOPE_CYCLE_COUNTER(STAT_WorldTick);

			// PIE 모드 월드 틱 순회
			bool bTicked = false;
			for (int32 WorldIdx = 0; WorldIdx < WorldContextlist.Num(); ++WorldIdx) {
				FWorldContext &Context = WorldContextlist[WorldIdx];
				if (Context.WorldType == EWorldType::PIE && Context.World()) {
					Context.World()->Tick(EWorldTick::All, DeltaTime);
					bTicked = true;
				}
			}
			if (!bTicked && PlayWorld) {
				PlayWorld->Tick(EWorldTick::All, DeltaTime);
			}

			// 단일 프레임 진행 후 일시정지 복구
			if (bIsStep) {
				for (int32 WorldIdx = 0; WorldIdx < WorldContextlist.Num(); ++WorldIdx) {
					FWorldContext &Context = WorldContextlist[WorldIdx];
					if (Context.WorldType == EWorldType::PIE && Context.World()) {
						Context.World()->GetbIsTickEnable() = false;
					}
				}
				if (PlayWorld) {
					PlayWorld->GetbIsTickEnable() = false;
				}
				bIsStep = false;
			}
		}
		{
			SCOPE_CYCLE_COUNTER(STAT_EditorTick);
			EditorUI->Tick(DeltaTime);
		}
	} 
	else 
	{
		{
			SCOPE_CYCLE_COUNTER(STAT_WorldTick);

			// 에디터 모드 월드 틱
			EditorWorld->Tick(EWorldTick::ViewportsOnly, DeltaTime);
		}
		{
			SCOPE_CYCLE_COUNTER(STAT_EditorTick);
			EditorUI->Tick(DeltaTime);
		}

		UpdateGizmoAndPicking();
	}
}

// 뷰포트 클라이언트를 순회하며 씬 렌더러를 통해 렌더링한다
void UEditorEngine::RenderViewports() {
	for (int32 ViewIndex = 0; ViewIndex < AllViewportClients.Num(); ++ViewIndex) {
		
		
		FEditorViewportClient *ViewClient = AllViewportClients[ViewIndex];
		if (!ViewClient || !ViewClient->IsActive())
			continue;

		const FRect &ViewRect = ViewClient->GetRect();
		if (ViewRect.Width <= 0.0f || ViewRect.Height <= 0.0f)
			continue;

		const bool bIsPIE = PlayWorld != nullptr;
		UWorld *CurrentWorld = bIsPIE ? PlayWorld : EditorWorld;

		// 뷰 사각형과 시점 정보 생성
		FSceneView SceneView;
		if (bIsPIE && ViewIndex == 0) {
			FGameViewportClient *GameClient = nullptr;
			for (int32 WorldIdx = 0; WorldIdx < WorldContextlist.Num(); ++WorldIdx) {
				if (WorldContextlist[WorldIdx].WorldType == EWorldType::PIE &&
						WorldContextlist[WorldIdx].GameViewport) {
					GameClient = WorldContextlist[WorldIdx].GameViewport.get();
					break;
				}
			}
			if (GameClient) {
				// 플레이 모드 게임 뷰포트 시점 계산
				SceneView = GameClient->CalcSceneView(ViewRect);
			} else {
				SceneView = ViewClient->CalcSceneView(ViewRect);
			}
		} else {
			SceneView = ViewClient->CalcSceneView(ViewRect);
		}

		// 씬 렌더러 생성
		FSceneRenderer SceneRenderer(CurrentWorld, SceneView);
		SceneRenderer.InitViews(Renderer);

		// 프레임 렌더링
		RenderFrame(ViewClient, SceneView, SceneRenderer, bIsPIE);
	}
}

// 화면을 표시하고 UI 변경 후 View 설정을 보관한다.
void UEditorEngine::EndFrame() {
	PresentFrame();
	// UI 변경 후 설정을 복사해 종료 시 카메라 수명에 의존하지 않는다.
	SettingsPanel->CaptureViewportSettings();
	// 프로파일러 반영
	FStatRegistry::EndFrame();
}

// 활성 뷰포트 클라이언트에 기즈모 및 피킹 처리를 위임한다
void UEditorEngine::UpdateGizmoAndPicking() {
	const int32 ViewIndex = ViewportsPanel->GetActiveViewIndex();
	if (ViewIndex < 0 || ViewIndex >= 4 || !ViewportsPanel->IsHovered())
		return;

	FEditorViewportClient *ViewClient =
			ViewportsPanel->GetViewportClient(ViewIndex);
	if (!ViewClient || !ViewClient->IsActive())
		return;

	const FVector2 LocalMousePosition = ViewportsPanel->GetLocalMousePosition();
	ViewClient->UpdateGizmoAndPicking(Gizmo.get(), LocalMousePosition, [this](AActor *SelectedActor) {
		OutlinerPanel->SelectActor(SelectedActor);
	});
}

// 뷰포트 하나의 씬과 에디터 요소를 렌더링한다
void UEditorEngine::RenderFrame(FEditorViewportClient *ViewClient,
			const FSceneView &SceneView,
			FSceneRenderer &SceneRenderer,
			const bool bIsPIE) {
  
	const int32 ViewIndex = SceneView.ViewIndex;
	FTexture2D *ColorTarget = ViewClient ? ViewClient->GetColorTarget() : nullptr;
	FTexture2D *DepthTarget = ViewClient ? ViewClient->GetDepthTarget() : nullptr;
	const uint32 Width = ViewClient
			? ViewClient->GetWidth()
			: static_cast<uint32>(SceneView.ViewRect.Width);
	const uint32 Height = ViewClient
			? ViewClient->GetHeight()
			: static_cast<uint32>(SceneView.ViewRect.Height);
	const FViewportSettings ViewportSetting{0, 0, Width, Height, 0.0f, 1.0f};

	RenderCommand::BeginRenderPass(ColorTarget, DepthTarget, Width, Height);

	// 라인 배처 렌더링
	if ((!bIsPIE || ViewIndex != 0) &&
			SettingsPanel->GetSettings().bDrawBatchLine) {
		LineBatcher->BeginFrame();

		if (SettingsPanel->GetSettings().bDrawBoundingBox) {
			LineBatcher->BuildVertexBuffer();
			EditorWorld->GetPathTracker().OnRender(LineBatcher.get());
		}

		if (Gizmo->GetTarget()) {
			if (ALightActor *LightActor =
			Cast<ALightActor>(Gizmo->GetTarget()->GetOwner())) {
				LightActor->GetSpotLightComponent()->DrawDebug(LineBatcher.get());
			}
		}

		LineBatcher->OnRender(SceneView.ViewProjectionMatrix);
	}

	const bool bDrawPrimitives =
			bIsPIE || SettingsPanel->GetSettings().bDrawPrimitives;

	// 스카이박스 렌더링
	SkyboxRenderer->OnRender(SceneView.ViewProjectionMatrix,
			SceneView.ViewLocation);

	// 불투명 메시 렌더링
	if (bDrawPrimitives) {
		SceneRenderer.RenderOpaque(Renderer);
	}

	// 에디터 그리드 렌더링
	if ((!bIsPIE || ViewIndex != 0) &&
			SettingsPanel->GetSettings().bDrawBatchLine) {
		EGridPlane GridPlane = EGridPlane::XY;
		if (ViewClient) {
			switch (ViewClient->GetViewportType()) {
			case ELevelViewportType::Top:
				GridPlane = EGridPlane::XY;
				break;
			case ELevelViewportType::Front:
				GridPlane = EGridPlane::YZ;
				break;
			case ELevelViewportType::Right:
				GridPlane = EGridPlane::XZ;
				break;
			default:
				GridPlane = EGridPlane::XY;
				break;
			}
		}

		if (SettingsPanel->GetSettings().bDrawPSGrid && SceneView.bIsPerspective) {
			GridRenderer->OnRenderPSGrid(
					SceneView.ViewProjectionMatrix, SceneView.ViewLocation,
					SettingsPanel->GetSettings(), ViewportSetting);
		} else {
			GridRenderer->OnRenderBatchGrid(
					SceneView.ViewProjectionMatrix, SceneView.ViewLocation,
					SceneView.ViewForward, GridPlane,
					static_cast<float>(SettingsPanel->GetSettings().GridSpacing),
					SceneView.bIsPerspective, ViewportSetting);
		}
	}

	// 반투명 메시 렌더링
	if (bDrawPrimitives) {
		SceneRenderer.RenderTranslucent(Renderer);
	}

	// 텍스트 컴포넌트 렌더링
	UWorld *TargetWorld = bIsPIE ? PlayWorld : EditorWorld;
	for (TObjectIterator<UTextRenderComponent> TextComponent; TextComponent;
			 ++TextComponent) {
		if (!TextComponent || !TextComponent->GetFont() ||
				!TextComponent->IsVisible()) {
			continue;
		}

		if (TextComponent->GetOwner() &&
				TextComponent->GetOwner()->GetWorld() != TargetWorld) {
			continue;
		}

		TextRenderer->OnRender(
				TextComponent->GetText(), TextComponent->GetWorldMatrix(),
				TextComponent->GetTextSize(), *TextComponent->GetFont(),
				SceneView.ViewProjectionMatrix);
	}

	// 씬 뎁스 렌더링
	if (ViewClient && ViewClient->IsSceneDepth()) {
		Renderer->RenderSceneDepthPass(SceneView, DepthTarget, ColorTarget, ViewClient->GetMaxRange());
	}

	// 에디터 오버레이 렌더링
	if (!bIsPIE || ViewIndex != 0) {
		// 활성 뷰포트에만 아웃라인과 기즈모 렌더링
		const bool bIsActiveViewport = (ViewportsPanel && ViewIndex == ViewportsPanel->GetActiveViewIndex());
		if (Outline->GetTarget() && bIsActiveViewport) {
			OutlineRenderer->OnRender(*Outline, SceneView.ViewProjectionMatrix,
			ViewportSetting);
		}

		if (Gizmo->GetTarget() && bIsActiveViewport) {
			RenderCommand::ClearDepthStencil(DepthTarget);
			GizmoRenderer->OnRender(*Gizmo, SceneView.ViewProjectionMatrix,
			SceneView.ViewLocation,
			!SceneView.bIsPerspective);
		}

		RenderCommand::ClearDepthStencil(DepthTarget);

		if (SettingsPanel->GetSettings().bShowUUID) {
			RenderActorUUIDs(SceneView);
		}
	}

	RenderCommand::EndRenderPass();
}

// View Texture가 포함된 UI를 Swapchain 백버퍼에 합성한다. Present는
// FEngineLoop가 한다.
void UEditorEngine::PresentFrame() {
	// Swapchain 렌더링
	RenderCommand::BeginRenderPass(MainWindowSC->GetBackbuffer(), nullptr,
			MainWindow ? MainWindow->GetWidth() : 0,
			MainWindow ? MainWindow->GetHeight() : 0);

	{
		SCOPE_CYCLE_COUNTER(STAT_ImGui);

		ImGuiRenderer->Begin();

		EditorUI->OnRender();

		ImGuiRenderer->End();
	}

	RenderCommand::EndRenderPass();
}

// 종료 전 정리
void UEditorEngine::PreExit() {
	if (SettingsPanel) {
		SettingsPanel->CaptureViewportSettings();
		SettingsPanel->SaveSettings();
		SettingsPanel->SetViewportsPanel(nullptr);
	}
	if (EditorControlsPanel) {
		EditorControlsPanel->SetViewportsPanel(nullptr);
	}
	ImGuiRenderer->Shutdown();
}

// 선택과 Gizmo 참조를 정리한 뒤 Actor를 삭제한다.
void UEditorEngine::DeleteActor(AActor *Actor) {
	if (!Actor)
		return;

	OutlinerPanel->SelectActor(nullptr);

	Actor->Destroy();
}

// 씬 변경으로 무효화된 에디터의 선택 참조를 모두 해제한다.
void UEditorEngine::ResetSceneSelection() {
	Gizmo->SetTarget(nullptr);
	Outline->SetTarget(nullptr);
	DetailsPanel->SetTarget(nullptr);
	OutlinerPanel->SelectActor(nullptr);
}

// 새 씬 생성이 성공하면 에디터 선택 상태를 초기화한다.
void UEditorEngine::CreateNewScene() {
	if (!FEditorFileUtils::NewScene(EditorWorld))
		return;

	ResetSceneSelection();
}

// 씬 불러오기가 성공하면 에디터 선택 상태를 초기화한다.
void UEditorEngine::OpenScene() {
	if (!FEditorFileUtils::LoadScene(EditorWorld))
		return;

	ResetSceneSelection();
}

// 공통 파일 유틸리티로 현재 씬을 저장한다.
void UEditorEngine::SaveCurrentScene() {
	FEditorFileUtils::SaveScene(EditorWorld);
}

// 공통 파일 유틸리티로 새 경로에 씬을 저장한다.
void UEditorEngine::SaveSceneAs() {
	FEditorFileUtils::SaveSceneAs(EditorWorld);
}

void UEditorEngine::RenderActorUUIDs(const FSceneView &SceneView) {
	const bool bOrtho = !SceneView.bIsPerspective;
	UWorld *TargetWorld = PlayWorld ? PlayWorld : EditorWorld;

	for (AActor *Actor : TargetWorld->GetPersistentLevel()->GetActors()) {
		if (!Actor)
			continue;
		UPrimitiveComponent *Primitive =
				Cast<UPrimitiveComponent>(Actor->GetRootComponent());

		if (!Primitive)
			continue;

		// 액터 머리 위 위치 계산
		FBox Box = Primitive->CalcBounds();
		FVector UUIDLocation((Box.Min.X + Box.Max.X) * 0.5f,
			(Box.Min.Y + Box.Max.Y) * 0.5f, Box.Max.Z + 0.5f);

		FString Text = "UUID : " + std::to_string(Actor->GetUUID());
		TextRenderer->BuildTextMesh(Text, 0.5f, *SystemFont);
		// 빌보드로 렌더링

		FVector Facing = bOrtho ? (SceneView.ViewForward * -1.0f)
			: (SceneView.ViewLocation - UUIDLocation).Normalized();
		FVector Up = FVector(0.0f, 0.0f, 1.0f);
		FVector Right = FVector::Cross(Up, Facing).Normalized();
		FVector RealUp = FVector::Cross(Facing, Right).Normalized();
		FMatrix BillboardWorld = FMatrix::Identity;
		BillboardWorld.M[0][0] = Facing.X;
		BillboardWorld.M[0][1] = Facing.Y;
		BillboardWorld.M[0][2] = Facing.Z;
		BillboardWorld.M[1][0] = Right.X;
		BillboardWorld.M[1][1] = Right.Y;
		BillboardWorld.M[1][2] = Right.Z;
		BillboardWorld.M[2][0] = RealUp.X;
		BillboardWorld.M[2][1] = RealUp.Y;
		BillboardWorld.M[2][2] = RealUp.Z;
		BillboardWorld.M[3][0] = UUIDLocation.X;
		BillboardWorld.M[3][1] = UUIDLocation.Y;
		BillboardWorld.M[3][2] = UUIDLocation.Z;

		TextRenderer->OnRender(Text, BillboardWorld, 0.5f, *SystemFont,
			SceneView.ViewProjectionMatrix);
	}
}

void UEditorEngine::CreatePIESession()
{
	// 선택 해제
	ResetSceneSelection();
	PIEState = EPIEState::Playing;

	// 게임 인스턴스 생성 및 초기화
	GameInstance = FObjectFactory::ConstructObject<UGameInstance>();
	FWorldContext* Worldctx = &CreateNewWorldContext(EWorldType::PIE);		 // worldcontext 만들기

	if (GameInstance)
	{
		GameInstance->InitializeForPlayInEditor(WorldContextlist.size()-1); 
		GameInstance->StartPlayInEditorGameInstance();
		PlayWorld = GameInstance->GetWorld();
	}

	// 패널 월드 설정
	if (PlayWorld)
	{
		EditorControlsPanel->SetWorld(PlayWorld);
		OutlinerPanel->SetWorld(PlayWorld);
		DetailsPanel->SetWorld(PlayWorld);
		SettingsPanel->SetWorld(PlayWorld);
	}
}

void UEditorEngine::StopPIESession()
{
	PIEState = EPIEState::Stopped;

	// 선택 해제
	ResetSceneSelection();

	// 게임 세션 종료
	if (PlayWorld)
	{
		PlayWorld->EndPlay();
	}

	if (GameInstance)
	{
		GameInstance->Shutdown();
		GameInstance = nullptr;
	}

	// 플레이 월드 자원 해제
	if (PlayWorld)
	{
		PlayWorld->ClearWorld();
		for (ULevel* Level : PlayWorld->GetLevel())
		{
			delete Level;
		}
		delete PlayWorld;
		PlayWorld = nullptr;
	}

	// 월드 컨텍스트 목록 정리
	for (int32 Index = WorldContextlist.Num() - 1; Index >= 0; --Index)
	{
		if (WorldContextlist[Index].WorldType == EWorldType::PIE)
		{
			WorldContextlist.RemoveAt(Index, 1);
		}
	}

	// 에디터 카메라 갱신 재연결
	if (EditorWorld && EditorWorld->GetMainCamera())
	{
		ACameraActor* Camera = EditorWorld->GetMainCamera();
		Camera->RegisterAllActorTickFunctions(false);
		Camera->SetWorld(EditorWorld);
		Camera->RegisterAllActorTickFunctions(true);
	}

	// 에디터 패널 월드 복원
	EditorControlsPanel->SetWorld(EditorWorld);
	OutlinerPanel->SetWorld(EditorWorld);
	DetailsPanel->SetWorld(EditorWorld);
	SettingsPanel->SetWorld(EditorWorld);

	// 참조 맵 초기화
	OriginNewAnnotataion.Reset();
}

UWorld* UEditorEngine::CreatePIEWorld()
{
	FWorldContext* Context = GetWorldContextFromPIEInstance(0);
	if (!Context)
	{
		Context = &CreateNewWorldContext(EWorldType::PIE);
		Context->PIEInstance = 0;
	}
	return CreatePIEWorldByDuplication(*Context, EditorWorld);
}

UWorld* UEditorEngine::CreatePIEWorldByDuplication(FWorldContext& WorldContext, UWorld* InWorld)
{
	if (!InWorld)
	{
		HTR_LOG(Error, "InWorld is nullptr");
		return nullptr;
	}

	double StartTime = FPlatformTime::Seconds();

	// 복제 월드 생성
	UWorld* NewPIEWorld = FObjectFactory::ConstructObject<UWorld>();
	NewPIEWorld->Init();

	// 직렬화 복사 및 참조 복원
	SerializeWorldForPIE(InWorld, NewPIEWorld);
	RecoverPIEWorldReferences(InWorld, NewPIEWorld);

	// 컨텍스트 갱신
	WorldContext.SetCurrentWorld(NewPIEWorld);
	WorldContext.WorldType = EWorldType::PIE;

	HTR_LOG(Info, "PIE: Created PIE world by copying editor world ({:.4f}s)", FPlatformTime::Seconds() - StartTime);
	return NewPIEWorld;
}

void UEditorEngine::SerializeWorldForPIE(UWorld* InEditorWorld, UWorld* InPIEWorld)
{
	json WorldData;
	InEditorWorld->Serialize(WorldData, false);
	InPIEWorld->Serialize(WorldData, true);

	// 에디터 월드와 플레이 월드 매핑
	OriginNewAnnotataion.Add(InEditorWorld, InPIEWorld);

	// 레벨 순회 및 복제
	for (int32 i = 0; i < InEditorWorld->GetLevel().Num(); ++i)
	{
		json LevelData;
		ULevel* OriginalLevel = InEditorWorld->GetLevel()[i];
		OriginalLevel->Serialize(LevelData, false);
		ULevel* NewLevel = Cast<ULevel>(FObjectFactory::ConstructObject(OriginalLevel->GetClass(), InPIEWorld));
		NewLevel->Serialize(LevelData, true);
		OriginNewAnnotataion.Add(OriginalLevel, NewLevel);

		// 액터 순회 및 복제
		for (int32 j = 0; j < OriginalLevel->GetActors().Num(); ++j)
		{
			json ActorData;
			AActor* OriginalActor = OriginalLevel->GetActors()[j];
			OriginalActor->Serialize(ActorData, false);
			AActor* NewActor = Cast<AActor>(FObjectFactory::ConstructObject(OriginalActor->GetClass(), NewLevel));
			NewActor->Serialize(ActorData, true);
			OriginNewAnnotataion.Add(InEditorWorld->GetLevel()[i]->GetActors()[j], NewActor);

			// 컴포넌트 순회 및 복제
			for (int32 k = 0; k < OriginalActor->GetComponents().Num(); ++k)
			{
				json ActorCompData;
				UActorComponent* OriginalComp = OriginalActor->GetComponents()[k];
				OriginalComp->Serialize(ActorCompData, false);
				UActorComponent* NewActorComp = nullptr;

				// 기본 생성 컴포넌트 검사
				for (UActorComponent* DupComponents : NewActor->GetComponents())
				{
					if (DupComponents && DupComponents->GetFName() == OriginalComp->GetFName() && DupComponents->GetClass() == OriginalComp->GetClass())
					{
						NewActorComp = DupComponents;
						break;
					}
				}

				if (NewActorComp == nullptr)
				{
					NewActorComp = Cast<UActorComponent>(FObjectFactory::ConstructObject(OriginalComp->GetClass(), NewActor));
					NewActor->AddComponents(NewActorComp);
				}

				NewActorComp->Serialize(ActorCompData, true);
				OriginNewAnnotataion.Add(OriginalComp, NewActorComp);
			}
		}
	}
}

UWorld* UEditorEngine::RecoverPIEWorldReferences(UWorld* InEditorWorld, UWorld* InPIEWorld)
{
	// 참조 관계 복원
	for (auto pair : OriginNewAnnotataion)
	{
		// 레벨 참조 복원
		if (pair.second->IsA(ULevel::StaticClass()))
		{
			ULevel* OriginalLevel = Cast<ULevel>(pair.first);
			ULevel* NewLevel = Cast<ULevel>(pair.second);
			NewLevel->SetWorld(InPIEWorld);
			InPIEWorld->AddLevel(NewLevel);
		}

		// 액터 참조 복원
		if (pair.second->IsA(AActor::StaticClass()))
		{
			AActor* OriginalActor = Cast<AActor>(pair.first);
			AActor* NewActor = Cast<AActor>(pair.second);
			NewActor->SetWorld(InPIEWorld);
			NewActor->SetLevel(Cast<ULevel>(OriginNewAnnotataion[OriginalActor->GetLevel()]));
			NewActor->GetLevel()->AddActor(NewActor);

			USceneComponent* OriginalRoot = OriginalActor->GetRootComponent();
			if (OriginalRoot == nullptr)
			{
				NewActor->SetRootComponent(nullptr);
			}
			else
			{
				NewActor->SetRootComponent(Cast<USceneComponent>(OriginNewAnnotataion[OriginalRoot]));
			}
		}

		// 컴포넌트 참조 복원
		if (pair.second->IsA(UActorComponent::StaticClass()))
		{
			UActorComponent* OriginalActorComp = Cast<UActorComponent>(pair.first);
			UActorComponent* NewActorComp = Cast<UActorComponent>(pair.second);
			NewActorComp->SetOwner(Cast<AActor>(OriginNewAnnotataion[OriginalActorComp->GetOwner()]));

			if (UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(NewActorComp))
			{
				InPIEWorld->GetScene().AddPrimitive(Primitive);
			}

			USceneComponent* SceneOrigin = Cast<USceneComponent>(OriginalActorComp);
			USceneComponent* Scene = Cast<USceneComponent>(NewActorComp);
			if (SceneOrigin != nullptr && Scene != nullptr)
			{
				if (SceneOrigin->GetAttachParent() == nullptr)
				{
					Scene->SetupAttachment(nullptr);
				}
				else
				{
					Scene->SetupAttachment(Cast<USceneComponent>(OriginNewAnnotataion[SceneOrigin->GetAttachParent()]));
				}
			}
		}
	}

	// 현재 레벨과 퍼시스턴트 레벨 복원
	InPIEWorld->SetCurrentLevel(Cast<ULevel>(OriginNewAnnotataion[InEditorWorld->GetCurrentLevel()]));
	InPIEWorld->SetPersistentLevel(Cast<ULevel>(OriginNewAnnotataion[InEditorWorld->GetPersistentLevel()]));
	InPIEWorld->GetWorldType() = EWorldType::PIE;

	return InPIEWorld;
}