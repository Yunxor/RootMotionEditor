// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimPreviewInstance.h"
#include "RMETypes.h"

class SRootMotionEditor;
class FRMEPreviewScene;


struct FRootMotionEditorPreviewActor
{
public:
	bool SetupPreviewActor(UWorld* World, UAnimSequence* InAnimation);
	void UpdatePreviewActor(float PlayTime, class FRMEViewModel* InViewModel);
	void ClearPreviewActor();
	bool DrawPreviewActor();

	void Destroy();
	
	UAnimPreviewInstance* GetAnimPreviewInstanceInternal();
	UDebugSkelMeshComponent* GetDebugSkelMeshComponent() const;
	UAnimPreviewInstance* GetAnimPreviewInstance() const;

	const UAnimSequence* GetAnimAsset() const { return AnimAssetPtr.Get(); }

private:
	TWeakObjectPtr<AActor> ActorPtr;
	TWeakObjectPtr<UAnimSequence> AnimAssetPtr;
};



/**
 * 
 */
class FRMEViewModel : public TSharedFromThis<FRMEViewModel>, public FGCObject
{
public:
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override { return TEXT("FRootMotionEditedViewModel"); }

	void Initialize(const TSharedRef<FRMEPreviewScene>& InPreviewScene, const TSharedRef<class FRMEContext>& InContext);
	
	void Tick(float DeltaSeconds);

	void SetSelectedAnimation(UAnimSequence* InAnimation);
	
	UWorld* GetWorld();

	bool IsEditorSelection() const { return true; }
	float GetPlayTime() const { return PlayTime; }
	void SetPlayTime(float NewPlayTime, bool bInTickPlayTime);

	TRange<double> GetPlayTimeRange() const;

	void SetRootMotionViewMode(ERootMotionViewMode::Type InType);
	ERootMotionViewMode::Type GetRootMotionViewMode() const { return RootMotionViewMode; }

	FRMEContext* GetContext() const { return EditedContext.Pin().Get(); }

	void PreviewBackwardEnd();
	void PreviewBackwardStep();
	void PreviewBackward();
	void PreviewPause();
	void PreviewForward();
	void PreviewForwardStep();
	void PreviewForwardEnd();

	FTransform GetRootMotionTransform(float Time) const;

	UDebugSkelMeshComponent* GetDebugSkelMeshComponent() const { return PreviewActor.GetDebugSkelMeshComponent(); }
	const UAnimSequence* GetAnimation() const { return PreviewActor.GetAnimAsset(); }
	
private:
	FRootMotionEditorPreviewActor PreviewActor;

	/** Weak pointer to the PreviewScene */
	TWeakPtr<FRMEPreviewScene> PreviewScenePtr;
	TWeakPtr<FRMEContext> EditedContext;

	float PlayTime = 0.f;
	float DeltaTimeMultiplier = 1.f;
	float StepDeltaTime = 1.f / 30.f;
	
	/** From zero to the play length of the longest preview */
	float MinPreviewPlayLength = 0.f;
	float MaxPreviewPlayLength = 0.f;

	 ERootMotionViewMode::Type RootMotionViewMode = ERootMotionViewMode::None;
};
