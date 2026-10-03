#pragma once

#include "Editor/EditorUI/EditorPanel.h"

#include <functional>

class FEditorUI
{
public:
	bool Init(bool bInUseDockSpace = true, bool bInPassthruCentralNode = false);
	void Tick(float DeltaTime);
	void OnRender();

	template <typename T>
	T* AddEditorPanel()
	{
		TUniquePtr<T> newPanel = MakeUnique<T>();
		T* Ret = newPanel.get();
		Ret->Init();
		Panels.Add(std::move(newPanel));
		
		return Ret;
	}

	void SetNewSceneCallback(std::function<void()> InCallback) { OnNewScene = InCallback; }
	void SetOpenSceneCallback(std::function<void()> InCallback) { OnOpenScene = InCallback; }
	void SetSaveSceneCallback(std::function<void()> InCallback) { OnSaveScene = InCallback; }
	void SetSaveSceneAsCallback(std::function<void()> InCallback) { OnSaveSceneAs = InCallback; }
	void SetCreatePIECallback(std::function<void()> InCallback) { OnCreatePIE = InCallback; }
	void SetStopPIECallback(std::function<void()> InCallback) { OnStopPIE = InCallback; }

private:
	bool bUseDockSpace = true;
	bool bPassthruCentralNode = false;
	bool bPIESessionActive = false;

	void DrawMainMenuBar();
	void DrawMainToolBar();

	TArray<TUniquePtr<IEditorPanel>> Panels;

	std::function<void()> OnNewScene;
	std::function<void()> OnOpenScene;
	std::function<void()> OnSaveScene;
	std::function<void()> OnSaveSceneAs;
	std::function<void()> OnCreatePIE;
	std::function<void()> OnStopPIE;

};
