#include "RMECurveEditor.h"
#include "CurveEditor.h"
#include "RichCurveEditorModel.h"
#include "RMEStatics.h"
#include "SCurveEditorPanel.h"
#include "SRMEAssetsSelector.h"
#include "Curves/CurveVector.h"
#include "RMEContext.h"
#include "RMETypes.h"
#include "RMEViewModel.h"
#include "Tree/CurveEditorTreeFilter.h"
#include "Tree/SCurveEditorTree.h"
#include "Tree/SCurveEditorTreeFilterStatusBar.h"
#include "Tree/SCurveEditorTreePin.h"
#include "Tree/SCurveEditorTreeSelect.h"
#include "Tree/SCurveEditorTreeTextFilter.h"
#include "Widgets/Layout/SScrollBorder.h"
#include "Widgets/Input/SSegmentedControl.h"

#define LOCTEXT_NAMESPACE "RMECurveEditor"

FRMECurveEditorTreeItem::FRMECurveEditorTreeItem(TWeakObjectPtr<UObject> InCurveOwner, FRichCurve* InCurveToEdit, FText InCurveName, FLinearColor InCurveColor)
	: CurveOwner(InCurveOwner)
	, CurveToEdit(InCurveToEdit)
	, CurveName(InCurveName)
	, CurveColor(InCurveColor)
{
}

TSharedPtr<SWidget> FRMECurveEditorTreeItem::GenerateCurveEditorTreeWidget(const FName& InColumnName, TWeakPtr<FCurveEditor> InCurveEditor,
	FCurveEditorTreeItemID InTreeItemID, const TSharedRef<ITableRow>& TableRow)
{
	if (InColumnName == ColumnNames.Label)
	{
		return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.Padding(FMargin(4.f))
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Right)
		.AutoWidth()
		[
			SNew(STextBlock)
			.Text(CurveName)
			.ColorAndOpacity(FSlateColor(CurveColor))
		];
	}
	else if (InColumnName == ColumnNames.SelectHeader)
	{
		return SNew(SCurveEditorTreeSelect, InCurveEditor, InTreeItemID, TableRow);
	}
	else if (InColumnName == ColumnNames.PinHeader)
	{
		return SNew(SCurveEditorTreePin, InCurveEditor, InTreeItemID, TableRow);
	}

	return nullptr;
}

void FRMECurveEditorTreeItem::CreateCurveModels(TArray<TUniquePtr<FCurveModel>>& OutCurveModels)
{
	if (!CurveOwner.IsValid())
	{
		return;
	}

	TUniquePtr<FRichCurveEditorModelRaw> NewCurve = MakeUnique<FRichCurveEditorModelRaw>(CurveToEdit, CurveOwner.Get());
	NewCurve->SetShortDisplayName(CurveName);
	NewCurve->SetColor(CurveColor, false);
	OutCurveModels.Add(MoveTemp(NewCurve));
}

bool FRMECurveEditorTreeItem::PassesFilter(const FCurveEditorTreeFilter* InFilter) const
{
	if (InFilter->GetType() == ECurveEditorTreeFilterType::Text)
	{
		FString DisplayNameAsString = CurveName.ToString();

		const FCurveEditorTreeTextFilter* Filter = static_cast<const FCurveEditorTreeTextFilter*>(InFilter);
		for (const FCurveEditorTreeTextFilterTerm& Term : Filter->GetTerms())
		{
			if (!Term.Match(DisplayNameAsString).IsTotalMatch())
			{
				return false;
			}
		}

		return true;
	}

	return false;
	
}


/**
 *	FRMECurveEditor
 */

const FName FRMECurveEditor::CurveEditorTabName(TEXT("RootMotionEditorCurveEditorTab"));
const FName FRMECurveEditor::CurveEditorConfigTabName(TEXT("RootMotionCurveEditorConfigTab"));

void FRMECurveEditor::Initialize()
{
	CurveEditor = MakeShared<FCurveEditor>();
	SetupCurveEditor();
}

void FRMECurveEditor::OnDestroy()
{
	Config->RemoveFromRoot();
	Config = nullptr;
}

