// Fill out your copyright notice in the Description page of Project Settings.


#include "SRMEAssetsSelector.h"

#include "RMEContext.h"
#include "RMETypes.h"
#include "Animation/Skeleton.h"
#include "Engine/SkeletalMesh.h"
#include "Misc/MessageDialog.h"
#include "PropertyEditorModule.h"
#include "SWarningOrErrorBox.h"

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
	CommitSelectionState();
}

SRMEAssetsSelector::~SRMEAssetsSelector()
{
	AssetCollection->RemoveFromRoot();
}

void SRMEAssetsSelector::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SScrollBox)
		+SScrollBox::Slot()
		.Padding(5.f)
		[
			SNew(SWarningOrErrorBox)
			.Message(LOCTEXT("AssetsSelectorWarnings",
				"You have already configured the same curve asset in multiple curve parameters, which will result in the data being overwritten when writing it."))
			.Visibility_Lambda([this](){ return this->bHasRepeatedCurve ? EVisibility::Visible : EVisibility::Collapsed; })
		]
		+SScrollBox::Slot()
		[
			Widget.ToSharedRef()
		]
	];

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

void SRMEAssetsSelector::CheckAssetValidation()
{
	if (AssetCollection)
	{
		bHasRepeatedCurve = AssetCollection->HasRepeatedCurve();
	}
}

void SRMEAssetsSelector::OnFinishedChangingProperties(const FPropertyChangedEvent& ChangedEvent)
{
	if (bIsUpdatingAssetCollection || !AssetCollection)
	{
		CheckAssetValidation();
		return;
	}

	if (ChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(URMEAssetCollection, AnimSequence))
	{
		HandleAnimationSequenceChanged();
	}
	else if (ChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(URMEAssetCollection, PreviewMesh))
	{
		HandlePreviewMeshChanged();
	}

	CheckAssetValidation();
}

void SRMEAssetsSelector::ApplySelectionToContext() const
{
	if (FRMEContext* Context = FRMEContext::Get())
	{
		Context->SetPreviewAssets(
			AssetCollection ? AssetCollection->AnimSequence : nullptr,
			AssetCollection ? AssetCollection->PreviewMesh : nullptr);
	}
}

void SRMEAssetsSelector::CommitSelectionState()
{
	LastCommittedAnimation = AssetCollection ? AssetCollection->AnimSequence : nullptr;
	LastCommittedPreviewMesh = AssetCollection ? AssetCollection->PreviewMesh : nullptr;
	bLastCommittedManualPreviewMeshOverride = bHasManualPreviewMeshOverride;
}

void SRMEAssetsSelector::RestoreCommittedSelection()
{
	if (!AssetCollection)
	{
		return;
	}

	bIsUpdatingAssetCollection = true;
	AssetCollection->AnimSequence = LastCommittedAnimation.Get();
	AssetCollection->PreviewMesh = LastCommittedPreviewMesh.Get();
	bHasManualPreviewMeshOverride = bLastCommittedManualPreviewMeshOverride;
	bIsUpdatingAssetCollection = false;

	if (Widget.IsValid())
	{
		Widget->ForceRefresh();
	}

	ApplySelectionToContext();
}

void SRMEAssetsSelector::SetPreviewMeshValue(USkeletalMesh* InPreviewMesh, bool bManualOverride)
{
	if (!AssetCollection)
	{
		return;
	}

	bIsUpdatingAssetCollection = true;
	AssetCollection->PreviewMesh = InPreviewMesh;
	bHasManualPreviewMeshOverride = bManualOverride && InPreviewMesh != nullptr;
	bIsUpdatingAssetCollection = false;

	if (Widget.IsValid())
	{
		Widget->ForceRefresh();
	}
}

USkeletalMesh* SRMEAssetsSelector::ResolveAnimationPreviewMesh(UAnimSequence* InAnimation) const
{
	if (InAnimation == nullptr)
	{
		return nullptr;
	}

	USkeleton* Skeleton = InAnimation->GetSkeleton();
	return Skeleton ? Skeleton->GetAssetPreviewMesh(InAnimation) : nullptr;
}

bool SRMEAssetsSelector::IsPreviewMeshCompatibleWithAnimation(const USkeletalMesh* InPreviewMesh, const UAnimSequence* InAnimation) const
{
	if (InAnimation == nullptr || InPreviewMesh == nullptr)
	{
		return true;
	}

	const USkeleton* Skeleton = InAnimation->GetSkeleton();
	return Skeleton != nullptr && Skeleton->IsCompatibleMesh(InPreviewMesh);
}

void SRMEAssetsSelector::HandleAnimationSequenceChanged()
{
	if (!AssetCollection)
	{
		return;
	}

	UAnimSequence* NewAnimation = AssetCollection->AnimSequence;
	if (NewAnimation == nullptr)
	{
		ApplySelectionToContext();
		CommitSelectionState();
		return;
	}

	if (!bHasManualPreviewMeshOverride)
	{
		SetPreviewMeshValue(ResolveAnimationPreviewMesh(NewAnimation), false);
		ApplySelectionToContext();
		CommitSelectionState();
		return;
	}

	if (IsPreviewMeshCompatibleWithAnimation(AssetCollection->PreviewMesh, NewAnimation))
	{
		ApplySelectionToContext();
		CommitSelectionState();
		return;
	}

	const EAppReturnType::Type Choice = FMessageDialog::Open(
		EAppMsgType::YesNo,
		FText::Format(
			LOCTEXT("OverrideIncompatiblePreviewMesh",
				"The current animation asset ({0}) is not compatible with the manually assigned preview mesh ({1}). Do you want to replace the preview mesh with the animation's preview mesh?"),
			FText::FromString(GetNameSafe(NewAnimation)),
			FText::FromString(GetNameSafe(AssetCollection->PreviewMesh))));

	if (Choice == EAppReturnType::Yes)
	{
		SetPreviewMeshValue(ResolveAnimationPreviewMesh(NewAnimation), false);
		ApplySelectionToContext();
		CommitSelectionState();
		return;
	}

	RestoreCommittedSelection();
}

void SRMEAssetsSelector::HandlePreviewMeshChanged()
{
	if (!AssetCollection)
	{
		return;
	}

	if (AssetCollection->PreviewMesh == nullptr)
	{
		SetPreviewMeshValue(ResolveAnimationPreviewMesh(AssetCollection->AnimSequence), false);
		ApplySelectionToContext();
		CommitSelectionState();
		return;
	}

	if (!IsPreviewMeshCompatibleWithAnimation(AssetCollection->PreviewMesh, AssetCollection->AnimSequence))
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			FText::Format(
				LOCTEXT("InvalidPreviewMesh",
					"The selected preview mesh ({0}) is not compatible with the current animation asset ({1}). The previous preview mesh will be restored."),
				FText::FromString(GetNameSafe(AssetCollection->PreviewMesh)),
				FText::FromString(GetNameSafe(AssetCollection->AnimSequence))));
		RestoreCommittedSelection();
		return;
	}

	bHasManualPreviewMeshOverride = true;
	ApplySelectionToContext();
	CommitSelectionState();
}

#undef LOCTEXT_NAMESPACE

