// Fill out your copyright notice in the Description page of Project Settings.


#include "SRootMotionEditor.h"
#include "RMEContext.h"
#include "RMECurveEditor.h"
#include "RootMotionEditorModule.h"
#include "SRMEPreview.h"
#include "SRMEAssetsSelector.h"
#include "SRMEViewport.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"


#define LOCTEXT_NAMESPACE "SRootMotionEditor"

const FName SRootMotionEditor::WindowName(TEXT("RootMotionEditorMainTab"));

void SRootMotionEditor::RegisterTabSpawner()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
			SRootMotionEditor::WindowName,
			FOnSpawnTab::CreateStatic(&SRootMotionEditor::SpawnRootMotionEditor)
		)
		.SetDisplayName(LOCTEXT("FRootMotionEditorTabTitle", "RootMotionEditor"))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsDebugCategory())
		// .SetIcon(FSlateIcon(IconName, "Tab.Main"))
		.SetMenuType(ETabSpawnerMenuType::Hidden)
		// FORT-497240 - Issue is that having a Tab with nested tabs will auto close when in the sidebar if any area of the nested tabs are clicked
		.SetCanSidebarTab(false);
}

void SRootMotionEditor::UnregisterTabSpawner()
{
	if (FSlateApplication::IsInitialized())
	{
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(SRootMotionEditor::WindowName);
	}
}

TSharedRef<class SDockTab> SRootMotionEditor::SpawnRootMotionEditor(const class FSpawnTabArgs& Args)
{
	auto NomadTab = SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		.Label(LOCTEXT("FRootMotionEditorTabTitle", "RootMotionEditor"));

	auto TabManager = FGlobalTabmanager::Get()->NewTabManager(NomadTab);
	TabManager->SetOnPersistLayout(
		FTabManager::FOnPersistLayout::CreateStatic(
			[](const TSharedRef<FTabManager::FLayout>& InLayout)
			{
				if (InLayout->GetPrimaryArea().Pin().IsValid())
				{
					FLayoutSaveRestore::SaveToConfig(FRootMotionEditorModule::RootMotionEditorLayoutIni, InLayout);
					GConfig->Flush(false, FRootMotionEditorModule::RootMotionEditorLayoutIni);
				}
			}
		)
	);

	NomadTab->SetOnTabClosed(
		SDockTab::FOnTabClosedCallback::CreateStatic(
			[](TSharedRef<SDockTab> Self, TWeakPtr<FTabManager> TabManager)
			{
				TSharedPtr<FTabManager> OwningTabManager = TabManager.Pin();
				if (OwningTabManager.IsValid())
				{
					FLayoutSaveRestore::SaveToConfig(FRootMotionEditorModule::RootMotionEditorLayoutIni, OwningTabManager->PersistLayout());
					GConfig->Flush(false, FRootMotionEditorModule::RootMotionEditorLayoutIni);
					OwningTabManager->CloseAllAreas();
				}
			}
			, TWeakPtr<FTabManager>(TabManager)
		)
	);

	auto MainWidget = SNew(SRootMotionEditor)
		.TabManager(TabManager);

	NomadTab->SetContent(MainWidget);
	return NomadTab;
}

SRootMotionEditor::SRootMotionEditor()
{
}

SRootMotionEditor::~SRootMotionEditor()
{
	if (CurveEditor.IsValid())
	{
		CurveEditor->OnDestroy();
	}
	CurveEditor = nullptr;
}

