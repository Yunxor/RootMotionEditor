#include "RMEContext.h"
#include "RMEPreviewScene.h"
#include "RMETypes.h"
#include "RMEViewModel.h"
#include "SRMEViewport.h"


void FRMEContext::Initialize()
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
	ViewModel->Initialize(PreviewScene.ToSharedRef(), StaticCastSharedRef<FRMEContext>(AsShared()));

	CurveDataPtr = URMECurveContainer::Create();
}

void FRMEContext::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(CurveDataPtr);
}

FRootMotionEditorPreviewRequiredArgs FRMEContext::MakePreviewRequiredArgs()
{
	return FRootMotionEditorPreviewRequiredArgs(
		ViewModel.ToSharedRef(), PreviewScene.ToSharedRef());
}

void FRMEContext::InitTab(TSharedPtr<class FTabManager> TabManager)
{
	MainTagManager = TabManager;
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

void FRMEContext::SetRootMotionViewMode(int32 InViewMode)
{
	ViewModel->SetRootMotionViewMode(static_cast<ERootMotionViewMode::Type>(InViewMode));
}

int32 FRMEContext::GetRootMotionViewMode() const
{
	return ViewModel->GetRootMotionViewMode();
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