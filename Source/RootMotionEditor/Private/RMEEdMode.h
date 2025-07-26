// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EdMode.h"

/**
 * 
 */

class FRMEViewModel;

namespace RootMotionEditor
{
	class FRMEEdMode : public FEdMode
	{
	public:
		const static FEditorModeID EdModeId;

		FRMEEdMode();
		virtual ~FRMEEdMode();

		virtual void Tick(FEditorViewportClient* ViewportClient, float DeltaTime) override;
		virtual void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) override;
		virtual bool HandleClick(
			FEditorViewportClient* InViewportClient,
			HHitProxy* HitProxy,
			const FViewportClick& Click) override;
		virtual bool StartTracking(FEditorViewportClient* InViewportClient, FViewport* InViewport) override;
		virtual bool EndTracking(FEditorViewportClient* InViewportClient, FViewport* InViewport) override;
		virtual bool InputDelta(
			FEditorViewportClient* InViewportClient,
			FViewport* InViewport,
			FVector& InDrag,
			FRotator& InRot,
			FVector& InScale) override;
		virtual bool InputKey(
			FEditorViewportClient* ViewportClient,
			FViewport* Viewport,
			FKey Key,
			EInputEvent Event) override;
		virtual bool AllowWidgetMove() override;
		virtual bool ShouldDrawWidget() const override;
		virtual FVector GetWidgetLocation() const override;
		virtual bool GetCustomDrawingCoordinateSystem(FMatrix& InMatrix, void* InData) override;
		virtual bool GetCustomInputCoordinateSystem(FMatrix& InMatrix, void* InData) override;

	private:
		FRMEViewModel* ViewModel = nullptr;
	};
}