void FRMECurveEditor::SetupCurveEditor()
{
	check(CurveEditor.IsValid());
	CurveEditor->InputZoomToFitPadding = 0.0f;
	CurveEditor->OutputZoomToFitPadding = 0.01f;
	FCurveEditorInitParams CurveEditorInitParams;
	CurveEditor->InitCurveEditor(CurveEditorInitParams);
	
	TUniquePtr<ICurveEditorBounds> EditorBounds = MakeUnique<FStaticCurveEditorBounds>();
	EditorBounds->SetInputBounds(-1.05, 1.05);
	CurveEditor->SetBounds(MoveTemp(EditorBounds));

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	{
		FDetailsViewArgs ViewArgs;
		{
			ViewArgs.bHideSelectionTip = true;
			ViewArgs.bAllowSearch = false;
		}
		ConfigWidget = PropertyModule.CreateDetailView(ViewArgs);

		Config = NewObject<URMECurveEditorConfig>();
		Config->AddToRoot();

		ConfigWidget->SetObject(Config);
		ConfigWidget->OnFinishedChangingProperties().AddSP(this, &FRMECurveEditor::OnFinishedChangingProperties);
	}
}

URMECurveContainer* FRMECurveEditor::GetCurveContainer() const
{
	FRMEContext* Context = FRMEContext::Get();
	if (Context != nullptr)
	{
		return Context->GetCurveContainer();
	}
	return nullptr;
}

void FRMECurveEditor::RegisterTabSpawner(const TSharedPtr<FTabManager>& TabManager)
{
	if (!CurveEditor.IsValid())
	{
		Initialize();
	}

	WeakTabManager = TabManager;

	CurveEditorTree = SNew(SCurveEditorTree, CurveEditor);

	CurveEditorPanel = SNew(SCurveEditorPanel, CurveEditor.ToSharedRef())
		.TabManager(TabManager)
		.TreeContent()
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew(CurveEditorSearchBox, SCurveEditorTreeTextFilter, CurveEditor)
			]
			+SVerticalBox::Slot()
			[
				SNew(SScrollBorder, CurveEditorTree.ToSharedRef())
				[
					CurveEditorTree.ToSharedRef()
				]
			]
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SCurveEditorTreeFilterStatusBar, CurveEditor)
			]
		];
	
	OnLoadCurveDataCompleted.AddLambda([]()
	{
		if (FRMEContext* Context = FRMEContext::Get())
		{
			if (FRMEViewModel* ViewModel = Context->GetViewModel())
			{
				ViewModel->SetRootMotionViewMode(ERootMotionViewMode::CurveEditor);
			}
		}
	});

	
	TabManager->RegisterTabSpawner(
			CurveEditorTabName,
			FOnSpawnTab::CreateLambda(
				[this](const FSpawnTabArgs&)
				{
					return SNew(SDockTab)
						.TabRole(ETabRole::PanelTab)
						.Label(LOCTEXT("CurveEditorTitle", "Edited Root Motion Curve"))
						[
							SNew(SVerticalBox)
							+SVerticalBox::Slot()
							.AutoHeight()
							[
								CreateToolbar()
							]
							+SVerticalBox::Slot()
							.AutoHeight()
							[
								CreateCurveEditorToolbar()
							]
							+SVerticalBox::Slot()
							.FillHeight(1.f)
							[
								CurveEditorPanel.ToSharedRef()
							]
						];
				}
			)
		)
		.SetDisplayName(LOCTEXT("CurveEditorTabTitle", "Edited Root Motion Curve"))
		.SetTooltipText(LOCTEXT("CurveEditorTooltipText", "Open the Edited Root Motion Curve tab."));
}

