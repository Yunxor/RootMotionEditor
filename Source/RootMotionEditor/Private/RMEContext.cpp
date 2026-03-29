#include "RMEContext.h"
#include "RMECurveEditor.h"
#include "RMEPreviewScene.h"
#include "RMEViewModel.h"
#include "SRMEViewport.h"


FRMEContext* FRMEContext::Instance;
void FRMEContext::Initialize()
{
	Instance = new FRMEContext;
	Instance->Setup();
}

void FRMEContext::Shutdown()
{
	delete Instance;
}

void FRMEContext::Setup()
{
	if (!PreviewScene.IsValid())
	{
		PreviewScene = MakeShareable(
		new FRMEPreviewScene(
			FPreviewScene::ConstructionValues()
				.SetCreatePhysicsScene(false)
				.SetTransactional(false)
				.ForceUseMovementComponentInNonGameWorld(true)));

		//Temporary fix for missing attached assets - MDW (Copied from FPersonaToolkit::CreatePreviewScene)
		PreviewScene->GetWorld()->GetWorldSettings()->SetIsTemporarilyHiddenInEditor(false);
	}

	ViewModel = MakeShared<FRMEViewModel>();
	ViewModel->Initialize(PreviewScene.ToSharedRef());
	CurveEditorPtr.Reset();

	CurveDataPtr = URMECurveContainer::Create();
}

void FRMEContext::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(CurveDataPtr);
}

FRMEPreviewRequiredArgs FRMEContext::MakePreviewRequiredArgs()
{
	return FRMEPreviewRequiredArgs(
		ViewModel.ToSharedRef(), PreviewScene.ToSharedRef());
}

void FRMEContext::InitTab(TSharedPtr<class FTabManager> TabManager)
{
	MainTagManager = TabManager;
}

void FRMEContext::SetCurveEditor(const TSharedPtr<FRMECurveEditor>& InCurveEditor)
{
	CurveEditorPtr = InCurveEditor;
}

void FRMEContext::ClearCurveEditor()
{
	CurveEditorPtr.Reset();
}

bool FRMEContext::IsEditorSelection() const
{
	return ViewModel->IsEditorSelection();
}

float FRMEContext::GetViewModelPlayTime() const
{
	return ViewModel->GetPlayTime();
}

TRange<double> FRMEContext::GetViewModelPlayTimeRange() const
{
	return ViewModel->GetPlayTimeRange();
}


void FRMEContext::SetViewModelPlayTime(float InPlayTime, bool bInTickPlayTime)
{
	ViewModel->SetPlayTime(InPlayTime, bInTickPlayTime);
}

void FRMEContext::SetAnimationAsset(UAnimSequence* InAnimationSequence)
{
	CurrentAnimation = InAnimationSequence;
	ViewModel->SetSelectedAnimation(InAnimationSequence);
}

UAnimSequence* FRMEContext::GetAnimationAsset() const
{
	return CurrentAnimation;
}

void FRMEContext::SetRootMotionViewMode(ERMERootMotionViewMode InViewMode)
{
	ViewModel->SetRootMotionViewMode(InViewMode);
}

ERMERootMotionViewMode FRMEContext::GetRootMotionViewMode() const
{
	return ViewModel->GetRootMotionViewMode();
}

void FRMEContext::SetPreviewEditMode(ERMEPreviewEditMode InEditMode)
{
	ViewModel->SetPreviewEditMode(InEditMode);
}

ERMEPreviewEditMode FRMEContext::GetPreviewEditMode() const
{
	return ViewModel->GetPreviewEditMode();
}

FTransform FRMEContext::GetPreviewManipulatorTransform() const
{
	return ViewModel->GetManipulatorTransform();
}

FVector FRMEContext::GetPreviewManipulatorLocation() const
{
	return ViewModel->GetManipulatorLocation();
}

void FRMEContext::SetPreviewManipulatorTransform(const FTransform& InTransform)
{
	ViewModel->SetManipulatorTransform(InTransform);
}

void FRMEContext::SetPreviewManipulatorLocation(const FVector& InLocation)
{
	ViewModel->SetManipulatorLocation(InLocation);
}

void FRMEContext::AddPreviewManipulatorTranslation(const FVector& InTranslation)
{
	ViewModel->AddManipulatorTranslation(InTranslation);
}

bool FRMEContext::AddPreviewKeyAtCurrentTime()
{
	if (!ViewModel.IsValid())
	{
		return false;
	}

	TSharedPtr<FRMECurveEditor> CurveEditor = CurveEditorPtr.Pin();
	if (!CurveEditor.IsValid())
	{
		return false;
	}

	const bool bHasAddedKey = CurveEditor->AddPreviewKey(
		ViewModel->GetPreviewEditMode(),
		ViewModel->GetPlayTime(),
		ViewModel->GetManipulatorTransform());

	if (bHasAddedKey)
	{
		ViewModel->SetRootMotionViewMode(ERMERootMotionViewMode::Editor);
		ViewModel->SyncManipulatorToCurrentRootMotion();
	}

	return bHasAddedKey;
}

void FRMEContext::PreviewBackwardEnd()
{
	ViewModel->PreviewBackwardEnd();
}

void FRMEContext::PreviewBackwardStep()
{
	ViewModel->PreviewBackwardStep();
}

void FRMEContext::PreviewBackward()
{
	ViewModel->PreviewBackward();
}

void FRMEContext::PreviewPause()
{
	ViewModel->PreviewPause();
}

void FRMEContext::PreviewForward()
{
	ViewModel->PreviewForward();
}

void FRMEContext::PreviewForwardStep()
{
	ViewModel->PreviewForwardStep();
}

void FRMEContext::PreviewForwardEnd()
{
	ViewModel->PreviewForwardEnd();
}

FTransform FRMEContext::GetCurveTransform(float Time, float Weight) const
{
	if (CurveDataPtr == nullptr || CurveDataPtr->CurveData == nullptr)
	{
		return FTransform::Identity;
	}

	return CurveDataPtr->CurveData->Evaluate(Time, Weight);
}

const FTransformCurve* FRMEContext::GetRootMotionTransformCurve() const
{
	if (CurveDataPtr == nullptr)
	{
		return nullptr;
	}
	return CurveDataPtr->CurveData;
}
