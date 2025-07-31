// Fill out your copyright notice in the Description page of Project Settings.


#include "SRMEViewport.h"
#include "AssetEditorModeManager.h"
#include "RMECommands.h"
#include "RMEEdMode.h"
#include "SMEViewportToolBar.h"
#include "RMEPreviewScene.h"
#include "RMEStatics.h"
#include "RMEViewModel.h"
#include "RootMotionEditorModule.h"
#include "UnrealWidget.h"
#include "RMETypes.h"
#include "Animation/DebugSkelMeshComponent.h"

#define LOCTEXT_NAMESPACE "SRMEViewport"

FText ConcatenateLine(const FText& InText, const FText& InNewLine)
{
	if(InText.IsEmpty())
	{
		return InNewLine;
	}

	return FText::Format(LOCTEXT("ViewportTextNewlineFormatter", "{0}\n{1}"), InText, InNewLine);
}


FRMEViewportClient::FRMEViewportClient(
	const TSharedRef<FRMEPreviewScene>& InPreviewScene,
	const TSharedRef<SRMEViewport>& InViewport,
	const TSharedRef<FRMEViewModel>& InViewModel)
	: FEditorViewportClient(nullptr, &InPreviewScene.Get(), StaticCastSharedRef<SEditorViewport>(InViewport))
	, ViewModel(InViewModel)
{
	
	Widget->SetUsesEditorModeTools(ModeTools.Get());
	StaticCastSharedPtr<FAssetEditorModeManager>(ModeTools)->SetPreviewScene(&InPreviewScene.Get());
	ModeTools->SetDefaultMode(RootMotionEditor::FRMEEdMode::EdModeId);

	SetRealtime(true);

	SetWidgetCoordSystemSpace(COORD_Local);
	ModeTools->SetWidgetMode(UE::Widget::WM_Translate);
}



void FRMEViewportClient::Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	FRMEViewModel* ViewModePtr = ViewModel.Pin().Get();
	UDebugSkelMeshComponent* PreviewComponent = ViewModePtr ? ViewModePtr->GetDebugSkelMeshComponent() : nullptr;

	if (!CanDrawPreviewComponent(PreviewComponent))
	{
		return;
	}
	
	FEditorViewportClient::Draw(View, PDI);


	DrawRootMotionData(PreviewComponent, PDI);
}

void FRMEViewportClient::TrackingStarted(const struct FInputEventState& InInputState, bool bIsDragging, bool bNudge)
{
	
	ModeTools->StartTracking(this, Viewport);
}

void FRMEViewportClient::TrackingStopped()
{
	ModeTools->EndTracking(this, Viewport);
	Invalidate();
}

bool FRMEViewportClient::CanDrawPreviewComponent(UDebugSkelMeshComponent* MeshComponent)
{
	if (!MeshComponent || !MeshComponent->GetSkinnedAsset() || MeshComponent->GetSkinnedAsset()->IsCompiling())
	{
		return false;
	}

	return true;
}

void FRMEViewportClient::DrawRootMotionData(UDebugSkelMeshComponent* MeshComponent, FPrimitiveDrawInterface* PDI) const
{
	constexpr float DepthBias = 2.0f;
	constexpr bool bScreenSpace = true;
	
	if (!MeshComponent || !MeshComponent->GetSkeletalMeshAsset())
	{
		return;
	}
	
	FRMEViewModel* ViewModelPtr = ViewModel.Pin().Get();
	if (!ViewModelPtr)
	{
		return;
	}
		
	const EVisualizeRootMotionMode VisMode = MeshComponent->GetVisualizeRootMotionMode();
	if (VisMode == EVisualizeRootMotionMode::None)
	{
		return;
	}
	
	const UAnimSequence* Animation = ViewModelPtr->GetAnimation();

	if (!Animation)
	{
		return;
	}
	
	const FColor TrajectoryColor = FColor::Black.WithAlpha(64);
	const USkeleton* Skeleton = Animation->GetSkeleton();
	check(Skeleton);
	EAxis::Type SkeletonForwardAxis = Skeleton->GetPreviewForwardAxis();
	
	FVector PrevLocation;
	
	// Draw root motion trajectory
	const int32 NumFrames = Animation->GetNumberOfSampledKeys();
	const float PlayLength = Animation->GetPlayLength();
	const FFrameRate FrameRate = Animation->GetSamplingFrameRate();
	for (int32 Frame = 0; Frame <= NumFrames; Frame++)
	{
		const double Time = FMath::Clamp(FrameRate.AsSeconds(Frame), 0., (double)PlayLength);
		const FTransform& Transform = ViewModelPtr->GetRootMotionTransform(Time);
		const FVector Location = Transform.GetLocation();

		const bool bFirstOrLastPoint = Frame == 0 || Frame == NumFrames;

		PDI->DrawPoint(Location, TrajectoryColor, bFirstOrLastPoint ? 12.f : 6.f, SDPG_World);

		if (VisMode == EVisualizeRootMotionMode::TrajectoryAndOrientation)
		{
			if (bFirstOrLastPoint || (Frame % 3 == 0))
			{
				const FVector XAxis = Transform.GetUnitAxis(SkeletonForwardAxis);
				const FColor AxisColor = RootMotionEditorStatics::GetColorForAxis(SkeletonForwardAxis);

				FVector YAxis, ZAxis;
				XAxis.FindBestAxisVectors(YAxis,ZAxis);
				RootMotionEditorStatics::DrawFlatArrow(PDI, Transform.GetLocation(), XAxis, ZAxis, AxisColor.WithAlpha(64), 15.0f, 8, nullptr, SDPG_World, 1.0f);
			}
		}

		if (Frame > 0)
		{
			PDI->DrawTranslucentLine(PrevLocation, Location, TrajectoryColor, SDPG_World, 1.0f, DepthBias, bScreenSpace);
		}
		PrevLocation = Location;
	}

	
	// Draw current location on the root motion.
	{
		const float CurrentTime = MeshComponent->GetPosition();
		const FTransform& Transform = ViewModelPtr->GetRootMotionTransform(CurrentTime);

		const FVector XAxis = Transform.GetUnitAxis(SkeletonForwardAxis);
		const FColor AxisColor = RootMotionEditorStatics::GetColorForAxis(SkeletonForwardAxis);

		FVector YAxis, ZAxis;
		XAxis.FindBestAxisVectors(YAxis,ZAxis);

		if (VisMode == EVisualizeRootMotionMode::TrajectoryAndOrientation)
		{
			RootMotionEditorStatics::DrawFlatArrow(PDI, Transform.GetLocation(), XAxis, ZAxis, AxisColor, 30.0f, 15, GEngine->ArrowMaterialYellow->GetRenderProxy(), SDPG_Foreground, 1.0f);
		}
		RootMotionEditorStatics::DrawCoordinateSystem(PDI, Transform, 10.0f, 20.0f, DepthBias, bScreenSpace, 200);
	}
}