void FRMECurveEditor::RegisterConfigTabSpawner(const TSharedPtr<class FTabManager>& TabManager)
{
	TabManager->RegisterTabSpawner(
			CurveEditorConfigTabName,
			FOnSpawnTab::CreateLambda(
				[this](const FSpawnTabArgs&)
				{
					return SNew(SDockTab)
						.TabRole(ETabRole::PanelTab)
						.Label(LOCTEXT("LabelTitle", "Curve Editor Settings"))
						[
							SNew(SScrollBox)
							+SScrollBox::Slot()
							.Padding(5.f)
							[
								SNew(SSegmentedControl<ERMEBoneExtractMode>)
								.Value_Lambda([this]()
								{
									return Config->ExtractMode;
								})
								.OnValueChanged_Lambda([this](ERMEBoneExtractMode NewMode)
								{
									Config->ExtractMode = NewMode;
								})
								+SSegmentedControl<ERMEBoneExtractMode>::Slot(ERMEBoneExtractMode::RootMotion).Text(LOCTEXT("RootMotionExtractMode", "Root Motion"))
								+SSegmentedControl<ERMEBoneExtractMode>::Slot(ERMEBoneExtractMode::AnimPose).Text(LOCTEXT("AnimPoseExtractMode", "Anim Pose"))
							]
							+SScrollBox::Slot()
							[
								ConfigWidget.ToSharedRef()
							]
						];
				}
			)
		)
		.SetDisplayName(LOCTEXT("TabTitle", "Curve Editor Settings"))
		.SetTooltipText(LOCTEXT("TooltipText", "Open the Curve Editor Settings tab."));
}

FText FRMECurveEditor::GetBoneMessage(bool bIsCustomBone)
{
	if (bIsCustomBone)
	{
		return LOCTEXT("CurveEditorSettingsNotes_Custom",
				"It will use the 'GetAnimPose' function to extract bone data.");
	}

	return LOCTEXT("CurveEditorSettingsNotes_Default",
				"It will use the 'ExtractRootMotion' function to extract bone data, that based on the first bone of the specific animation asset.");
}

void FRMECurveEditor::OnFinishedChangingProperties(const FPropertyChangedEvent& ChangedEvent)
{
	if (Config)
	{
		bIsSetCustomBone = !Config->CustomLoadBoneName.IsNone() || !Config->CustomSaveBoneName.IsNone();
	}
}

