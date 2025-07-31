// Fill out your copyright notice in the Description page of Project Settings.


#include "SRMEAssetsSelector.h"

#include "RMEContext.h"
#include "RMETypes.h"
#include "RootMotionEditorModule.h"

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
	InitWidget();
	
	AssetCollection = NewObject<URMEAssetCollection>();
	AssetCollection->AddToRoot();
}

SRMEAssetsSelector::~SRMEAssetsSelector()
{
	AssetCollection->RemoveFromRoot();
}

void SRMEAssetsSelector::Construct(const FArguments& InArgs)
{
	ChildSlot.AttachWidget(Widget.ToSharedRef());

	if (!Widget.IsValid())
	{
		InitWidget();
	}
	
	if (Widget.IsValid())
	{
		Widget->SetObject(AssetCollection);
		Widget->OnFinishedChangingProperties().AddSP(this, &SRMEAssetsSelector::OnFinishedChangingProperties);
	}
}

void SRMEAssetsSelector::InitWidget()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	{
		FDetailsViewArgs ViewArgs;
		{
			ViewArgs.bHideSelectionTip = true;
			ViewArgs.bAllowSearch = false;
		}
		Widget = PropertyModule.CreateDetailView(ViewArgs);
	}
}

bool SRMEAssetsSelector::HasAnyCurveAsset() const
{
	return AssetCollection ? AssetCollection->HasAnyCurveAsset() : false;
}

UAnimSequence* SRMEAssetsSelector::GetSequence() const
{
	return AssetCollection ? AssetCollection->AnimSequence : nullptr;
}

void SRMEAssetsSelector::OnFinishedChangingProperties(const FPropertyChangedEvent& ChangedEvent) const
{
	if (ChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(URMEAssetCollection, AnimSequence))
	{
		if (AssetCollection)
		{
			TSharedRef<FRMEContext> Context = FRootMotionEditorModule::Get().GetContext();
			Context->SetAnimationAsset(AssetCollection->AnimSequence);
		}
	}
}

#undef LOCTEXT_NAMESPACE