/**
 *	SRMEViewport
 */

void SRMEViewport::Construct(const FArguments& InArgs, const FRMEPreviewRequiredArgs& InRequiredArgs)
{
	ViewModel = InRequiredArgs.ViewModel;
	PreviewScenePtr = InRequiredArgs.PreviewScene;
	
	SEditorViewport::Construct(
		SEditorViewport::FArguments()
		.IsEnabled(FSlateApplication::Get().GetNormalExecutionAttribute())
		.AddMetaData<FTagMetaData>(TEXT("AnimationTools.Viewport"))
	);
}

TSharedRef<class SEditorViewport> SRMEViewport::GetViewportWidget()
{
	return SharedThis(this);
}

TSharedPtr<FExtender> SRMEViewport::GetExtenders() const
{
	TSharedPtr<FExtender> Result(MakeShareable(new FExtender));
	return Result;
}

void SRMEViewport::OnFloatingButtonClicked()
{
}

FText SRMEViewport::GetDisplayString() const
{
	UDebugSkelMeshComponent* PreviewComponent = GetPreviewMeshComponent();

	FText DefaultText;
	
	if (PreviewComponent)
	{
		if (PreviewComponent->IsPreviewOn())
		{
			DefaultText = FText::Format(LOCTEXT("Previewing", "Previewing {0}"), FText::FromString(PreviewComponent->GetPreviewText()));
		}
	}
	else
	{
		DefaultText = LOCTEXT("NoMesh", "No skeletal mesh.");
	}

	FText RootMotionModeText = FText::Format(LOCTEXT("RootMotionModeText", "Root Motion Mode: {0}"), FText::FromString(ERootMotionViewMode::GetDisplayName(GetRootMotionViewMode())));
	DefaultText = ConcatenateLine(DefaultText, RootMotionModeText);

	FText VisualizeText = StaticEnum<EVisualizeRootMotionMode>()->GetDisplayNameTextByValue((int64)GetVisualizeRootMotionMode());
	FText VisualizeModeText = FText::Format(LOCTEXT("VisualizeModeText", "Trajectory Mode: {0}"), VisualizeText);
	DefaultText = ConcatenateLine(DefaultText, VisualizeModeText);

	return DefaultText;
}

UDebugSkelMeshComponent* SRMEViewport::GetPreviewMeshComponent() const
{
	FRMEViewModel* ViewModePtr = ViewModel.Pin().Get();
	return ViewModePtr ? ViewModePtr->GetDebugSkelMeshComponent() : nullptr;
}

void SRMEViewport::SetVisualizeRootMotionMode(EVisualizeRootMotionMode Mode)
{
	UDebugSkelMeshComponent* PreviewComponent = GetPreviewMeshComponent();
	if (PreviewComponent)
	{
		PreviewComponent->SetVisualizeRootMotionMode(Mode);
	}
}

EVisualizeRootMotionMode SRMEViewport::GetVisualizeRootMotionMode() const
{
	UDebugSkelMeshComponent* PreviewComponent = GetPreviewMeshComponent();
	if (PreviewComponent)
	{
		return PreviewComponent->GetVisualizeRootMotionMode();
	}
	return EVisualizeRootMotionMode::None;
}