TSharedRef<SWidget> FRMECurveEditor::CreateToolbar()
{
	FToolBarBuilder ToolbarBuilder(nullptr, FMultiBoxCustomization::None);
    
	ToolbarBuilder.BeginSection("CurveOperations");
	{
		ToolbarBuilder.AddToolBarButton(
			FUIAction(
				FExecuteAction::CreateSP(this, &FRMECurveEditor::CreateDefaultCurves),
				FCanExecuteAction::CreateSP(this, &FRMECurveEditor::CanEditCurve)),
			NAME_None,
			LOCTEXT("AddCurve", "Add Curve"),
			LOCTEXT("AddCurveTooltip", "Add a new curve"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Plus")
		);
		
		ToolbarBuilder.AddToolBarButton(
		FUIAction(FExecuteAction::CreateSP(this, &FRMECurveEditor::ClearAllCurves),
			FCanExecuteAction::CreateSP(this, &FRMECurveEditor::CanEditCurve)),
			NAME_None,
			LOCTEXT("ClearCurves", "Clear"),
			LOCTEXT("ClearCurvesTooltip", "Remove all curves"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.X")
		);
        
		// ToolbarBuilder.AddToolBarButton(
		// FUIAction(FExecuteAction::CreateSP(this, &FRMECurveEditor::SaveCurveData),
		// 	FCanExecuteAction::CreateSP(this, &FRMECurveEditor::CanEditCurve)),
		// 	NAME_None,
		// 	LOCTEXT("SaveCurves", "Save"),
		// 	LOCTEXT("SaveCurvesTooltip", "Save curve data to file"),
		// 	FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Save")
		// );

		ToolbarBuilder.AddSeparator();

		ToolbarBuilder.AddToolBarButton(
			FUIAction(
				FExecuteAction::CreateSP(this, &FRMECurveEditor::LoadExternalCurveData),
				FCanExecuteAction::CreateSP(this, &FRMECurveEditor::HasExternalCurve)
				),
			NAME_None,
			LOCTEXT("LoadCurve", "Load From Curve"),
			LOCTEXT("LoadCurveTooltip", "Load curve data from curve asset file"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Blueprints")
		);

		ToolbarBuilder.AddToolBarButton(
			FUIAction(
				FExecuteAction::CreateSP(this, &FRMECurveEditor::LoadExternalAnimData),
				FCanExecuteAction::CreateSP(this, &FRMECurveEditor::HasExternalAnim)
				),
			NAME_None,
			LOCTEXT("LoadAnim", "Load From Anim"),
			LOCTEXT("LoadAnimTooltip", "Load curve data from anim asset file"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Blueprints")
		);

		ToolbarBuilder.AddSeparator();

		ToolbarBuilder.AddToolBarButton(
		FUIAction(FExecuteAction::CreateSP(this, &FRMECurveEditor::SaveToExternalCurveData),
			FCanExecuteAction::CreateSP(this, &FRMECurveEditor::HasExternalCurve)
			),
			NAME_None,
			LOCTEXT("SaveToCurveAsset", "Save To Curve Asset"),
			LOCTEXT("SaveToCurveTooltip", "Save curve data to curve asset file"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Save")
		);

		ToolbarBuilder.AddToolBarButton(
		FUIAction(FExecuteAction::CreateSP(this, &FRMECurveEditor::SaveToExternalAnimData),
			FCanExecuteAction::CreateSP(this, &FRMECurveEditor::HasExternalAnim)
			),
			NAME_None,
			LOCTEXT("SaveToAnimAsset", "Save To Anim Asset"),
			LOCTEXT("SaveToAnimTooltip", "Save curve data to anim asset file"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Save")
		);
	}
	ToolbarBuilder.EndSection();
    
	return ToolbarBuilder.MakeWidget();
}

TSharedRef<SWidget> FRMECurveEditor::CreateCurveEditorToolbar()
{
	FSlimHorizontalToolBarBuilder ToolBarBuilder(CurveEditorPanel->GetCommands(), FMultiBoxCustomization::None, CurveEditorPanel->GetToolbarExtender(), true);
	ToolBarBuilder.BeginSection("Asset");
	ToolBarBuilder.EndSection();

	return ToolBarBuilder.MakeWidget();
}

void FRMECurveEditor::AddNewCurveInternal(FVectorCurve& CurveData, UObject* CurveOwner, const FString& ChannelName)
{
	check(CurveEditor.IsValid());
	
	for (int32 Index = 0; Index < 3; Index++)
	{
		FRichCurve* Curve = &CurveData.FloatCurves[Index];

		const FText CurveNameText = FText::FromName(URMECurveContainer::GetFullCurveName(ChannelName, Index));
		const FLinearColor Color = URMECurveContainer::GetCurveAxisColor(Index);
			
		TSharedPtr<FRMECurveEditorTreeItem> TreeItem = MakeShared<FRMECurveEditorTreeItem>(CurveOwner, Curve, CurveNameText, Color);
		FCurveEditorTreeItem* NewItem = CurveEditor->AddTreeItem(FCurveEditorTreeItemID::Invalid());
		NewItem->SetStrongItem(TreeItem);

		// for (const FCurveModelID& CurveModel : NewItem->GetOrCreateCurves(CurveEditor.Get()))
		// {
		// 	CurveEditor->PinCurve(CurveModel);
		// }
		//
		// NewItem->GetOrCreateCurves(CurveEditor.Get());
	}
}

void FRMECurveEditor::CreateDefaultCurves()
{
	FRMEContext* Context = FRMEContext::Get();
	ensureMsgf(Context, TEXT("The Context of root motion editor is null !!"));
	AddNewCurve(Context->GetCurveContainer());
}

void FRMECurveEditor::AddNewCurve(URMECurveContainer* Container)
{
	if (Container == nullptr)
	{
		return;
	}
	
	check(CurveEditor.IsValid());
	
	AddNewCurveInternal(Container->CurveData->TranslationCurve, Container, "Translation");
	AddNewCurveInternal(Container->CurveData->RotationCurve, Container, "Rotation");
	AddNewCurveInternal(Container->CurveData->ScaleCurve, Container, "Scale");
	
	CurveEditor->ZoomToFit();

	bHasCurveEdited = true;
}


void FRMECurveEditor::ClearAllCurves()
{
	ClearEditorAllCurves();
}

void FRMECurveEditor::ClearEditorAllCurves()
{
	check(CurveEditor.IsValid());
	CurveEditor->RemoveAllTreeItems();
	CurveEditor->RemoveAllCurves();

	if (URMECurveContainer* CurveDataPtr = GetCurveContainer())
	{
		CurveDataPtr->ClearAllKeys();
	}

	bHasCurveEdited = false;
}

void FRMECurveEditor::SaveCurveData()
{
	// todo: save to local file.
}

bool FRMECurveEditor::CanEditCurve() const
{
	return true;
}

#pragma region Curves Asset

bool FRMECurveEditor::HasExternalCurve() const
{
	TSharedPtr<SRMEAssetsSelector> Selector = RootMotionEditorStatics::GetTabWidget<SRMEAssetsSelector>(WeakTabManager.Pin().Get(), SRMEAssetsSelector::TabName);
	return Selector.IsValid() ? Selector->HasAnyCurveAsset() : false;
}

void FRMECurveEditor::LoadExternalCurveData()
{
	TSharedPtr<SRMEAssetsSelector> Selector = RootMotionEditorStatics::GetTabWidget<SRMEAssetsSelector>(WeakTabManager.Pin().Get(), SRMEAssetsSelector::TabName);
	URMEAssetCollection* AssetCollection = Selector ? Selector->GetAssetCollection() : nullptr;
	if (AssetCollection && AssetCollection->HasAnyCurveAsset())
	{
		ClearEditorAllCurves();
		URMECurveContainer* CurveDataPtr = GetCurveContainer();
		CurveDataPtr->PushCurveData(AssetCollection->MotionCurve, AssetCollection->RotationCurve, AssetCollection->ScaleCurve);
		AddNewCurve(CurveDataPtr);

		OnLoadCurveDataCompleted.Broadcast();
	}
}

void FRMECurveEditor::SaveToExternalCurveData()
{
	URMECurveContainer* CurveDataPtr = GetCurveContainer();
	if (CurveDataPtr == nullptr)
	{
		return;
	}
	
	TSharedPtr<SRMEAssetsSelector> Selector = RootMotionEditorStatics::GetTabWidget<SRMEAssetsSelector>(WeakTabManager.Pin().Get(), SRMEAssetsSelector::TabName);
	URMEAssetCollection* AssetCollection = Selector ? Selector->GetAssetCollection() : nullptr;
	if (!AssetCollection || !AssetCollection->HasAnyCurveAsset())
	{
		return;
	}
	
	auto SaveCurve = [](UCurveVector* CurveAsset, const FVectorCurve& EditedCurve)
	{
		if (IsValid(CurveAsset))
		{
			CurveAsset->FloatCurves[0] = EditedCurve.FloatCurves[0];
			CurveAsset->FloatCurves[1] = EditedCurve.FloatCurves[1];
			CurveAsset->FloatCurves[2] = EditedCurve.FloatCurves[2];
			
			CurveAsset->MarkPackageDirty();
		}
	};

	if (FTransformCurve* CurveData = CurveDataPtr->CurveData)
	{
		SaveCurve(AssetCollection->MotionCurve, CurveData->TranslationCurve);
		SaveCurve(AssetCollection->RotationCurve, CurveData->RotationCurve);
		SaveCurve(AssetCollection->ScaleCurve, CurveData->ScaleCurve);
	}
	
	
}

#pragma endregion Curves Asset


#pragma region Anim Asset

bool FRMECurveEditor::HasExternalAnim() const
{
	TSharedPtr<SRMEAssetsSelector> Selector = RootMotionEditorStatics::GetTabWidget<SRMEAssetsSelector>(WeakTabManager.Pin().Get(), SRMEAssetsSelector::TabName);
	return Selector.IsValid() ? Selector->GetSequence() != nullptr : false;
}

void FRMECurveEditor::LoadExternalAnimData()
{
	TSharedPtr<SRMEAssetsSelector> Selector = RootMotionEditorStatics::GetTabWidget<SRMEAssetsSelector>(WeakTabManager.Pin().Get(), SRMEAssetsSelector::TabName);
	UAnimSequence* AnimSequence = Selector ? Selector->GetSequence() : nullptr;
	if (AnimSequence != nullptr)
	{
		checkf(Config, TEXT("Not fount root motion editor config, please check it."));
		
		if (Config->ExtractMode == ERMEBoneExtractMode::AnimPose)
		{
			FName TargetBoneName = Config->CustomLoadBoneName;
			if (TargetBoneName.IsNone() || !RootMotionEditorStatics::IsValidBoneName(AnimSequence, TargetBoneName))
			{
				FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("InvalidAnimData", "Load bone name ({0}) is invalid from the anim({1})."), FText::FromString(TargetBoneName.ToString()), FText::FromString(GetNameSafe(AnimSequence))));
				return;
			}

			// Prompts user.
			if (bHasCurveEdited)
			{
				const EAppReturnType::Type Choice = FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("HasEditedCurve", "You have already edited the curve. Are you sure you want to discard it and write it into the animation data ?"));
				if (Choice == EAppReturnType::No)
				{
					return;
				}
			}

			ClearEditorAllCurves();

			URMECurveContainer* CurveDataPtr = GetCurveContainer();
			const FTransformCurve& Curve = RootMotionEditorStatics::BakeAnimPoseBoneToCurve(AnimSequence, TargetBoneName, Config->SampleRate,
					Config->ExtractChannels, Config->bIsAdditiveCurve, Config->EvaluationOptions, Config->Space);
			CurveDataPtr->CopyCurveData(Curve);
			AddNewCurve(CurveDataPtr);

			OnLoadCurveDataCompleted.Broadcast();
		}
		else if (Config->ExtractMode == ERMEBoneExtractMode::RootMotion)
		{
			if (!AnimSequence->HasRootMotion())
			{
				FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("InvalidMotionData", "The AnimSequence haven't root motion, please check whether EnableRootMotion is enabled."));
				return;
			}

			// Prompts user.
			if (bHasCurveEdited)
			{
				const EAppReturnType::Type Choice = FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("HasEditedCurve", "You have already edited the curve. Are you sure you want to discard it and write it into the animation data ?"));
				if (Choice == EAppReturnType::No)
				{
					return;
				}
			}

			ClearEditorAllCurves();

			URMECurveContainer* CurveDataPtr = GetCurveContainer();
			const FTransformCurve& Curve = RootMotionEditorStatics::BakeRootBoneToCurve(AnimSequence, Config->SampleRate, Config->ExtractChannels, Config->bIsAdditiveCurve);
			CurveDataPtr->CopyCurveData(Curve);
			AddNewCurve(CurveDataPtr);

			OnLoadCurveDataCompleted.Broadcast();
		}
	}
}

void FRMECurveEditor::SaveToExternalAnimData()
{
	FRMEContext* Context = FRMEContext::Get();
	const FTransformCurve* CurveData = Context ? Context->GetRootMotionTransformCurve() : nullptr;
	if (!bHasCurveEdited || !CurveData)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoCurveData", "Not found your edited curve data. Can't write to the anim asset."));
		return;
	}
	
	TSharedPtr<SRMEAssetsSelector> Selector = RootMotionEditorStatics::GetTabWidget<SRMEAssetsSelector>(WeakTabManager.Pin().Get(), SRMEAssetsSelector::TabName);
	UAnimSequence* AnimSequence = Selector ? Selector->GetSequence() : nullptr;

	if (AnimSequence == nullptr)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("InvalidAnimData", "Not found anim asset."));
		return;
	}

	const EAppReturnType::Type Choice = FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("OverrideAnimRootData", "It will override the root skeleton data of the animation. Are you sure ?"));
	if (Choice == EAppReturnType::No)
	{
		return;
	}

	const bool bIsCustomSave = Config && !Config->CustomSaveBoneName.IsNone();
	FName CustomBoneToSave = NAME_None;

	if (bIsCustomSave)
	{
		CustomBoneToSave = Config->CustomSaveBoneName;

		// check bone.
		if (!RootMotionEditorStatics::IsValidBoneName(AnimSequence, CustomBoneToSave))
		{
			FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("InvalidAnimData", "Save bone name ({0}) is invalid.from the anim({1})."), FText::FromString(CustomBoneToSave.ToString()), FText::FromString(GetNameSafe(AnimSequence))));
			return;
		}
	}

	RootMotionEditorStatics::OverrideAnimBoneMotion(AnimSequence, *CurveData, CustomBoneToSave);

	AnimSequence->MarkPackageDirty();
}

#pragma endregion Anim Asset

#undef LOCTEXT_NAMESPACE
