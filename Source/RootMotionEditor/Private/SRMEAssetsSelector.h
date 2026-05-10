// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class IDetailsView;
struct FPropertyChangedEvent;

/**
 * 
 */
class SRMEAssetsSelector : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SRMEAssetsSelector) { }
	SLATE_END_ARGS()

	static FName TabName;

	static void RegisterTabSpawner(const TSharedPtr<FTabManager>& TabManager);
	
public:
	SRMEAssetsSelector();
	virtual ~SRMEAssetsSelector();

	void Construct(const FArguments& InArgs);
	void InitWidget();

	class URMEAssetCollection* GetAssetCollection() const { return AssetCollection; }
	bool HasAnyCurveAsset() const;
	UAnimSequence* GetSequence() const;

	void CheckAssetValidation();
	void OnFinishedChangingProperties(const FPropertyChangedEvent& ChangedEvent);
	
protected:
	void ApplySelectionToContext() const;
	void CommitSelectionState();
	void RestoreCommittedSelection();
	void SetPreviewMeshValue(class USkeletalMesh* InPreviewMesh, bool bManualOverride);
	class USkeletalMesh* ResolveAnimationPreviewMesh(class UAnimSequence* InAnimation) const;
	bool IsPreviewMeshCompatibleWithAnimation(const class USkeletalMesh* InPreviewMesh, const class UAnimSequence* InAnimation) const;
	void HandleAnimationSequenceChanged();
	void HandlePreviewMeshChanged();

	TSharedPtr<IDetailsView> Widget;
	TObjectPtr<class URMEAssetCollection> AssetCollection = nullptr;

	bool bHasRepeatedCurve = false;
	bool bHasManualPreviewMeshOverride = false;
	bool bIsUpdatingAssetCollection = false;
	bool bLastCommittedManualPreviewMeshOverride = false;
	TWeakObjectPtr<class UAnimSequence> LastCommittedAnimation;
	TWeakObjectPtr<class USkeletalMesh> LastCommittedPreviewMesh;
};
