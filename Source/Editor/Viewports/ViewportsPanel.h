#pragma once

#include "Editor/EditorUI/EditorPanel.h"
#include "Editor/Viewports/EditorViewportClient.h" 
#include "Editor/Viewports/ViewportLayout.h"
#include <functional>

// PIE 실행 상태
enum class EPIEState : uint8 {
	Stopped,
	Playing,
	Paused
};

// PIE 버튼 액션
enum class EPIEAction : uint8 {
	None,
	Play,
	Pause,
	Resume,
	Step,
	Stop
};

class FViewportsPanel : public IEditorPanel {
public:
	// 활성 뷰포트 인덱스 설정
	void SetActiveViewIndex(const int32 InIndex) { ActiveViewIndex = InIndex; }
	int32 GetActiveViewIndex() const { return ActiveViewIndex; }

	// 네 View의 렌더 타깃과 UI 제어 상태를 초기화한다.
	bool Init() override;
	// 프레임 입력에서 Splitter Drag와 View별 UI 요청을 수집한다.
	void Tick(float DeltaTime) override;
	// 각 View Texture와 Splitter·Layout 조작 UI를 ImGui 패널에 그린다.
	void OnRender() override;
	const char *GetPanelName() const override { return "Viewports"; }

	// Core가 계산한 View Rect에 맞춰 해당 클라이언트의 활성 상태와 크기를 갱신한다.
	void SetView(int32 ViewIndex, const FRect &Rect, bool bActive);

	FVector2 GetContentSize() const { return {ContentSize.x, ContentSize.y}; }
	FVector2 GetLocalMousePosition() const;
	bool IsHovered() const;

	float ConsumeHorizontalDrag();
	float ConsumeVerticalDrag();

	// UI에서 발생한 Layout 변경 요청을 한 번 소비하도록 반환한다.
	bool ConsumeLayoutRequest(ELayoutMode &OutMode, int32 &OutSingleViewIndex);
	void SetLayoutMode(ELayoutMode InMode, int32 InSingleIndex = 0) {
		CurrentLayoutMode = InMode;
		CurrentSingleViewIndex = InSingleIndex;
	}

	// PIE 상태 조회 및 콜백 연동
	void SetPIEStateGetter(std::function<EPIEState()> InGetter) { PIEStateGetter = InGetter; }
	EPIEState GetPIEState() const { return PIEStateGetter ? PIEStateGetter() : PIEState; }
	void SetPIEState(EPIEState NewState) { PIEState = NewState; }

	EPIEAction ConsumePIEAction() {
		const EPIEAction Action = PendingPIEAction;
		PendingPIEAction = EPIEAction::None;
		return Action;
	}

	// 뷰포트 클라이언트 조회
	FEditorViewportClient* GetViewportClient(const int32 ViewIndex) const
	{
			if (ViewIndex >= 0 && ViewIndex < 4)
			{
					return ViewportClients[ViewIndex].get();
			}
			return nullptr;
	}

	void SetShowNoCamera(bool bValue) { bShowNoCamera = bValue; }

private:
	int32 ActiveViewIndex = 0;

	// 통계 정보 출력
	void DrawStatOverlay(ImDrawList *DrawList, const ImVec2 &ViewMin) const;

	TUniquePtr<FEditorViewportClient> ViewportClients[4]{};
	ImVec2 ContentOrigin{};
	ImVec2 ContentSize{1.0f, 1.0f};
	bool bHovered = false;
	bool bStatResetButtonVisible = false;
	ImVec2 StatResetButtonMin{};
	ImVec2 StatResetButtonMax{};
	float PendingHorizontalDrag = 0.0f;
	float PendingVerticalDrag = 0.0f;
	// 누른 뒤 커서가 Splitter 영역을 벗어나도 버튼을 놓을 때까지 드래그를 유지한다.
	bool bDraggingVerticalSplitter = false;
	bool bDraggingHorizontalSplitter = false;
	ELayoutMode CurrentLayoutMode = ELayoutMode::QuadSplit;
	int32 CurrentSingleViewIndex = 0;
	bool bHasLayoutRequest = false;
	ELayoutMode RequestedLayoutMode = ELayoutMode::QuadSplit;
	int32 RequestedSingleViewIndex = 0;

	// PIE 상태 및 대기 액션
	EPIEState PIEState = EPIEState::Stopped;
	std::function<EPIEState()> PIEStateGetter;
	EPIEAction PendingPIEAction = EPIEAction::None;
};
