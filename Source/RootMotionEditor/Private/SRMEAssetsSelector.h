// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

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

	void OnFinishedChangingProperties(const FPropertyChangedEvent& ChangedEvent) const;
	
protected:
	TSharedPtr<IDetailsView> Widget;
	TObjectPtr<class URMEAssetCollection> AssetCollection = nullptr;
};