void SRootMotionEditor::Construct(const FArguments& InArgs)
{
	TabManager = InArgs._TabManager;

	TSharedRef<FRMEContext> Context = FRootMotionEditorModule::Get().GetContext();
	Context->InitTab(TabManager);
	
	FRMEPreviewRequiredArgs RequiredArgs = Context->MakePreviewRequiredArgs();

	// Register DockTab.
	SRMEAssetsSelector::RegisterTabSpawner(TabManager);
	SRMEPreview::RegisterTabSpawner(TabManager, Context);


	if (!CurveEditor.IsValid())
	{
		CurveEditor = MakeShared<FRMECurveEditor>();
		CurveEditor->Initialize();
	}
	CurveEditor->RegisterTabSpawner(TabManager, Context);
	CurveEditor->RegisterConfigTabSpawner(TabManager);
	

	// Default Layout.
	TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout =
		FTabManager::NewLayout("RootMotionEditor_StandaloneLayout_v0.7")
			->AddArea
			(
				// Main application area
				FTabManager::NewPrimaryArea()
				->SetOrientation(Orient_Horizontal)
				->Split
				(
					FTabManager::NewSplitter()
					->SetOrientation(Orient_Vertical)
					->SetSizeCoefficient(0.2f)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.6f)
						->AddTab(SRMEAssetsSelector::TabName, ETabState::OpenedTab)
						->SetHideTabWell(false)
					)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.4f)
						->AddTab(FRMECurveEditor::CurveEditorConfigTabName, ETabState::OpenedTab)
						->SetHideTabWell(false)
					)
				)
				->Split
				(
					FTabManager::NewSplitter()
					->SetOrientation(Orient_Vertical)
					->SetSizeCoefficient(0.8f)
					->Split
					 (
						FTabManager::NewStack()
						->SetSizeCoefficient(0.6f)
						->AddTab(SRMEPreview::TabName, ETabState::OpenedTab)
						->SetHideTabWell(true)
					 )
					 ->Split
					 (
						FTabManager::NewStack()
						->SetSizeCoefficient(0.4f)
						->AddTab(FRMECurveEditor::CurveEditorTabName, ETabState::OpenedTab)
						->SetHideTabWell(true)
					 )
				)
			);

	// Load layout config save in local file.
	StandaloneDefaultLayout = FLayoutSaveRestore::LoadFromConfig(FRootMotionEditorModule::RootMotionEditorLayoutIni, StandaloneDefaultLayout);

	// apply config.
	TSharedRef<SWidget> TabContents = TabManager->RestoreFrom(StandaloneDefaultLayout, TSharedPtr<SWindow>()).ToSharedRef();


	// create & initialize main menu
	FMenuBarBuilder MenuBarBuilder = FMenuBarBuilder(TSharedPtr<FUICommandList>());

	MenuBarBuilder.AddPullDownMenu(
		LOCTEXT("WindowMenuLabel", "Window"),
		FText::GetEmpty(),
		FNewMenuDelegate::CreateSP(this, &SRootMotionEditor::FillWindowMenu),
		"Window"
	);

	TSharedRef<SWidget> MenuWidget = MenuBarBuilder.MakeWidget();

	// Tell tab-manager about the multi-box for platforms with a global menu bar
	TabManager->SetMenuMultiBox(MenuBarBuilder.GetMultiBox(), MenuWidget);

	
	ChildSlot
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			MenuWidget
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			MakeToolbar()
		]
		+SVerticalBox::Slot()
		.Padding(2.0)
		[
			TabContents
		]
	];
}

void SRootMotionEditor::FillWindowMenu(FMenuBuilder& MenuBuilder)
{
	if (!TabManager.IsValid())
	{
		return;
	}
	
	TabManager->PopulateLocalTabSpawnerMenu(MenuBuilder);
}

TSharedRef<SWidget> SRootMotionEditor::MakeToolbar()
{
	FSlimHorizontalToolBarBuilder ToolbarBuilder(MakeShareable(new FUICommandList), FMultiBoxCustomization::None);
	ToolbarBuilder.BeginStyleOverride("CalloutToolbar");
	ToolbarBuilder.BeginSection("Main");

	// TODO: Add button.
	
	{
		// ToolbarBuilder.AddToolBarButton(
		// 	FUIAction(
		// 		FExecuteAction::CreateLambda([=]() {}),
		// 		FCanExecuteAction(),
		// 		FIsActionChecked::CreateLambda([=]() { })
		// 	),
		// 	NAME_None,
		// 	LOCTEXT("HudEnableLabel", "HUD"),
		// 	LOCTEXT("HudEnableTooltip", "Enables or disables the debug hud completely."),
		// 	FSlateIcon(FAppStyle::Get().GetStyleSetName(), "Debug"),
		// 	EUserInterfaceActionType::ToggleButton
		// );
	}
	
	// ToolbarBuilder.AddSeparator();
	

	ToolbarBuilder.EndSection();
	ToolbarBuilder.EndStyleOverride();

	return ToolbarBuilder.MakeWidget();
}

void SRootMotionEditor::NotifyCurveChanged()
{
	
}

#undef LOCTEXT_NAMESPACE
