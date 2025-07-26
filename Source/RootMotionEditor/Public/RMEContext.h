#pragma once
#include "CoreMinimal.h"

class URMECurveContainer;

class FRMEContext : public TSharedFromThis<FRMEContext>, public FGCObject
{
	
public:
	void Initialize();

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override { return TEXT("FRootMotionEditedContext"); }

	struct FRootMotionEditorPreviewRequiredArgs MakePreviewRequiredArgs();

	void InitTab(TSharedPtr<class FTabManager> TabManager);

	class FRMEViewModel* GetViewModel() const { return ViewModel.Get(); }

	// view model
	bool IsEditorSelection() const;
	float GetViewModelPlayTime() const;
	TRange<double> GetViewModelPlayTimeRange() const;
	void SetViewModelPlayTime(float InPlayTime, bool bInTickPlayTime);

	void SetAnimationAsset(UAnimSequence* InAnimationSequence);
	UAnimSequence* GetAnimationAsset() const;

	void SetRootMotionViewMode(int32 InViewMode);
	int32 GetRootMotionViewMode() const;


	// Slider button callback
	void PreviewBackwardEnd();
	void PreviewBackwardStep();
	void PreviewBackward();
	void PreviewPause();
	void PreviewForward();
	void PreviewForwardStep();
	void PreviewForwardEnd();

	// Curve
	const FTransformCurve* GetRootMotionTransformCurve() const;
	FTransform GetCurveTransform(float Time, float Weight) const;
	URMECurveContainer* GetCurveContainer() const { return CurveDataPtr.Get();}

protected:
	TObjectPtr<URMECurveContainer> CurveDataPtr;
	
	TSharedPtr<class FTabManager> MainTagManager;
	TSharedPtr<class FRMEViewModel> ViewModel;
	TSharedPtr<class FRMEPreviewScene> PreviewScene;
	
	TObjectPtr<UAnimSequence> CurrentAnimation;
};
