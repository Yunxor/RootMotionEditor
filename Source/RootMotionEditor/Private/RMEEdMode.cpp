// Fill out your copyright notice in the Description page of Project Settings.


#include "RMEEdMode.h"

#include "RMEViewModel.h"
#include "SRMEViewport.h"

namespace RootMotionEditor
{
	const FEditorModeID FRMEEdMode::EdModeId = TEXT("RootMotionEditorEdMode");
	
	FRMEEdMode::FRMEEdMode()
	{
	}

	FRMEEdMode::~FRMEEdMode()
	{
	}

	void FRMEEdMode::Tick(FEditorViewportClient* ViewportClient, float DeltaTime)
	{
		FEdMode::Tick(ViewportClient, DeltaTime);

		FRootMotionEditorViewportClient* RootMotionEditorViewportClient = static_cast<FRootMotionEditorViewportClient*>(ViewportClient);
		if (RootMotionEditorViewportClient)
		{
			// ensure we redraw even if PIE is active
			RootMotionEditorViewportClient->Invalidate();

			if (!ViewModel)
			{
				ViewModel = RootMotionEditorViewportClient->ViewModel.Pin().Get();
			}
		}

		if (ViewModel)
		{
			ViewModel->Tick(DeltaTime);
		}
	}

	void FRMEEdMode::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
	{
		FEdMode::Render(View, Viewport, PDI);
	}

	bool FRMEEdMode::HandleClick(FEditorViewportClient* InViewportClient, HHitProxy* HitProxy, const FViewportClick& Click)
	{
		return FEdMode::HandleClick(InViewportClient, HitProxy, Click);
	}

	bool FRMEEdMode::StartTracking(FEditorViewportClient* InViewportClient, FViewport* InViewport)
	{
		return FEdMode::StartTracking(InViewportClient, InViewport);
	}

	bool FRMEEdMode::EndTracking(FEditorViewportClient* InViewportClient, FViewport* InViewport)
	{
		return FEdMode::EndTracking(InViewportClient, InViewport);
	}

	bool FRMEEdMode::InputDelta(FEditorViewportClient* InViewportClient, FViewport* InViewport, FVector& InDrag, FRotator& InRot, FVector& InScale)
	{
		return FEdMode::InputDelta(InViewportClient, InViewport, InDrag, InRot, InScale);
	}

	bool FRMEEdMode::InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
	{
		return FEdMode::InputKey(ViewportClient, Viewport, Key, Event);
	}

	bool FRMEEdMode::AllowWidgetMove()
	{
		return FEdMode::AllowWidgetMove();
	}

	bool FRMEEdMode::ShouldDrawWidget() const
	{
		return FEdMode::ShouldDrawWidget();
	}

	FVector FRMEEdMode::GetWidgetLocation() const
	{
		return FEdMode::GetWidgetLocation();
	}

	bool FRMEEdMode::GetCustomDrawingCoordinateSystem(FMatrix& InMatrix, void* InData)
	{
		return FEdMode::GetCustomDrawingCoordinateSystem(InMatrix, InData);
	}

	bool FRMEEdMode::GetCustomInputCoordinateSystem(FMatrix& InMatrix, void* InData)
	{
		return FEdMode::GetCustomInputCoordinateSystem(InMatrix, InData);
	}
}