bool SRMEViewport::CanVisualizeRootMotion() const
{
	return true;
}

bool SRMEViewport::IsVisualizeRootMotionModeSet(EVisualizeRootMotionMode Mode) const
{
	UDebugSkelMeshComponent* PreviewComponent = GetPreviewMeshComponent();
	if (PreviewComponent)
	{
		return PreviewComponent->IsVisualizeRootMotionMode(Mode);
	}
	
	return false;
}

void SRMEViewport::BindCommands()
{
	SEditorViewport::BindCommands();

	const FRMECommands& Commands = FRMECommands::Get();
	
	CommandList->MapAction(
		Commands.RootMotionNone,
		FExecuteAction::CreateSP(this, &SRMEViewport::OnSetRootMotionViewMode, (int32)ERootMotionViewMode::None),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &SRMEViewport::IsRootMotionViewModeSet, (int32)ERootMotionViewMode::None)
	);

	CommandList->MapAction(
		Commands.RootMotionFromAnimAsset,
		FExecuteAction::CreateSP(this, &SRMEViewport::OnSetRootMotionViewMode, (int32)ERootMotionViewMode::AnimAsset),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &SRMEViewport::IsRootMotionViewModeSet, (int32)ERootMotionViewMode::AnimAsset)
	);

	CommandList->MapAction(
		Commands.RootMotionFromCurveEditor,
		FExecuteAction::CreateSP(this, &SRMEViewport::OnSetRootMotionViewMode, (int32)ERootMotionViewMode::CurveEditor),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &SRMEViewport::IsRootMotionViewModeSet, (int32)ERootMotionViewMode::CurveEditor)
	);


	CommandList->MapAction(
		Commands.DoNotVisualizeRootMotion,
		FExecuteAction::CreateSP(this, &SRMEViewport::SetVisualizeRootMotionMode, EVisualizeRootMotionMode::None),
		FIsActionChecked::CreateSP(this, &SRMEViewport::CanVisualizeRootMotion),
		FIsActionChecked::CreateSP(this, &SRMEViewport::IsVisualizeRootMotionModeSet, EVisualizeRootMotionMode::None));

	CommandList->MapAction(
		Commands.VisualizeRootMotionTrajectory,
		FExecuteAction::CreateSP(this, &SRMEViewport::SetVisualizeRootMotionMode, EVisualizeRootMotionMode::Trajectory),
		FIsActionChecked::CreateSP(this, &SRMEViewport::CanVisualizeRootMotion),
		FIsActionChecked::CreateSP(this, &SRMEViewport::IsVisualizeRootMotionModeSet, EVisualizeRootMotionMode::Trajectory));

	CommandList->MapAction(
		Commands.VisualizeRootMotionTrajectoryAndOrientation,
		FExecuteAction::CreateSP(this, &SRMEViewport::SetVisualizeRootMotionMode, EVisualizeRootMotionMode::TrajectoryAndOrientation),
		FIsActionChecked::CreateSP(this, &SRMEViewport::CanVisualizeRootMotion),
		FIsActionChecked::CreateSP(this, &SRMEViewport::IsVisualizeRootMotionModeSet, EVisualizeRootMotionMode::TrajectoryAndOrientation));
}

TSharedRef<FEditorViewportClient> SRMEViewport::MakeEditorViewportClient()
{
	ViewportClient = MakeShared<FRMEViewportClient>(
			PreviewScenePtr.Pin().ToSharedRef(),
			SharedThis(this),
			ViewModel.Pin().ToSharedRef()
			);
	ViewportClient->ViewportType = LVT_Perspective;
	ViewportClient->bSetListenerPosition = false;
	ViewportClient->SetViewLocation(EditorViewportDefs::DefaultPerspectiveViewLocation);
	ViewportClient->SetViewRotation(EditorViewportDefs::DefaultPerspectiveViewRotation);

	return ViewportClient.ToSharedRef();
}

TSharedPtr<SWidget> SRMEViewport::MakeViewportToolbar()
{
	return SAssignNew(ViewportToolbar, SMEViewportToolBar, SharedThis(this));
}

void SRMEViewport::OnSetRootMotionViewMode(int32 ViewMode)
{
	TSharedRef<FRMEContext> Context = FRootMotionEditorModule::Get().GetContext();
	Context->SetRootMotionViewMode(ViewMode);
	Invalidate();
}

bool SRMEViewport::IsRootMotionViewModeSet(int32 ViewMode) const
{
	TSharedRef<FRMEContext> Context = FRootMotionEditorModule::Get().GetContext();
	return Context->GetRootMotionViewMode() == ViewMode;
}

int32 SRMEViewport::GetRootMotionViewMode() const
{
	TSharedRef<FRMEContext> Context = FRootMotionEditorModule::Get().GetContext();
	return Context->GetRootMotionViewMode();
}

#undef LOCTEXT_NAMESPACE