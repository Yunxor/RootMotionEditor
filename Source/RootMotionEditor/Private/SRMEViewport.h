// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SEditorViewport.h"
#include "EditorViewportClient.h"
#include "SCommonEditorViewportToolbarBase.h"
#include "Animation/DebugSkelMeshComponent.h"


class SMEViewportToolBar;
class SRMEViewport;
class FRMEPreviewScene;
class FRMEViewModel;

struct FRootMotionEditorPreviewRequiredArgs
{
	FRootMotionEditorPreviewRequiredArgs(
		const TSharedRef<FRMEViewModel>& InViewModel,
		const TSharedRef<FRMEPreviewScene>& InPreviewScene)
		: ViewModel(InViewModel)
		, PreviewScene(InPreviewScene)
	{
	}

	TSharedRef<FRMEViewModel> ViewModel;

	TSharedRef<FRMEPreviewScene> PreviewScene;
};


class FRootMotionEditorViewportClient : public FEditorViewportClient
{
public:
	FRootMotionEditorViewportClient(
		const TSharedRef<FRMEPreviewScene>& InPreviewScene,
		const TSharedRef<SRMEViewport>& InViewport,
		const TSharedRef<FRMEViewModel>& InViewModel);
	virtual ~FRootMotionEditorViewportClient() {}

	// ~FEditorViewportClient interface
	virtual void Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	virtual void TrackingStarted(const struct FInputEventState& InInputState, bool bIsDragging, bool bNudge) override;
	virtual void TrackingStopped() override;
	bool CanDrawPreviewComponent(UDebugSkelMeshComponent* MeshComponent);
	// ~End of FEditorViewportClient interface

	virtual void DrawRootMotionData(UDebugSkelMeshComponent* MeshComponent, FPrimitiveDrawInterface* PDI) const;

	
	/** Asset editor we are embedded in */
	TWeakPtr<FRMEViewModel> ViewModel;
};

class SRMEViewport : public SEditorViewport, public ICommonEditorViewportToolbarInfoProvider
{
public:
	SLATE_BEGIN_ARGS(SRMEViewport) {}
	SLATE_END_ARGS();


	void Construct(const FArguments& InArgs, const FRootMotionEditorPreviewRequiredArgs& InRequiredArgs);
	virtual ~SRMEViewport() {}


	// ~ICommonEditorViewportToolbarInfoProvider interface
	virtual TSharedRef<class SEditorViewport> GetViewportWidget() override;
	virtual TSharedPtr<FExtender> GetExtenders() const override;
	virtual void OnFloatingButtonClicked() override;
	FText GetDisplayString() const;
	UDebugSkelMeshComponent* GetPreviewMeshComponent() const;
	// ~End of ICommonEditorViewportToolbarInfoProvider interface

protected:
	void SetVisualizeRootMotionMode(EVisualizeRootMotionMode Mode);
	EVisualizeRootMotionMode GetVisualizeRootMotionMode() const;
	bool CanVisualizeRootMotion() const;
	bool IsVisualizeRootMotionModeSet(EVisualizeRootMotionMode Mode) const;
	// ~SEditorViewport interface
	virtual void BindCommands() override;
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
	virtual TSharedPtr<SWidget> MakeViewportToolbar() override;
	void OnSetRootMotionViewMode(int32 ViewMode);
	bool IsRootMotionViewModeSet(int32 ViewMode) const;
	int32 GetRootMotionViewMode() const;
	// ~End of SEditorViewport interface


	/** Viewport client */
	TSharedPtr<FRootMotionEditorViewportClient> ViewportClient;

	/** The viewport toolbar */
	TSharedPtr<SMEViewportToolBar> ViewportToolbar;
	
	/** The preview scene that we are viewing */
	TWeakPtr<FRMEPreviewScene> PreviewScenePtr;

	TWeakPtr<FRMEViewModel> ViewModel;

};
