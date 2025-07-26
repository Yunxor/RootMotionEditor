// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

enum class ERMECurveType : uint8;
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

	TSharedRef<SWidget> CreateAnimAssetWidget();

	bool HasAnyCurveAsset() const;
	class UCurveVector* GetCurveAsset(ERMECurveType CurveType) const;
	UAnimSequence* GetSequence() const { return SelectedSequence; }

	TSharedRef<class SRMECurveSelector> GetCurveSelector() const { return CurveSelector.ToSharedRef(); }

protected:
	TSharedPtr<class SRMECurveSelector> CurveSelector = nullptr;
	TObjectPtr<class UAnimSequence> SelectedSequence = nullptr;
};
