// Fill out your copyright notice in the Description page of Project Settings.


#include "RMEEdMode.h"

#include "EditorModeManager.h"
#include "InputCoreTypes.h"
#include "RMEContext.h"
#include "RMEViewModel.h"
#include "SRMEViewport.h"

namespace RootMotionEditor
{
	namespace
	{
		UE::Widget::EWidgetMode GetWidgetModeFromPreviewEditMode(ERMEPreviewEditMode EditMode)
		{
			switch (EditMode)
			{
			case ERMEPreviewEditMode::Translation:
				return UE::Widget::EWidgetMode::WM_Translate;

			case ERMEPreviewEditMode::Rotation:
				return UE::Widget::EWidgetMode::WM_Rotate;

			case ERMEPreviewEditMode::Scale:
				return UE::Widget::EWidgetMode::WM_Scale;

			case ERMEPreviewEditMode::View:
			default:
				return UE::Widget::EWidgetMode::WM_None;
			}
		}
	}

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

		FRMEViewportClient* RootMotionEditorViewportClient = static_cast<FRMEViewportClient*>(ViewportClient);
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
			SyncWidgetSettings();
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
		if (!CanEditWithManipulator())
		{
			return FEdMode::InputDelta(InViewportClient, InViewport, InDrag, InRot, InScale);
		}

		const EAxisList::Type CurrentAxis = InViewportClient ? InViewportClient->GetCurrentWidgetAxis() : EAxisList::None;
		if (CurrentAxis == EAxisList::None)
		{
			return false;
		}

		if (ViewModel->GetPreviewEditMode() == ERMEPreviewEditMode::Translation)
		{
			ViewModel->AddManipulatorTranslation(InDrag);

			if (InViewport)
			{
				InViewport->Invalidate();
			}
			if (InViewportClient)
			{
				InViewportClient->Invalidate();
			}
			return true;
		}

		return FEdMode::InputDelta(InViewportClient, InViewport, InDrag, InRot, InScale);
	}

	bool FRMEEdMode::InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
	{
		if (CanEditWithManipulator() && Event == IE_Pressed && Key == EKeys::S)
		{
			if (FRMEContext* Context = FRMEContext::Get())
			{
				const bool bHasAddedKey = Context->AddPreviewKeyAtCurrentTime();
				if (bHasAddedKey)
				{
					if (Viewport)
					{
						Viewport->Invalidate();
					}
					if (ViewportClient)
					{
						ViewportClient->Invalidate();
					}
				}

				return bHasAddedKey;
			}
		}

		return FEdMode::InputKey(ViewportClient, Viewport, Key, Event);
	}

	bool FRMEEdMode::AllowWidgetMove()
	{
		return CanEditWithManipulator();
	}

	bool FRMEEdMode::ShouldDrawWidget() const
	{
		return CanEditWithManipulator();
	}

	FVector FRMEEdMode::GetWidgetLocation() const
	{
		if (CanEditWithManipulator())
		{
			return ViewModel->GetManipulatorLocation();
		}

		return FEdMode::GetWidgetLocation();
	}

	bool FRMEEdMode::GetCustomDrawingCoordinateSystem(FMatrix& InMatrix, void* InData)
	{
		if (CanEditWithManipulator())
		{
			return false;
		}

		return FEdMode::GetCustomDrawingCoordinateSystem(InMatrix, InData);
	}

	bool FRMEEdMode::GetCustomInputCoordinateSystem(FMatrix& InMatrix, void* InData)
	{
		if (CanEditWithManipulator())
		{
			return false;
		}

		return FEdMode::GetCustomInputCoordinateSystem(InMatrix, InData);
	}

	bool FRMEEdMode::CanEditWithManipulator() const
	{
		return ViewModel != nullptr && ViewModel->GetPreviewEditMode() != ERMEPreviewEditMode::View;
	}

	void FRMEEdMode::SyncWidgetSettings()
	{
		if (ViewModel == nullptr)
		{
			return;
		}

		if (FEditorModeTools* ModeManager = GetModeManager())
		{
			ModeManager->SetCoordSystem(COORD_World);

			const UE::Widget::EWidgetMode DesiredWidgetMode = GetWidgetModeFromPreviewEditMode(ViewModel->GetPreviewEditMode());
			if (ModeManager->GetWidgetMode() != DesiredWidgetMode)
			{
				ModeManager->SetWidgetMode(DesiredWidgetMode);
			}
		}
	}
}
