// Fill out your copyright notice in the Description page of Project Settings.


#include "SRMEAssetsSelector.h"

#include "PropertyCustomizationHelpers.h"
#include "RMEContext.h"
#include "RootMotionEditorModule.h"
#include "SRMECurveSelector.h"
#include "Curves/CurveVector.h"
#include "ThumbnailRendering/ThumbnailManager.h"

#define LOCTEXT_NAMESPACE "SRootMotionEditedAssetView"

FName SRMEAssetsSelector::TabName = FName(TEXT("RootMotionEditorAssetViewTab"));

void SRMEAssetsSelector::RegisterTabSpawner(const TSharedPtr<FTabManager>& TabManager)
{
	TabManager->RegisterTabSpawner(
			TabName,
			FOnSpawnTab::CreateLambda(
				[=](const FSpawnTabArgs&)
				{
					return SNew(SDockTab)
						.TabRole(ETabRole::PanelTab)
						.Label(LOCTEXT("ViewAssetTitle", "Asset Selector"))
						[
							SNew(SRMEAssetsSelector)
						];
				}
			)
		)
		.SetDisplayName(LOCTEXT("ViewAssetTabTitle", "Asset Selector"))
		.SetTooltipText(LOCTEXT("ViewAssetTooltipText", "Open the Asset Selector tab."));
}

SRMEAssetsSelector::SRMEAssetsSelector()
{
	
}

SRMEAssetsSelector::~SRMEAssetsSelector()
{
}

void SRMEAssetsSelector::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoHeight()
			[
				CreateAnimAssetWidget()
			]
			+SVerticalBox::Slot()
			.VAlign(VAlign_Center)
			.FillHeight(1.f)
			[
				SAssignNew(CurveSelector, SRMECurveSelector)
			]
		]
	];
}

TSharedRef<SWidget> SRMEAssetsSelector::CreateAnimAssetWidget()
{
	auto Widget = SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			SNew(STextBlock)
			.Text(FText::FromString("AnimSequence :"))
		]
		+SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.FillWidth(1.f)
		[
			SNew(SObjectPropertyEntryBox)
			.AllowedClass(UAnimSequence::StaticClass())
			.ThumbnailPool(UThumbnailManager::Get().GetSharedThumbnailPool())
			.DisplayThumbnail(true)
			.OnObjectChanged_Lambda([this](const FAssetData& AssetData)
			{
				TSharedRef<FRMEContext> Context = FRootMotionEditorModule::Get().GetContext();
				UAnimSequence* Seq = Cast<UAnimSequence>(AssetData.GetAsset());
				SelectedSequence = Seq;
				Context->SetAnimationAsset(Seq);
			})
			.ObjectPath_Lambda([]()
			{
				TSharedRef<FRMEContext> Context = FRootMotionEditorModule::Get().GetContext();
				UAnimSequence* AnimSeq = Context->GetAnimationAsset();
				return AnimSeq ? AnimSeq->GetPathName() : "None";
			})
		];
	
	return Widget;
}

bool SRMEAssetsSelector::HasAnyCurveAsset() const
{
	return CurveSelector.IsValid() ? CurveSelector->HasAnyCurveAsset() : false;
}

UCurveVector* SRMEAssetsSelector::GetCurveAsset(ERMECurveType CurveType) const
{
	return CurveSelector.IsValid() ? CurveSelector->GetCurveAsset(CurveType) : nullptr;
}

#undef LOCTEXT_NAMESPACE

